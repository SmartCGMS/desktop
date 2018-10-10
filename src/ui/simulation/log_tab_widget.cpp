/**
 * SmartCGMS - continuous glucose monitoring and controlling framework
 * https://diabetes.zcu.cz/
 *
 * Contact:
 * diabetes@mail.kiv.zcu.cz
 * Medical Informatics, Department of Computer Science and Engineering
 * Faculty of Applied Sciences, University of West Bohemia
 * Technicka 8
 * 314 06, Pilsen
 *
 * Licensing terms:
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * a) For non-profit, academic research, this software is available under the
 *    GPLv3 license. When publishing any related work, user of this software
 *    must:
 *    1) let us know about the publication,
 *    2) acknowledge this software and respective literature - see the
 *       https://diabetes.zcu.cz/about#publications,
 *    3) At least, the user of this software must cite the following paper:
 *       Parallel software architecture for the next generation of glucose
 *       monitoring, Proceedings of the 8th International Conference on Current
 *       and Future Trends of Information and Communication Technologies
 *       in Healthcare (ICTH 2018) November 5-8, 2018, Leuven, Belgium
 * b) For any other use, especially commercial use, you must contact us and
 *    obtain specific terms and conditions for the use of the software.
 */

#include "log_tab_widget.h"

#include "../../../../common/lang/dstrings.h"
#include "../../../../common/utils/QtUtils.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QScrollBar>
#include <fstream>
#include <sstream>

#include <QtCore/QTimer>
#include <QtCore/QEventLoop>
#include <QtWidgets/QTableView>
#include <QtWidgets/QHeaderView>

#include "moc_log_tab_widget.cpp"

/* RAW subtab widget */

CLog_Subtab_Raw_Widget::CLog_Subtab_Raw_Widget(QWidget *parent)
	: CAbstract_Simulation_Tab_Widget(parent)
{
	mLogContents = new QTextEdit();

	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->addWidget(mLogContents);
	setLayout(mainLayout);
}

void CLog_Subtab_Raw_Widget::Log_Message(const std::wstring &msg)
{
	// copy, then pass by value
	std::wstring logmsg(msg);

	QEventLoop loop;
	Q_UNUSED(loop);
	QTimer::singleShot(0, this, [this, logmsg]()
	{
		mLogContents->append(StdWStringToQString(logmsg));
	});
}

CAbstract_Simulation_Tab_Widget* CLog_Subtab_Raw_Widget::Clone()
{
	CLog_Subtab_Raw_Widget* cloned = new CLog_Subtab_Raw_Widget();
	cloned->Set_Contents(mLogContents->document()->toPlainText());

	return cloned;
}

void CLog_Subtab_Raw_Widget::Set_Contents(const QString& contents)
{
	mLogContents->document()->setPlainText(contents);
}

/* TABLE subtab widget */

CLog_Subtab_Table_Widget::CLog_Subtab_Table_Widget(QWidget *parent)
	: CAbstract_Simulation_Tab_Widget(parent)
{
	QGridLayout *mainLayout = new QGridLayout;

	mTableView = new QTableView();
	mModel = new CLog_Table_Model(this);
	mTableView->setModel(mModel);
	mainLayout->addWidget(mTableView);
	
	mTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	setLayout(mainLayout);
}

void CLog_Subtab_Table_Widget::Log_Message(const std::wstring &msg)
{
	// copy, then pass by value
	std::wstring logmsg(msg);

	//QEventLoop loop;
	//Q_UNUSED(loop);
	QTimer::singleShot(0, this, [this, logmsg]()
	{
		mModel->Log_Message(logmsg);
		// scrolling to bottom is disabled for now
		//mTableView->scrollToBottom();
	});
}

CAbstract_Simulation_Tab_Widget* CLog_Subtab_Table_Widget::Clone()
{
	CLog_Subtab_Table_Widget* cloned = new CLog_Subtab_Table_Widget();
	cloned->Append_From_Model(mModel);

	return cloned;
}

void CLog_Subtab_Table_Widget::Append_From_Model(CLog_Table_Model* source)
{
	for (int i = 0; i < source->rowCount(); i++)
	{
		mModel->insertRows(i, 1, QModelIndex());

		for (int j = 0; j < source->columnCount(); j++)
		{
			QModelIndex idx = mModel->index(i, j, QModelIndex());
			mModel->setData(idx, source->data(idx), Qt::DisplayRole);
		}
	}
}

CLog_Table_Model::CLog_Table_Model(QObject *parent) noexcept : QAbstractTableModel(parent) {

	std::wistringstream iss(dsLog_Header);
	std::wstring str;
	while (std::getline(iss, str, L';')) {
		mHeaderTitles.push_back(str);
	}
}

int CLog_Table_Model::rowCount(const QModelIndex &idx) const
{
	return static_cast<int>(mLogRows.size());
}

int CLog_Table_Model::columnCount(const QModelIndex &idx) const
{
	return static_cast<int>(mHeaderTitles.size());
}

QVariant CLog_Table_Model::data(const QModelIndex &index, int role) const
{
	// content of log cells
	if (role == Qt::DisplayRole)
	{
		const size_t row = static_cast<size_t>(index.row());
		const size_t col = static_cast<size_t>(index.column());

		if (row < mLogRows.size() && col < mLogRows[row].size())
			return mLogRows[row][col];
	}

	return QVariant();
}

QVariant CLog_Table_Model::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		// horizontal - use column names stored during widget creation
		if (orientation == Qt::Horizontal)
		{
			if (section >= 0 && section < static_cast<int>(mHeaderTitles.size()))
				return StdWStringToQString(mHeaderTitles[section]);
		}
	}

	return QVariant();
}

bool CLog_Table_Model::insertRows(int position, int rows, const QModelIndex &index)
{
	beginInsertRows(QModelIndex(), position, position + rows - 1);

	const size_t oldSize = mLogRows.size();

	mLogRows.resize(static_cast<size_t>(position) + size_t(1));
	// stretch vector inside to desired size
	for (size_t i = oldSize; i < mLogRows.size(); i++)
		mLogRows[i].resize(mHeaderTitles.size());

	endInsertRows();

	return true;
}

bool CLog_Table_Model::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid())
		return false;

	const size_t row = static_cast<size_t>(index.row());
	const size_t col = static_cast<size_t>(index.column());

	mLogRows[row][col] = value.toString();
	emit(dataChanged(index, index));

	return true;
}

void CLog_Table_Model::Log_Message(const std::wstring &msg)
{
	int row = static_cast<int>(mLogRows.size());

	insertRows(row, 1, QModelIndex());

	int i = 0;
	std::wistringstream iss(msg);
	std::wstring str;
	while (std::getline(iss, str, L';')) {
		QModelIndex idx = index(row, i++, QModelIndex());
		setData(idx, StdWStringToQString(str), Qt::EditRole);
	}
}

/* PARENT widget */

CLog_Tab_Widget::CLog_Tab_Widget(QWidget *parent)
	: CAbstract_Simulation_Tab_Widget(parent)
{
	Init_Layout();

	mTableLogWidget = new CLog_Subtab_Table_Widget(this);
	mTabWidget->addTab(mTableLogWidget, tr(dsLog_Table_View));
	mRawLogWidget = new CLog_Subtab_Raw_Widget(this);
	mTabWidget->addTab(mRawLogWidget, tr(dsLog_Raw_View));
}

CLog_Tab_Widget::CLog_Tab_Widget(CLog_Subtab_Raw_Widget* cloned_raw, CLog_Subtab_Table_Widget* cloned_table)
{
	Init_Layout();

	mTableLogWidget = cloned_table;
	mTabWidget->addTab(mTableLogWidget, tr(dsLog_Table_View));
	mRawLogWidget = cloned_raw;
	mTabWidget->addTab(mRawLogWidget, tr(dsLog_Raw_View));
}

void CLog_Tab_Widget::Init_Layout()
{
	QGridLayout *mainLayout = new QGridLayout;

	mTabWidget = new QTabWidget();
	mainLayout->addWidget(mTabWidget);

	setLayout(mainLayout);
}

void CLog_Tab_Widget::Log_Message(const std::wstring &msg)
{
	mRawLogWidget->Log_Message(msg);
	mTableLogWidget->Log_Message(msg);
}

CAbstract_Simulation_Tab_Widget* CLog_Tab_Widget::Clone()
{
	CLog_Tab_Widget* cloned = new CLog_Tab_Widget(
		dynamic_cast<CLog_Subtab_Raw_Widget*>(mRawLogWidget->Clone()),
		dynamic_cast<CLog_Subtab_Table_Widget*>(mTableLogWidget->Clone()));

	return cloned;
}

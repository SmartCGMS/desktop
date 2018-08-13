#include "log_tab_widget.h"

#include "../../../../common/lang/dstrings.h"
#include "../../../../../common/QtUtils.h"

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

	QEventLoop loop;
	Q_UNUSED(loop);
	QTimer::singleShot(0, this, [this, logmsg]()
	{
		mModel->Log_Message(logmsg);
		// scrolling to bottom is disabled for now
		//mTableView->scrollToBottom();
	});
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
			if (section >= 0 && section < mHeaderTitles.size())
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
	QGridLayout *mainLayout = new QGridLayout;

	mTabWidget = new QTabWidget();
	mainLayout->addWidget(mTabWidget);

	mTableLogWidget = new CLog_Subtab_Table_Widget(this);
	mTabWidget->addTab(mTableLogWidget, tr(dsLog_Table_View));
	mRawLogWidget = new CLog_Subtab_Raw_Widget(this);
	mTabWidget->addTab(mRawLogWidget, tr(dsLog_Raw_View));

	setLayout(mainLayout);
}

void CLog_Tab_Widget::Log_Message(const std::wstring &msg)
{
	mRawLogWidget->Log_Message(msg);
	mTableLogWidget->Log_Message(msg);
}

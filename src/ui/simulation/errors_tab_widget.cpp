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

#include "errors_tab_widget.h"

#include "../../../../common/lang/dstrings.h"
#include "../../../../common/rtl/UILib.h"
#include "../../../../common/utils/QtUtils.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QScrollBar>

#include <fstream>
#include <cmath>

#include <QtCore/QTimer>
#include <QtCore/QEventLoop>

#include "moc_errors_tab_widget.cpp"

constexpr int Error_Column_Count = static_cast<int>(glucose::NError_Marker::count) + static_cast<int>(glucose::NError_Percentile::count);

// names of error types
const wchar_t* gError_Names[static_cast<size_t>(glucose::NError_Type::count)] = {
	dsError_Absolute,
	dsError_Relative
};
// ensure array length
static_assert((sizeof(gError_Names) / sizeof(const wchar_t*)) == static_cast<size_t>(glucose::NError_Type::count), "Error type names count does not match error types defined");

// names of columns in table
const wchar_t* gError_Column_Names[] = {
	dsError_Column_Average,
	dsError_Column_StdDev,
	dsError_Column_AIC,
	dsError_Column_Minimum,
	dsError_Column_First_Quantile,
	dsError_Column_Median,
	dsError_Column_Third_Quantile,
	dsError_Column_95_Quantile,
	dsError_Column_99_Quantile,
	dsError_Column_Maximum
};
// ensure array length
static_assert((sizeof(gError_Column_Names) / sizeof(const wchar_t*)) == Error_Column_Count, "Column count does not match error columns defined");

CError_Table_Model::CError_Table_Model(QObject *parent) noexcept : QAbstractTableModel(parent), mMaxSignalRow(0) {
}

int CError_Table_Model::rowCount(const QModelIndex &idx) const
{
	return mMaxSignalRow * static_cast<int>(glucose::NError_Type::count);
}

int CError_Table_Model::columnCount(const QModelIndex &idx) const
{
	return Error_Column_Count;
}

QString Format_Error_String(glucose::NError_Type type, double val)
{
	// infinite, NaN, or other values like that results in empty string
	const auto cl = std::fpclassify(val);
	if (cl != FP_NORMAL && cl != FP_ZERO)
		return QString();

	switch (type)
	{
		case glucose::NError_Type::Absolute:
			return QString("%1").arg(val, 0, 'g', 4);
		case glucose::NError_Type::Relative:
			return QString("%1%").arg(val*100.0, 0, 'g', 4);
	}

	return QString::number(val);
}

QVariant CError_Table_Model::data(const QModelIndex &index, int role) const
{
	// content of error cells
	if (role == Qt::DisplayRole)
	{
		const int row = index.row();
		double val;

		if (index.column() <static_cast<int>(glucose::NError_Marker::count))
			val = mErrors[row /static_cast<int>(glucose::NError_Type::count)][row %static_cast<int>(glucose::NError_Type::count)].markers[index.column()];
		else
			val = mErrors[row /static_cast<int>(glucose::NError_Type::count)][row %static_cast<int>(glucose::NError_Type::count)].percentile[index.column() - static_cast<int>(glucose::NError_Marker::count)];

		return Format_Error_String((glucose::NError_Type)(row %static_cast<int>(glucose::NError_Type::count)), val);
	}
	else if (role == Qt::BackgroundRole)
	{
		// use gray background for "average" column
		if (index.column() >= static_cast<int>(glucose::NError_Marker::count))
			return QBrush(QColor(224, 224, 224));
	}
	return QVariant();
}

QVariant CError_Table_Model::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		// horizontal - use column names (fixed)
		if (orientation == Qt::Horizontal)
		{
			if (section >= 0 && section < Error_Column_Count)
				return StdWStringToQString(gError_Column_Names[section]);
		}
		// vertical - use signal names and error type names
		else if (orientation == Qt::Vertical)
			return StdWStringToQString(mSignalNameList[section / static_cast<int>(glucose::NError_Type::count)] + L" (" + gError_Names[section % static_cast<int>(glucose::NError_Type::count)] + L")");
	}

	return QVariant();
}

bool CError_Table_Model::insertRows(int position, int rows, const QModelIndex &index)
{
	beginInsertRows(QModelIndex(), position, position + rows - 1);

	mErrors.resize(static_cast<size_t>(position) + size_t(1));

	endInsertRows();

	return true;
}

bool CError_Table_Model::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.isValid())
	{
		int row = index.row();

		if (index.column() < static_cast<int>(glucose::NError_Marker::count))
			mErrors[row /static_cast<int>(glucose::NError_Type::count)][row %static_cast<int>(glucose::NError_Type::count)].markers[index.column()] = value.toDouble();
		else if (index.column() < Error_Column_Count)
			mErrors[row /static_cast<int>(glucose::NError_Type::count)][row %static_cast<int>(glucose::NError_Type::count)].percentile[index.column() - static_cast<int>(glucose::NError_Marker::count)] = value.toDouble();

		emit(dataChanged(index, index));

		return true;
	}

	return false;
}

void CError_Table_Model::Set_Error(const GUID& signal_id, std::wstring signal_name, const glucose::TError_Markers& errors, const glucose::NError_Type type)
{
	bool added = false;
	int row;

	if (mSignalRow.find(signal_id) == mSignalRow.end())
	{
		row = mMaxSignalRow++;

		mSignalRow[signal_id] = row;
		mSignalNameList.push_back(signal_name);

		insertRows(row,static_cast<int>(glucose::NError_Type::count), QModelIndex());
	}
	else
		row = mSignalRow[signal_id];

	int offset = (int)type;

	QModelIndex idx = index(row*static_cast<size_t>(glucose::NError_Type::count) + offset, 0, QModelIndex());
	for (int i = 0; i < static_cast<int>(glucose::NError_Marker::count); i++)
	{
		idx = index(row*static_cast<size_t>(glucose::NError_Type::count) + offset, i, QModelIndex());
		setData(idx, errors.markers[i], Qt::EditRole);
	}
	for (int i = 0; i < static_cast<size_t>(glucose::NError_Percentile::count); i++)
	{
		idx = index(row*static_cast<size_t>(glucose::NError_Type::count) + offset, i + static_cast<int>(glucose::NError_Marker::count), QModelIndex());
		setData(idx, errors.percentile[i], Qt::EditRole);
	}
}




CErrors_Tab_Widget::CErrors_Tab_Widget(QWidget *parent) noexcept: CAbstract_Simulation_Tab_Widget(parent) {
	QGridLayout *mainLayout = new QGridLayout();

	mTableView = new QTableView();
	mModel = new CError_Table_Model(this);
	mTableView->setModel(mModel);
	mainLayout->addWidget(mTableView);

	auto models = glucose::get_model_descriptors();
	for (auto& model : models)
	{
		for (size_t i = 0; i < model.number_of_calculated_signals; i++)
			mSignalNames[model.calculated_signal_ids[i]] = model.description + std::wstring(L" - ") + model.calculated_signal_names[i];
	}

	for (size_t i = 0; i < glucose::signal_Virtual.size(); i++)		
		mSignalNames[glucose::signal_Virtual[i]] = dsSignal_Prefix_Virtual + std::wstring(L" ") + std::to_wstring(i);

	setLayout(mainLayout);
}

void CErrors_Tab_Widget::Update_Error_Metrics(const GUID& signal_id, glucose::TError_Markers& container, glucose::NError_Type type)
{
	QEventLoop loop;
	Q_UNUSED(loop);
	QTimer::singleShot(0, this, [this, signal_id, container, type]()
	{
		mModel->Set_Error(signal_id, mSignalNames[signal_id], container, type);

		mTableView->repaint();
	});
}

void CErrors_Tab_Widget::Reset()
{
	QEventLoop loop;
	Q_UNUSED(loop);
	QTimer::singleShot(0, this, [this]()
	{
		auto newModel = new CError_Table_Model(this);
		mTableView->setModel(newModel);

		delete mModel;
		mModel = newModel;
	});
}

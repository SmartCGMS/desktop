/**
 * SmartCGMS - continuous glucose monitoring and controlling framework
 * https://diabetes.zcu.cz/
 *
 * Copyright (c) since 2018 University of West Bohemia.
 *
 * Contact:
 * diabetes@mail.kiv.zcu.cz
 * Medical Informatics, Department of Computer Science and Engineering
 * Faculty of Applied Sciences, University of West Bohemia
 * Univerzitni 8
 * 301 00, Pilsen
 * 
 * 
 * Purpose of this software:
 * This software is intended to demonstrate work of the diabetes.zcu.cz research
 * group to other scientists, to complement our published papers. It is strictly
 * prohibited to use this software for diagnosis or treatment of any medical condition,
 * without obtaining all required approvals from respective regulatory bodies.
 *
 * Especially, a diabetic patient is warned that unauthorized use of this software
 * may result into severe injure, including death.
 *
 *
 * Licensing terms:
 * Unless required by applicable law or agreed to in writing, software
 * distributed under these license terms is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * a) For non-profit, academic research, this software is available under the
 *      GPLv3 license.
 * b) For any other use, especially commercial use, you must contact us and
 *       obtain specific terms and conditions for the use of the software.
 * c) When publishing work with results obtained using this software, you agree to cite the following paper:
 *       Tomas Koutny and Martin Ubl, "Parallel software architecture for the next generation of glucose
 *       monitoring", Procedia Computer Science, Volume 141C, pp. 279-286, 2018
 */

#include "errors_tab_widget.h"

#include "../../../../common/lang/dstrings.h"
#include "../../../../common/rtl/UILib.h"
#include "../../../../common/utils/QtUtils.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>

#include <iostream>
#include <fstream>
#include <cmath>

#include <QtCore/QTimer>
#include <QtCore/QEventLoop>

#include "moc_errors_tab_widget.cpp"

constexpr int Error_Column_Count = 15;
//= static_cast<int>(glucose::NError_Marker::count) + static_cast<int>(glucose::NError_Percentile::count) + static_cast<int>(glucose::NError_Range::count);

/*
// names of error types
const wchar_t* gError_Names[static_cast<size_t>(glucose::NError_Type::count)] = {
	dsError_Absolute,
	dsError_Relative
};
*/
// ensure array length
//static_assert((sizeof(gError_Names) / sizeof(const wchar_t*)) == static_cast<size_t>(glucose::NError_Type::count), "Error type names count does not match error types defined");

// names of columns in table
const std::array<const wchar_t*, Error_Column_Count>  gError_Column_Names = {
	dsDescription,	
	dsError_Column_Average,
	dsError_Column_StdDev,	
	dsError_Column_Sum,
	dsError_Column_Minimum,
	dsError_Column_First_Quantile,
	dsError_Column_Median,
	dsError_Column_Third_Quantile,
	dsError_Column_95_Quantile,
	dsError_Column_99_Quantile,
	dsError_Column_Maximum,
	dsError_Column_Range_5pct,
	dsError_Column_Range_10pct,
	dsError_Column_Range_25pct,
	dsError_Column_Range_50pct
};
// ensure array length
//static_assert((sizeof(gError_Column_Names) / sizeof(const wchar_t*)) == Error_Column_Count, "Column count does not match error columns defined");

CErrors_Tab_Widget_internal::CError_Table_Model::CError_Table_Model(QObject *parent) noexcept : QAbstractTableModel(parent), mMaxSignalRow(0) {
}

int CErrors_Tab_Widget_internal::CError_Table_Model::rowCount(const QModelIndex &idx) const
{
	//return mMaxSignalRow * static_cast<int>(glucose::NError_Type::count);
	return 3 * static_cast<int>(mSignal_Errors.size());
}

int CErrors_Tab_Widget_internal::CError_Table_Model::columnCount(const QModelIndex &idx) const {
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
		default:
			return QString::number(val);
	}
}

QVariant CErrors_Tab_Widget_internal::CError_Table_Model::data(const QModelIndex &index, int role) const {
	if (role == Qt::DisplayRole) {
		const int row = index.row();
		const int col = index.column();

		const int row_role = row % 3;	//0 is absolute error, then relative error

		constexpr int absolute_role = 0;
		constexpr int relative_role = 1;
		constexpr int spacing_role = 2;

		constexpr int desc_col = 0;		

		switch (row_role) {
			case absolute_role:
				switch (col) {					
				//case err_col: return QString::fromWCharArray(dsAbsolute); break;
				}
						
			case relative_role: break;
				switch (col) {
					//case err_col: return QString::fromWCharArray(dsRelative); break;
				}

			default: return QVariant();	//spacing role
		}
	}
	/*

	// content of error cells
	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		const int row = index.row();
		double val = std::numeric_limits<double>::quiet_NaN();

		if (index.column() < static_cast<int>(glucose::NError_Marker::count))
			val = mErrors[row /static_cast<int>(glucose::NError_Type::count)][row % static_cast<int>(glucose::NError_Type::count)].markers[index.column()];
		else if (index.column() < static_cast<int>(glucose::NError_Marker::count) + static_cast<int>(glucose::NError_Percentile::count))
			val = mErrors[row /static_cast<int>(glucose::NError_Type::count)][row % static_cast<int>(glucose::NError_Type::count)].percentile[index.column() - static_cast<int>(glucose::NError_Marker::count)];
		else if (index.column() < Error_Column_Count)
			val = mErrors[row / static_cast<int>(glucose::NError_Type::count)][row % static_cast<int>(glucose::NError_Type::count)].range[index.column() - static_cast<int>(glucose::NError_Marker::count) - static_cast<int>(glucose::NError_Percentile::count)];

		if (role == Qt::DisplayRole)
			return Format_Error_String((glucose::NError_Type)(row %static_cast<int>(glucose::NError_Type::count)), val);
		else
			return val;
	}
	else if (role == Qt::BackgroundRole)
	{
		// use gray background for "average" column
		if (index.column() >= static_cast<int>(glucose::NError_Marker::count) && index.column() < static_cast<int>(glucose::NError_Marker::count) + static_cast<int>(glucose::NError_Percentile::count))
			return QBrush(QColor(224, 224, 224));
	}

	*/
	return QVariant();
}

QVariant CErrors_Tab_Widget_internal::CError_Table_Model::headerData(int section, Qt::Orientation orientation, int role) const
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
		else if (orientation == Qt::Vertical) {
			switch (section % 3) {
				case 0:	return QString::fromWCharArray(dsAbsolute); break;
				case 1: return QString::fromWCharArray(dsRelative); break;
			}
		}


//			return StdWStringToQString(mSignalNameList[section / static_cast<int>(glucose::NError_Type::count)] + L" (" + gError_Names[section % static_cast<int>(glucose::NError_Type::count)] + L")");
	}

	return QVariant();
}

bool CErrors_Tab_Widget_internal::CError_Table_Model::insertRows(int position, int rows, const QModelIndex &index)
{
	beginInsertRows(QModelIndex(), position, position + rows - 1);

	mErrors.resize(static_cast<size_t>(position) + size_t(1));

	endInsertRows();

	return true;
}

bool CErrors_Tab_Widget_internal::CError_Table_Model::setData(const QModelIndex &index, const QVariant &value, int role) {
	return false;


	/* why did we allowed any editation?
	if (index.isValid())
	{
		int row = index.row();

		if (index.column() < static_cast<int>(glucose::NError_Marker::count))
			mErrors[row /static_cast<int>(glucose::NError_Type::count)][row %static_cast<int>(glucose::NError_Type::count)].markers[index.column()] = value.toDouble();
		else if (index.column() < static_cast<int>(glucose::NError_Marker::count) + static_cast<int>(glucose::NError_Percentile::count))
			mErrors[row / static_cast<int>(glucose::NError_Type::count)][row %static_cast<int>(glucose::NError_Type::count)].percentile[index.column() - static_cast<int>(glucose::NError_Marker::count)] = value.toDouble();
		else if (index.column() < Error_Column_Count)
			mErrors[row / static_cast<int>(glucose::NError_Type::count)][row %static_cast<int>(glucose::NError_Type::count)].range[index.column() - static_cast<int>(glucose::NError_Marker::count) - static_cast<int>(glucose::NError_Percentile::count)] = value.toDouble();

		emit(dataChanged(index, index));

		return true;
	}

	return false;
	*/
}

void CErrors_Tab_Widget_internal::CError_Table_Model::Set_Error(const GUID& signal_id, std::wstring signal_name, const glucose::TError_Markers& errors, const glucose::NError_Type type)
{
	int row;

	if (mSignalRow.find(signal_id) == mSignalRow.end())
	{
		row = mMaxSignalRow++;

		mSignalRow[signal_id] = row;
		mSignalNameList.push_back(signal_name);

		insertRows(row, static_cast<int>(glucose::NError_Type::count), QModelIndex());
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
	for (int i = 0; i < static_cast<int>(glucose::NError_Percentile::count); i++)
	{
		idx = index(row*static_cast<int>(glucose::NError_Type::count) + offset, i + static_cast<int>(glucose::NError_Marker::count), QModelIndex());
		setData(idx, errors.percentile[i], Qt::EditRole);
	}
	for (int i = 0; i < static_cast<int>(glucose::NError_Range::count); i++)
	{
		idx = index(row*static_cast<int>(glucose::NError_Type::count) + offset, i + static_cast<int>(glucose::NError_Marker::count) + static_cast<int>(glucose::NError_Percentile::count), QModelIndex());
		setData(idx, errors.range[i], Qt::EditRole);
	}
}

void CErrors_Tab_Widget_internal::CError_Table_Model::Set_From_Model(const std::map<GUID, int>& srcSignalMap, const std::vector<std::wstring>& srcSignalNameMap, CError_Table_Model* source)
{
	int row;

	for (const auto& sig : srcSignalMap)
	{
		row = mMaxSignalRow++;

		mSignalRow[sig.first] = row;
		mSignalNameList.push_back(srcSignalNameMap[sig.second]);

		insertRows(row, static_cast<int>(glucose::NError_Type::count), QModelIndex());
	}

	for (int i = 0; i < source->rowCount(); i++)
	{
		for (int j = 0; j < source->columnCount(); j++)
		{
			QModelIndex idx = index(i, j, QModelIndex());
			setData(idx, source->data(idx, Qt::EditRole), Qt::EditRole);
		}
	}
}

void CErrors_Tab_Widget_internal::CError_Table_Model::Clone_To_Model(CError_Table_Model* dest)
{
	dest->Set_From_Model(mSignalRow, mSignalNameList, this);
}


void CErrors_Tab_Widget_internal::CError_Table_Model::On_Filter_Configured(glucose::IFilter *filter) {
	if (glucose::SSignal_Error_Inspection insp = glucose::SSignal_Error_Inspection{ glucose::SFilter{filter} }) {
		wchar_t *tmp_desc;
		insp->Get_Description(&tmp_desc);		
		CErrors_Tab_Widget_internal::TSignal_Error_Inspection tmp{ tmp_desc, insp };
		insp->Calculate_Signal_Error(&tmp.recent_abs_error, &tmp.recent_rel_error);
		mSignal_Errors.push_back(tmp);
	}
}

void CErrors_Tab_Widget_internal::CError_Table_Model::Update_Errors() {

	for (auto &signal_error : mSignal_Errors) {		
		if (signal_error.signal_error->Peek_New_Data_Available() == S_OK) {
			if (signal_error.signal_error->Calculate_Signal_Error(&signal_error.recent_abs_error, &signal_error.recent_rel_error) == S_OK) {

			}
		}
	}
}



CErrors_Tab_Widget::CErrors_Tab_Widget(QWidget *parent) noexcept: CAbstract_Simulation_Tab_Widget(parent) {
	QGridLayout *mainLayout = new QGridLayout();

	mTableView = new QTableView();
	mModel = new CErrors_Tab_Widget_internal::CError_Table_Model(this);
	mTableView->setModel(mModel);
	mainLayout->addWidget(mTableView);

	QPushButton* exportBtn = new QPushButton(dsExport_To_CSV);
	mainLayout->addWidget(exportBtn, 1, 0);

	auto models = glucose::get_model_descriptors();
	for (auto& model : models)
	{
		for (size_t i = 0; i < model.number_of_calculated_signals; i++)
			mSignalNames[model.calculated_signal_ids[i]] = model.description + std::wstring(L" - ") + model.calculated_signal_names[i];
	}

	for (size_t i = 0; i < glucose::signal_Virtual.size(); i++)
		mSignalNames[glucose::signal_Virtual[i]] = dsSignal_Prefix_Virtual + std::wstring(L" ") + std::to_wstring(i);

	setLayout(mainLayout);

	connect(exportBtn, SIGNAL(clicked()), this, SLOT(Export_CSV_Button_Clicked()));
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

void CErrors_Tab_Widget::Export_CSV_Button_Clicked()
{
	auto path = QFileDialog::getSaveFileName(this, tr(dsExport_CSV_Dialog_Title), dsExport_CSV_Default_File_Name, tr(dsExport_CSV_Ext_Spec));
	if (path.length() != 0)
	{
		std::ofstream fs(path.toStdString());

		int fromRow, fromCol, toRow, toCol;
		fromRow = 0;
		fromCol = 0;
		toRow = mTableView->model()->rowCount() - 1;
		toCol = mTableView->model()->columnCount() - 1;

		auto* selModel = mTableView->selectionModel();
		if (selModel->hasSelection())
		{
			auto rowList = selModel->selectedRows();
			if (!rowList.empty())
			{
				fromRow = mTableView->model()->rowCount() - 1;
				toRow = 0;

				for (auto& val : rowList)
				{
					if (val.row() > toRow)
						toRow = val.row();
					if (val.row() < fromRow)
						fromRow = val.row();
				}
			}

			auto colList = selModel->selectedColumns();
			if (!colList.empty())
			{
				fromCol = mTableView->model()->columnCount() - 1;
				toCol = 0;

				for (auto& val : colList)
				{
					if (val.column() > toCol)
						toCol = val.column();
					if (val.column() < fromCol)
						fromCol = val.column();
				}
			}
		}

		// skip one column
		fs << ";";
		for (int j = fromCol; j <= toCol; j++)
			fs << mTableView->model()->headerData(j, Qt::Orientation::Horizontal).toString().toStdString() << ";";

		fs << std::endl;

		for (int i = fromRow; i <= toRow; i++)
		{
			fs << mTableView->model()->headerData(i, Qt::Orientation::Vertical).toString().toStdString() << ";";

			for (int j = fromCol; j <= toCol; j++)
			{
				QModelIndex idx = mTableView->model()->index(i, j, QModelIndex());

				fs << mTableView->model()->data(idx).toString().toStdString() << ";";
			}

			fs << std::endl;
		}
	}
}

CAbstract_Simulation_Tab_Widget* CErrors_Tab_Widget::Clone()
{
	CErrors_Tab_Widget* cloned = new CErrors_Tab_Widget();
	cloned->Clone_From_Model(mModel);

	return cloned;
}

void CErrors_Tab_Widget::Clone_From_Model(CErrors_Tab_Widget_internal::CError_Table_Model* source)
{
	source->Clone_To_Model(mModel);
}

void CErrors_Tab_Widget::Reset()
{
	QEventLoop loop;
	Q_UNUSED(loop);
	QTimer::singleShot(0, this, [this]()
	{
		auto newModel = new CErrors_Tab_Widget_internal::CError_Table_Model(this);
		mTableView->setModel(newModel);

		delete mModel;
		mModel = newModel;
	});
}


void CErrors_Tab_Widget::On_Filter_Configured(glucose::IFilter *filter) {
	if (mModel) mModel->On_Filter_Configured(filter);
}

void CErrors_Tab_Widget::Refresh() {
	if (mModel) mModel->Update_Errors();
}
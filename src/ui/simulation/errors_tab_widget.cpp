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

constexpr int Error_Column_Count = 16;

// names of columns in table
const std::array<const wchar_t*, Error_Column_Count>  gError_Column_Names = {
	dsDescription,	
	dsError_Column_Average,
	dsError_Column_StdDev,	
	dsError_Column_Sum,
	dsError_Column_Count,
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


CErrors_Tab_Widget_internal::CError_Table_Model::CError_Table_Model(QObject *parent) noexcept : QAbstractTableModel(parent) {
	//
}

int CErrors_Tab_Widget_internal::CError_Table_Model::rowCount(const QModelIndex &idx) const
{
	//return mMaxSignalRow * static_cast<int>(glucose::NError_Type::count);
	const size_t signal_errors_size = mSignal_Errors.size();
	return signal_errors_size == 0 ?  0 : 3 * static_cast<int>(signal_errors_size)-1;
}

int CErrors_Tab_Widget_internal::CError_Table_Model::columnCount(const QModelIndex &idx) const {
	return Error_Column_Count;
}

QString Format_Error_String(double val, bool relative)
{
	// infinite, NaN, or other values like that results in empty string
	const auto cl = std::fpclassify(val);
	if (cl != FP_NORMAL && cl != FP_ZERO)
		return QString();

	if (relative)  return QString("%1%").arg(val*100.0, 0, 'g', 4);
		else return QString("%1").arg(val, 0, 'g', 4);
}

QVariant CErrors_Tab_Widget_internal::CError_Table_Model::data(const QModelIndex &index, int role) const {

	auto display_signal = [](const int col, const glucose::TSignal_Error &error, bool relative) {
		constexpr int avg_col = 1;
		constexpr int stdev_col = 2;
		constexpr int sum_col = 3;
		constexpr int count_col = 4;
		
		constexpr int min_col = 5;
		constexpr int p25_col = 6;
		constexpr int mad_col = 7;
		constexpr int p75_col = 8;
		constexpr int p95_col = 9;
		constexpr int p99_col = 10;
		constexpr int max_col = 11;

		switch (col) {
			case avg_col:	return Format_Error_String(error.avg, relative);
			case stdev_col: return Format_Error_String(error.stddev, relative);
			case sum_col:	return Format_Error_String(error.sum, relative);
			case count_col: return Format_Error_String(error.count, false);	

			case min_col:	return Format_Error_String(error.ecdf[static_cast<size_t>(glucose::NECDF::min_value)], relative);
			case p25_col:	return Format_Error_String(error.ecdf[static_cast<size_t>(glucose::NECDF::p25)], relative);
			case mad_col:	return Format_Error_String(error.ecdf[static_cast<size_t>(glucose::NECDF::median)], relative);
			case p75_col:	return Format_Error_String(error.ecdf[static_cast<size_t>(glucose::NECDF::p75)], relative);
			case p95_col:	return Format_Error_String(error.ecdf[static_cast<size_t>(glucose::NECDF::p95)], relative);
			case p99_col:	return Format_Error_String(error.ecdf[static_cast<size_t>(glucose::NECDF::p99)], relative);
			case max_col:	return Format_Error_String(error.ecdf[static_cast<size_t>(glucose::NECDF::max_value)], relative);
			default:		return QString{};
		}
	};


	if (role == Qt::DisplayRole || role == Qt::EditRole) {
		const int row = index.row();
		const int col = index.column();

		const int row_role = row % 3;	//0 is absolute error, then relative error
		const size_t error_index = mSignal_Errors.size() / 3;

		constexpr int absolute_role = 0;
		constexpr int relative_role = 1;
		constexpr int spacing_role = 2;

		constexpr int desc_col = 0;		
		constexpr int rel5_col = 12;
		
		if (col == desc_col) {
			return QVariant{ QString::fromStdWString(mSignal_Errors[error_index].description) };
		}
		else if (col < rel5_col) {

			switch (row_role) {
				case absolute_role: return display_signal(col, mSignal_Errors[error_index].recent_abs_error, false); break;
				case relative_role: return display_signal(col, mSignal_Errors[error_index].recent_rel_error, true); break;
				default: return QVariant();	//spacing role				
			}
		}
		else if (col>= rel5_col) {
			//relative errors
			
		} else
			return QVariant{};	//default value							
	}	//end of Qt display role
	
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
	}

	return QVariant();
}

CErrors_Tab_Widget_internal::CError_Table_Model* CErrors_Tab_Widget_internal::CError_Table_Model::Clone(QObject *parent) {
	CErrors_Tab_Widget_internal::CError_Table_Model* result = new CErrors_Tab_Widget_internal::CError_Table_Model(parent);
	result->mSignal_Errors.assign(this->mSignal_Errors.begin(), this->mSignal_Errors.end());
	return result;
}


void CErrors_Tab_Widget_internal::CError_Table_Model::On_Filter_Configured(glucose::IFilter *filter) {
	CErrors_Tab_Widget_internal::TSignal_Error_Inspection inspection;

	inspection.signal_error = glucose::SSignal_Error_Inspection{ glucose::SFilter{filter} };
	if (inspection.signal_error) {
		wchar_t *tmp_desc;
		inspection.description = inspection.signal_error->Get_Description(&tmp_desc) == S_OK ? tmp_desc : dsSignal_Unknown;		
		inspection.signal_error->Calculate_Signal_Error(&inspection.recent_abs_error, &inspection.recent_rel_error);
		mSignal_Errors.push_back(inspection);
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


void CErrors_Tab_Widget_internal::CError_Table_Model::Clear_Filters() {
	mSignal_Errors.clear();
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

CAbstract_Simulation_Tab_Widget* CErrors_Tab_Widget::Clone() {
	CErrors_Tab_Widget* cloned_widget = new CErrors_Tab_Widget();
	auto empty_model = cloned_widget->mTableView->model();
	auto cloned_model = static_cast<CErrors_Tab_Widget_internal::CError_Table_Model*>(this->mTableView->model())->Clone();
	cloned_widget->mTableView->setModel(cloned_model);
	delete empty_model;

	return cloned_widget;
}

void CErrors_Tab_Widget::On_Filter_Configured(glucose::IFilter *filter) {
	if (mModel) mModel->On_Filter_Configured(filter);
}

void CErrors_Tab_Widget::Refresh() {
	if (mModel) mModel->Update_Errors();
	mTableView->update();	
}

void CErrors_Tab_Widget::Clear_Filters() {
	if (mModel) mModel->Clear_Filters();
}
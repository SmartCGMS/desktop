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
 * Univerzitni 8, 301 00 Pilsen
 * Czech Republic
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
 * a) This file is available under the Apache License, Version 2.0.
 * b) When publishing any derivative work or results obtained using this software, you agree to cite the following paper:
 *    Tomas Koutny and Martin Ubl, "SmartCGMS as a Testbed for a Blood-Glucose Level Prediction and/or 
 *    Control Challenge with (an FDA-Accepted) Diabetic Patient Simulation", Procedia Computer Science,  
 *    Volume 177, pp. 354-362, 2020
 */

#include "errors_tab_widget.h"

#include <scgms/lang/dstrings.h>
#include <scgms/rtl/UILib.h>
#include <scgms/utils/QtUtils.h>

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

constexpr int Error_Column_Count = 18;

// names of columns in table
const std::array<const wchar_t*, Error_Column_Count>  gError_Column_Names = {
	dsDescription,	
	dsError_Column_Average,
	dsError_Column_StdDev,
	dsError_Column_ExcKurtosis,
	dsError_Column_Skewness,
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

int CErrors_Tab_Widget_internal::CError_Table_Model::rowCount(const QModelIndex &idx) const {
	//return mMaxSignalRow * static_cast<int>(scgms::NError_Type::count);
	const size_t signal_errors_size = mSignal_Errors.size();
	return signal_errors_size == 0 ?  0 : 3 * static_cast<int>(signal_errors_size)-1;
}

int CErrors_Tab_Widget_internal::CError_Table_Model::columnCount(const QModelIndex &idx) const {
	return Error_Column_Count;
}

QString Format_Error_String(double val, bool relative) {
	// infinite, NaN, or other values like that results in empty string
	const auto cl = std::fpclassify(val);
	if (cl != FP_NORMAL && cl != FP_ZERO) {
		return QString();
	}

	if (relative) {
		return QString("%1%").arg(val * 100.0, 0, 'g', 4);
	}
	else {
		return QString("%1").arg(val, 0, 'g', 4);
	}
}

QVariant CErrors_Tab_Widget_internal::CError_Table_Model::data(const QModelIndex &index, int role) const {

	auto display_signal = [](const int col, const scgms::TSignal_Stats &error, bool is_relative) {
		constexpr int avg_col = 1;
		constexpr int stdev_col = 2;
		constexpr int exckurt_col = 3;
		constexpr int skew_col = 4;
		constexpr int sum_col = 5;
		constexpr int count_col = 6;
		
		constexpr int min_col = 7;
		constexpr int p25_col = 8;
		constexpr int med_col = 9;
		constexpr int p75_col = 10;
		constexpr int p95_col = 11;
		constexpr int p99_col = 12;
		constexpr int max_col = 13;

		switch (col) {
			case avg_col:
				return Format_Error_String(error.avg, is_relative);
			case stdev_col:
				return Format_Error_String(error.stddev, is_relative);
			case exckurt_col:
				return Format_Error_String(error.exc_kurtosis, is_relative);
			case skew_col:
				return Format_Error_String(error.skewness, is_relative);
			case sum_col:
				return Format_Error_String(error.sum, is_relative);
			case count_col:
				return Format_Error_String(error.count, false);
			case min_col:
				return Format_Error_String(error.ecdf[scgms::NECDF::min_value], is_relative);
			case p25_col:
				return Format_Error_String(error.ecdf[scgms::NECDF::p25], is_relative);
			case med_col:
				return Format_Error_String(error.ecdf[scgms::NECDF::median], is_relative);
			case p75_col:
				return Format_Error_String(error.ecdf[scgms::NECDF::p75], is_relative);
			case p95_col:
				return Format_Error_String(error.ecdf[scgms::NECDF::p95], is_relative);
			case p99_col:
				return Format_Error_String(error.ecdf[scgms::NECDF::p99], is_relative);
			case max_col:
				return Format_Error_String(error.ecdf[scgms::NECDF::max_value], is_relative);
			default:
				return QString{};
		}
	};

	if (role == Qt::DisplayRole || role == Qt::EditRole) {
		const int row = index.row();
		const int col = index.column();

		const int row_role = row % 3;	//0 is absolute error, then relative error
		const size_t error_index = row / 3;

		constexpr int absolute_role = 0;
		constexpr int relative_role = 1;

		constexpr int desc_col = 0;

		constexpr int rel5_col = 12;
		constexpr int rel10_col = 13;
		constexpr int rel25_col = 14;
		constexpr int rel50_col = 15;
		
		if (col == desc_col) {
			return QVariant{ QString::fromStdWString(mSignal_Errors[error_index].description) };
		}
		else if (col < rel5_col) {

			switch (row_role) {
				case absolute_role:
					return display_signal(col, mSignal_Errors[error_index].recent_abs_error, false);
				case relative_role:
					return display_signal(col, mSignal_Errors[error_index].recent_rel_error, true);
				default:
					return QVariant();	//spacing role
			}
		}
		else if ((col>= rel5_col) && (row_role == relative_role)) {
			//relative errors
				switch (col) {
					case rel5_col:
						return Format_Error_String(mSignal_Errors[error_index].r5, true);
					case rel10_col:
						return Format_Error_String(mSignal_Errors[error_index].r10, true);
					case rel25_col:
						return Format_Error_String(mSignal_Errors[error_index].r25, true);
					case rel50_col:
						return Format_Error_String(mSignal_Errors[error_index].r50, true);
					default:
						return display_signal(col, mSignal_Errors[error_index].recent_rel_error, true);
				}
		}
		else {
			return QVariant{};	//default value
		}
	}	//end of Qt display role
	
	return QVariant();
}

QVariant CErrors_Tab_Widget_internal::CError_Table_Model::headerData(int section, Qt::Orientation orientation, int role) const {

	if (role == Qt::DisplayRole) {
		// horizontal - use column names (fixed)
		if (orientation == Qt::Horizontal) {
			if (section >= 0 && section < Error_Column_Count) {
				return StdWStringToQString(gError_Column_Names[section]);
			}
		}
		// vertical - use signal names and error type names
		else if (orientation == Qt::Vertical) {
			switch (section % 3) {
				case 0:
					return QString::fromWCharArray(dsAbsolute);
				case 1:
					return QString::fromWCharArray(dsRelative);
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

void CErrors_Tab_Widget_internal::CError_Table_Model::On_Filter_Configured(scgms::IFilter *filter) {
	CErrors_Tab_Widget_internal::TSignal_Error_Inspection inspection;

	inspection.signal_error = scgms::SSignal_Error_Inspection{ scgms::SFilter{filter} };
	if (inspection.signal_error) {
		wchar_t *tmp_desc;
		inspection.description = inspection.signal_error->Get_Description(&tmp_desc) == S_OK ? tmp_desc : dsSignal_Unknown;
		inspection.signal_error->Calculate_Signal_Error(scgms::All_Segments_Id, &inspection.recent_abs_error, &inspection.recent_rel_error);
		inspection.r5 = inspection.r10 = inspection.r25 = inspection.r50 = std::numeric_limits<double>::quiet_NaN();
		mSignal_Errors.push_back(inspection);
	}
}

void CErrors_Tab_Widget_internal::CError_Table_Model::Update_Errors() {

	bool called_begin_reset = false;

	for (auto &signal_error : mSignal_Errors) {
		if (signal_error.signal_error->Logical_Clock(&mErrors_Logical_Clock) == S_OK) {

			if (!called_begin_reset) {
				beginResetModel();
				called_begin_reset = true;
			}

			if (signal_error.signal_error && signal_error.signal_error->Calculate_Signal_Error(scgms::All_Segments_Id, &signal_error.recent_abs_error, &signal_error.recent_rel_error) == S_OK) {
				if (signal_error.recent_rel_error.count > 0) {

					auto inv_ecdf = [&signal_error](const double threshold)->double {
						auto found = std::lower_bound(signal_error.recent_rel_error.ecdf.begin(), signal_error.recent_rel_error.ecdf.end(), threshold);
						if (found != signal_error.recent_rel_error.ecdf.end()) {
							return 0.01*static_cast<double>(std::distance(signal_error.recent_rel_error.ecdf.begin(), found));
						}
						else {
							return 1.0;	//100% relative
						}
					};

					signal_error.r5 = inv_ecdf(0.05);
					signal_error.r10 = inv_ecdf(0.1);
					signal_error.r25 = inv_ecdf(0.25);
					signal_error.r50 = inv_ecdf(0.50);
				}
			}
		}
	}

	if (called_begin_reset) {
		endResetModel();
	}

	emit dataChanged(createIndex(0, 0), createIndex(rowCount() - 1, Error_Column_Count - 1));
}

void CErrors_Tab_Widget_internal::CError_Table_Model::Clear_Filters(bool wipeTable) {
	if (wipeTable) {
		mSignal_Errors.clear();
	}
	else {
		for (auto& signal_error : mSignal_Errors) {
			signal_error.signal_error.reset();
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

	setLayout(mainLayout);

	connect(exportBtn, SIGNAL(clicked()), this, SLOT(Export_CSV_Button_Clicked()));
}

void CErrors_Tab_Widget::Export_CSV_Button_Clicked() {

	auto path = QFileDialog::getSaveFileName(this, tr(dsExport_CSV_Dialog_Title), dsExport_CSV_Default_File_Name, tr(dsExport_CSV_Ext_Spec));

	if (path.length() != 0) {
		std::ofstream fs(path.toStdString());

		int fromRow, fromCol, toRow, toCol;
		fromRow = 0;
		fromCol = 0;
		toRow = mTableView->model()->rowCount() - 1;
		toCol = mTableView->model()->columnCount() - 1;

		auto* selModel = mTableView->selectionModel();
		if (selModel->hasSelection()) {
			auto rowList = selModel->selectedRows();
			if (!rowList.empty()) {
				fromRow = mTableView->model()->rowCount() - 1;
				toRow = 0;

				for (auto& val : rowList) {
					if (val.row() > toRow) {
						toRow = val.row();
					}
					if (val.row() < fromRow) {
						fromRow = val.row();
					}
				}
			}

			auto colList = selModel->selectedColumns();
			if (!colList.empty()) {
				fromCol = mTableView->model()->columnCount() - 1;
				toCol = 0;

				for (auto& val : colList) {
					if (val.column() > toCol) {
						toCol = val.column();
					}
					if (val.column() < fromCol) {
						fromCol = val.column();
					}
				}
			}
		}

		// skip one column
		fs << ";";
		for (int j = fromCol; j <= toCol; j++) {
			fs << mTableView->model()->headerData(j, Qt::Orientation::Horizontal).toString().toStdString() << ";";
		}

		fs << std::endl;

		for (int i = fromRow; i <= toRow; i++) {
			fs << mTableView->model()->headerData(i, Qt::Orientation::Vertical).toString().toStdString() << ";";

			for (int j = fromCol; j <= toCol; j++) {
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

void CErrors_Tab_Widget::On_Filter_Configured(scgms::IFilter *filter) {
	if (mModel) {
		mModel->On_Filter_Configured(filter);
	}
}

void CErrors_Tab_Widget::Refresh() {
	if (mModel) {
		mModel->Update_Errors();
	}
}

void CErrors_Tab_Widget::Clear_Filters(bool wipeTable) {
	if (mModel) {
		mModel->Clear_Filters(wipeTable);
	}
}
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

#include "Model_Bounds_Panel.h"
#include "general_container_edit.h"

#include <scgms/lang/dstrings.h>
#include <scgms/rtl/UILib.h>
#include <scgms/utils/string_utils.h>

#include <QtWidgets/QLabel>
#include <QtGui/QDoubleValidator>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QApplication>

#include <array>

#include "moc_Model_Bounds_Panel.cpp"

CModel_Bounds_Panel_internal::CParameters_Table_Model::CParameters_Table_Model(QObject *parent) noexcept : QAbstractTableModel(parent) {
}

int CModel_Bounds_Panel_internal::CParameters_Table_Model::rowCount(const QModelIndex &parent) const {
	return static_cast<int>(mNames.size());
}

int CModel_Bounds_Panel_internal::CParameters_Table_Model::columnCount(const QModelIndex &parent) const {
	return 3;
}

double *CModel_Bounds_Panel_internal::CParameters_Table_Model::Get_Data(const int col) {
	double *data = nullptr;
	switch (col) {
		case 0:
			data = mLower_Bounds.data();
			break;
		case 1:
			data = mDefault_Values.data();
			break;
		case 2:
			data = mUpper_Bounds.data();
			break;
		default:
			break;
	}

	return data;
}

std::tuple<bool, size_t> CModel_Bounds_Panel_internal::CParameters_Table_Model::UI_Idx_To_Data_Idx(const int ui) const {

	if (ui > mNames.size()) {
		return { false, 0 };
	}

	if (mIndividualized_Segment_Count > 1) {
		const size_t rows_per_segment = mSegment_Specific_Parameter_Count + 1;	//+1 to denote the heading row
		
		if (ui > mNames.size() - mSegment_Agnostic_Parameter_Count) {	//ui points to the common parameters
			return { true, mDefault_Values.size() - mNames.size() + ui};
		}
		else {
			const size_t rem = ui % rows_per_segment;
			return { rem != 0, ui - (ui/ rows_per_segment) - 1};	//-1 to account the first string - "Segment 1"
		}
	}
	else {
		return { true, static_cast<size_t>(ui) };
	}
}

QVariant CModel_Bounds_Panel_internal::CParameters_Table_Model::data(const QModelIndex &index, int role) const {
	auto get_val = [this, &index](const size_t data_row)->double {
		switch (index.column()) {
			case 0:
				return mLower_Bounds[data_row];
			case 1:
				return mDefault_Values[data_row];
			case 2:
				return mUpper_Bounds[data_row];
			default:
				return std::numeric_limits<double>::quiet_NaN();
		}
	};

	if (role == Qt::DisplayRole || role == Qt::EditRole) {
		if (static_cast<size_t>(index.row()) >= mNames.size()) {
			return QVariant(std::numeric_limits<double>::quiet_NaN());
		}

		auto [non_empty_line, data_idx] = UI_Idx_To_Data_Idx(index.row());

		if (non_empty_line) {
			switch (mTypes[data_idx]) {
				case scgms::NModel_Parameter_Value::mptTime:
					return filter_config_window::CRatTime_Validator::rattime_to_string(get_val(data_idx));
				default:
					return QString::fromStdWString(dbl_2_wstr(get_val(data_idx)));
			}
		}
		else {
			return QVariant{};
		}

		return QVariant(std::numeric_limits<double>::quiet_NaN());
	}
	else {
		return QVariant{};
	}
}

bool CModel_Bounds_Panel_internal::CParameters_Table_Model::setData(const QModelIndex &index, const QVariant &value, int role) {
	if (static_cast<size_t>(index.row()) >= mNames.size()) {
		return false;
	}

	bool ok = false;
	const double val = value.toDouble(&ok);
	if (!ok) {
		return false;
	}

	auto [non_empty_line, data_idx] = UI_Idx_To_Data_Idx(index.row());

	if (non_empty_line) {
		Get_Data(index.column())[data_idx] = val;
	}

	return non_empty_line;
}

QVariant CModel_Bounds_Panel_internal::CParameters_Table_Model::headerData(int section, Qt::Orientation orientation, int role) const {
	const std::array<const char*, 3> column_names = {dsLower_Bounds, dsDefault_Parameters, dsUpper_Bounds};

	if (role == Qt::DisplayRole) {
		if (orientation == Qt::Horizontal) {
			if (section >= 0 && static_cast<size_t>(section) < column_names.size()) {
				return column_names[section];
			}
		}
		else if (orientation == Qt::Vertical) {
			if (section >= 0 && static_cast<size_t>(section) < mNames.size()) {
				return mNames[section];
			}
		}
	}

	return QVariant();
}

Qt::ItemFlags CModel_Bounds_Panel_internal::CParameters_Table_Model::flags(const QModelIndex & index) const {
	auto result = QAbstractTableModel::flags(index);
	
	auto [non_empty_line, data_idx] = UI_Idx_To_Data_Idx(index.row());
	if (non_empty_line) {
		result |= Qt::ItemIsEditable;
	}

	return result;
}

void CModel_Bounds_Panel_internal::CParameters_Table_Model::Load_Parameters(const scgms::TModel_Descriptor& model, const double* lower_bounds, const double* defaults, const double* upper_bounds, const size_t count) {
	beginResetModel();

	mNames.clear();
	mTypes.clear();
	mLower_Bounds.clear();
	mDefault_Values.clear();
	mUpper_Bounds.clear();

	if (model.id != Invalid_GUID) {
		mSegment_Agnostic_Parameter_Count = model.total_number_of_parameters - model.number_of_segment_specific_parameters;
		mSegment_Specific_Parameter_Count = model.number_of_segment_specific_parameters;
		size_t total_specific_parameters_in_doubles = model.number_of_segment_specific_parameters > 0 ? count - mSegment_Agnostic_Parameter_Count : 0;

		size_t segment_UI_idx = 1;

		bool valid_config = false;
		if (mSegment_Specific_Parameter_Count > 0) {
			valid_config = (total_specific_parameters_in_doubles % mSegment_Specific_Parameter_Count) == 0;
		}
		else {
			valid_config = count == model.total_number_of_parameters;
		}

		//check that the number of parameters is correct => check that they are not corrupted
		if (valid_config) {
			mIndividualized_Segment_Count = mSegment_Specific_Parameter_Count > 0 ? total_specific_parameters_in_doubles / mSegment_Specific_Parameter_Count : 0;

			bool common_params_str = false;
			for (size_t i = 0; i < count; i++) {
				//do we need to push a segment number name?
				if (mIndividualized_Segment_Count > 1) {
					if (i < total_specific_parameters_in_doubles) {
						if (i % mSegment_Specific_Parameter_Count == 0) {
							QString name{ dsSegment };
							name += QStringLiteral(" %1").arg(segment_UI_idx);
							mNames.push_back(name);
							segment_UI_idx++;
						}
					}
					else if ((i % mSegment_Specific_Parameter_Count == 0) && !common_params_str) {
						common_params_str = true;
						mNames.push_back(QString::fromWCharArray(dsCommon));
					}
				} 
		
				if (i < total_specific_parameters_in_doubles) {
					const size_t idx = i % mSegment_Specific_Parameter_Count;
					mTypes.push_back(model.parameter_types[idx]);
					mNames.push_back(QStringLiteral("{%1} %2").arg(segment_UI_idx-1).arg(QString::fromWCharArray(model.parameter_ui_names[idx])));
				}
				else {
					const size_t idx = i - total_specific_parameters_in_doubles + mSegment_Specific_Parameter_Count;
					mTypes.push_back(model.parameter_types[idx]);
					mNames.push_back(QString::fromWCharArray(model.parameter_ui_names[idx]));
				}
			}

			mLower_Bounds.assign(lower_bounds, lower_bounds + count);
			mDefault_Values.assign(defaults, defaults + count);
			mUpper_Bounds.assign(upper_bounds, upper_bounds + count);
		}
	}

	emit dataChanged(createIndex(0, 0), createIndex(rowCount() - 1, columnCount() - 1));

	endResetModel();
}

std::vector<double> CModel_Bounds_Panel_internal::CParameters_Table_Model::Store_Parameters() {
	std::vector<double> values;

	// "serialize" fields
	for (auto& container : { mLower_Bounds, mDefault_Values, mUpper_Bounds }) {
		for (size_t i = 0; i < container.size(); i++) {
			values.push_back(container[i]);
		}
	}

	return values;
}

CModel_Bounds_Panel_internal::CParameter_Value_Delegate::CParameter_Value_Delegate(CModel_Bounds_Panel_internal::CParameters_Table_Model* model, QObject* parent) :
	QItemDelegate(parent), mModel(model) {

}

QWidget* CModel_Bounds_Panel_internal::CParameter_Value_Delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,	const QModelIndex &index) const {
	
	auto [non_empty_line, data_idx] = mModel->UI_Idx_To_Data_Idx(index.row());

	QWidget* editor = nullptr;
	if (non_empty_line) {
		switch (mModel->mTypes[data_idx]) {
			case scgms::NModel_Parameter_Value::mptTime:
				editor = new filter_config_window::CRatTime_Container_Edit{ scgms::SFilter_Parameter{}, parent };
				break;
			default://double and bool
				editor = new filter_config_window::CDouble_Container_Edit{ scgms::SFilter_Parameter{}, parent };
				break;
		}
	}

	return editor;
}

void CModel_Bounds_Panel_internal::CParameter_Value_Delegate::setEditorData(QWidget *editor, const QModelIndex &index) const {	

	auto get_val = [this, &index](const size_t data_row)->double {
		switch (index.column()) {
			case 0:
				return mModel->mLower_Bounds[data_row];
			case 1:
				return mModel->mDefault_Values[data_row];
			case 2:
				return mModel->mUpper_Bounds[data_row];
			default:
				return std::numeric_limits<double>::quiet_NaN();
		}
	};
	
	auto [non_empty_line, data_row] = mModel->UI_Idx_To_Data_Idx(index.row());

	if (non_empty_line) {
		filter_config_window::IAs_Double_Container* true_editor = dynamic_cast<filter_config_window::IAs_Double_Container*>(editor);
		true_editor->set_double(get_val(data_row));
	}
}

void CModel_Bounds_Panel_internal::CParameter_Value_Delegate::setModelData(QWidget *editor, QAbstractItemModel *model,const QModelIndex &index) const {
	filter_config_window::IAs_Double_Container* true_editor = dynamic_cast<filter_config_window::IAs_Double_Container*>(editor);
	const double vv = true_editor->as_double();
	model->setData(index, QVariant(vv));
}

CModel_Bounds_Panel::CModel_Bounds_Panel(scgms::SFilter_Parameter parameter, QComboBox* modelSelector, const GUID& fixed_model, QWidget * parent)
	: CContainer_Edit(parameter), QWidget(parent), mModelSelector(modelSelector), mFixed_Model(fixed_model) {

	QVBoxLayout* layout = new QVBoxLayout();
	setLayout(layout);

	QWidget* contents = new QWidget();
	mLayout = new QVBoxLayout();
	
	mTableView = new QTableView();
	mModel = new CModel_Bounds_Panel_internal::CParameters_Table_Model(this);
	mTableView->setModel(mModel);

	auto delegate = new CModel_Bounds_Panel_internal::CParameter_Value_Delegate(mModel, this);
	mTableView->setItemDelegate(delegate);

	mTableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);

	mLayout->addWidget(mTableView);
	{
		QHBoxLayout *reset_layout = new QHBoxLayout();

		QString capt_root = dsReset_Bounds;
		capt_root +=" ";
		QPushButton* btn = new QPushButton(capt_root + dsLower_Bounds);
		connect(btn, SIGNAL(clicked()), this, SLOT(On_Reset_Lower()));
		reset_layout->addWidget(btn);

		btn = new QPushButton(capt_root + dsDefault_Parameters);
		connect(btn, SIGNAL(clicked()), this, SLOT(On_Reset_Defaults()));
		reset_layout->addWidget(btn);

		btn = new QPushButton(capt_root + dsUpper_Bounds);
		connect(btn, SIGNAL(clicked()), this, SLOT(On_Reset_Upper()));
		reset_layout->addWidget(btn);

		mLayout->addLayout(reset_layout);
	}

	contents->setLayout(mLayout);

	layout->addWidget(contents, Qt::AlignCenter);
	layout->addStretch();

	if (mModelSelector) {
		connect(mModelSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
			fetch_parameter();
		});
	}

	//	fetch_parameter(); - avoid possibly virtual call from ctor
}

void CModel_Bounds_Panel::store_parameter() {
	std::vector<double> values = mModel->Store_Parameters();
	
	check_rc(mParameter.set_double_array(values));
}


void CModel_Bounds_Panel::Reset_Parameters(std::vector<double> &values, std::function<const double*(const scgms::TModel_Descriptor&)> get_bounds) {
	scgms::TModel_Descriptor model = scgms::Null_Model_Descriptor;
	if (!Get_Currently_Selected_Model(model)) {
		return;
	}

	const double* bounds = get_bounds(model);
	
	//setup the segment-common/agnostic parameters	
	const size_t number_of_segment_common_parametes = model.total_number_of_parameters - model.number_of_segment_specific_parameters;
	const size_t first_common_parameter_in_values = values.size() - number_of_segment_common_parametes;

	size_t copy_start = 0;
	while (copy_start < first_common_parameter_in_values) {
		std::copy(bounds, bounds + model.number_of_segment_specific_parameters, values.begin() + copy_start);
		copy_start += model.number_of_segment_specific_parameters;
	}

	std::copy(bounds + model.number_of_segment_specific_parameters, bounds + model.total_number_of_parameters, values.end() - number_of_segment_common_parametes);

	mTableView->viewport()->update();
}

void CModel_Bounds_Panel::On_Reset_Lower() {
	Reset_Parameters(mModel->mLower_Bounds, [](const scgms::TModel_Descriptor& model)->const double* {return model.lower_bound; });
}

void CModel_Bounds_Panel::On_Reset_Defaults() {
	scgms::TModel_Descriptor model = scgms::Null_Model_Descriptor;
	if (!Get_Currently_Selected_Model(model)) {
		return;
	}

	//also trim down the number of segmetns to one
	mModel->mLower_Bounds.resize(model.total_number_of_parameters);
	mModel->mDefault_Values.resize(model.total_number_of_parameters);
	mModel->mUpper_Bounds.resize(model.total_number_of_parameters);


	mTableView->resizeColumnsToContents();
	mTableView->resizeRowsToContents();

	Reset_Parameters(mModel->mDefault_Values, [](const scgms::TModel_Descriptor& model)->const double* {
		return model.default_values;
	});
}

void CModel_Bounds_Panel::On_Reset_Upper() {
	Reset_Parameters(mModel->mUpper_Bounds, [](const scgms::TModel_Descriptor& model)->const double* {
		return model.upper_bound;
	});
}

bool CModel_Bounds_Panel::Get_Currently_Selected_Model(scgms::TModel_Descriptor& model) {
	GUID selectedModelGUID = Invalid_GUID;

	// get selected model GUID
	if (mModelSelector) {
		if (mModelSelector->currentIndex() >= 0) {
			selectedModelGUID = *reinterpret_cast<const GUID*>(mModelSelector->currentData().toByteArray().constData());
		}
	}
	else {
		selectedModelGUID = mFixed_Model;
	}

	return scgms::get_model_descriptor_by_id(selectedModelGUID, model);
}

void CModel_Bounds_Panel::fetch_parameter() {

	scgms::TModel_Descriptor model = scgms::Null_Model_Descriptor;
	if (Get_Currently_Selected_Model(model)) {
		//fetch default parameters
		double* lb = const_cast<double*>(model.lower_bound);
		double* def = const_cast<double*>(model.default_values);
		double* ub = const_cast<double*>(model.upper_bound);
		size_t param_count = model.total_number_of_parameters;

		//and try to load custom ones
		HRESULT rc;
		std::vector<double> parameters = mParameter.as_double_array(rc);
		
		if (Succeeded(rc)) {
			param_count = parameters.size() / 3;

			bool valid_param_size = parameters.size() % 3 == 0;	//OK, looks like we have low, def and upper
			//also check, if the number of parameters is actually OK when considering segment specific and segment agnostic parameters
			if (valid_param_size) {
				//beware, parameter-classification is not mandatory, hence it could be zero
				if (model.number_of_segment_specific_parameters > 0) {
					valid_param_size = (param_count - model.total_number_of_parameters + model.number_of_segment_specific_parameters) % model.number_of_segment_specific_parameters == 0;
				}
			}

			if (valid_param_size) {
				lb = parameters.data();
				def = lb + param_count;
				ub = lb + 2 * param_count;
			}
			else {
				//signalize the error!
				QMessageBox::warning(QApplication::activeWindow(), QString::fromWCharArray(dsError), QString::fromWCharArray(dsStored_Parameters_Corrupted_Not_Loaded));
			}
		}
		else {
			//ignore if we know that the parameter was not set yet
			if (rc != E_NOT_SET) {
				check_rc(rc);
			}
		}

		mModel->Load_Parameters(model, lb, def, ub, param_count);
	}
	else {
		mModel->Load_Parameters(scgms::Null_Model_Descriptor, nullptr, nullptr, nullptr, 0);
	}

	mTableView->resizeColumnsToContents();
	mTableView->resizeRowsToContents();
}

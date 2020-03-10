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

#include "Model_Bounds_Panel.h"

#include "../../../../common/lang/dstrings.h"
#include "../../../../common/rtl/UILib.h"

#include <QtWidgets/QLabel>
#include <QtGui/QDoubleValidator>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QApplication>

#include "moc_Model_Bounds_Panel.cpp"

CModel_Bounds_Panel::CModel_Bounds_Panel(scgms::SFilter_Parameter parameter, QComboBox* modelSelector, QWidget * parent)
	: CContainer_Edit(parameter), QWidget(parent), mModelSelector(modelSelector) {
	QVBoxLayout* layout = new QVBoxLayout();
	setLayout(layout);

	QWidget* contents = new QWidget();
	mLayout = new QGridLayout();
	contents->setLayout(mLayout);

	layout->addWidget(contents);
	layout->addStretch();

	connect(mModelSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
		fetch_parameter();
	});

	fetch_parameter();
}

void CModel_Bounds_Panel::store_parameter() {
	std::vector<double> values;

	// "serialize" fields
	for (auto& container : { mLowerBoundEdits, mDefaultsEdits, mUpperBoundEdits })
	{
		for (size_t i = 0; i < container.size(); i++) {			
			values.push_back(container[i]->as_double());
		}
	}

	check_rc(mParameter.set_double_array(values));
}

void CModel_Bounds_Panel::Reset_UI(const scgms::TModel_Descriptor& model, const double* lower_bounds, const double* defaults, const double* upper_bounds)
{
	// clear layout
	int colums = mLayout->columnCount();
	int rows = mLayout->rowCount();

	for (int j = 0; j < rows; j++)
	{
		for (int k = 0; k < colums; k++)
		{
			QLayoutItem* item = mLayout->itemAtPosition(j, k);

			if (!item)
				continue;

			mLayout->removeItem(item);
			delete item->widget();
			delete item;
		}
	}

	// add column headers
	mLayout->addWidget(new QLabel(dsLower_Bounds), 0, 1);
	mLayout->addWidget(new QLabel(dsDefault_Parameters), 0, 2);
	mLayout->addWidget(new QLabel(dsUpper_Bounds), 0, 3);

	auto create_edit = [this](const scgms::NModel_Parameter_Value parameter_type, const double val) -> filter_config_window::IAs_Double_Container* {

		filter_config_window::IAs_Double_Container* container = nullptr;

		switch (parameter_type) {
			case scgms::NModel_Parameter_Value::mptDouble:
				container = new filter_config_window::CDouble_Container_Edit{ scgms::SFilter_Parameter{}, this };
				break;

			case scgms::NModel_Parameter_Value::mptTime:
				container = new filter_config_window::CRatTime_Container_Edit{ scgms::SFilter_Parameter{}, this };
				break;		

			default: 
				break;
		}
		
		if (container) container->set_double(val);
		
		return container;
	};


	mLowerBoundEdits.clear();
	mDefaultsEdits.clear();
	mUpperBoundEdits.clear();

	for (size_t i = 0; i < model.number_of_parameters; i++) {			
		mLowerBoundEdits.push_back(create_edit(model.parameter_types[i], lower_bounds[i]));
		mDefaultsEdits.push_back(create_edit(model.parameter_types[i], defaults[i]));
		mUpperBoundEdits.push_back(create_edit(model.parameter_types[i], upper_bounds[i]));
	}

	for (int i = 0; i < static_cast<int>(model.number_of_parameters); i++) {
		mLayout->addWidget(new QLabel(QString::fromWCharArray(model.parameter_ui_names[i])), i + 1, 0);

		mLayout->addWidget(dynamic_cast<QWidget*>(mLowerBoundEdits[i]), i + 1, 1);
		mLayout->addWidget(dynamic_cast<QWidget*>(mDefaultsEdits[i]), i + 1, 2);
		mLayout->addWidget(dynamic_cast<QWidget*>(mUpperBoundEdits[i]), i + 1, 3);
	}

	// add reset buttons
	QPushButton* btn;

	btn = new QPushButton(dsReset_Bounds);
	connect(btn, SIGNAL(clicked()), this, SLOT(On_Reset_Lower()));
	mLayout->addWidget(btn, static_cast<int>(model.number_of_parameters+ 1), 1);

	btn = new QPushButton(dsReset_Bounds);
	connect(btn, SIGNAL(clicked()), this, SLOT(On_Reset_Defaults()));
	mLayout->addWidget(btn, static_cast<int>(model.number_of_parameters + 1), 2);

	btn = new QPushButton(dsReset_Bounds);
	connect(btn, SIGNAL(clicked()), this, SLOT(On_Reset_Upper()));
	mLayout->addWidget(btn, static_cast<int>(model.number_of_parameters + 1), 3);

	for (int i = 0; i < static_cast<int>(model.number_of_parameters) + 2; i++)
		mLayout->setRowStretch((int)i, 1);
}

void CModel_Bounds_Panel::Reset_Parameters(const std::vector<filter_config_window::IAs_Double_Container*> &containers, std::function<const double*(const scgms::TModel_Descriptor&)> get_bounds) {
	scgms::TModel_Descriptor model = scgms::Null_Model_Descriptor;
	if (!Get_Current_Selected_Model(model))
		return;

	const double* bounds = get_bounds(model);

	for (size_t i = 0; i < model.number_of_parameters; i++) {		
		containers[i]->set_double(bounds[i]);
	}		
}

void CModel_Bounds_Panel::On_Reset_Lower() {
	Reset_Parameters(mLowerBoundEdits, [](const scgms::TModel_Descriptor& model)->const double* {return model.lower_bound; });
}

void CModel_Bounds_Panel::On_Reset_Defaults() {
	Reset_Parameters(mDefaultsEdits, [](const scgms::TModel_Descriptor& model)->const double* {return model.default_values; });
}

void CModel_Bounds_Panel::On_Reset_Upper() {
	Reset_Parameters(mUpperBoundEdits, [](const scgms::TModel_Descriptor& model)->const double* {return model.upper_bound; });
}

bool CModel_Bounds_Panel::Get_Current_Selected_Model(scgms::TModel_Descriptor& model)
{
	if (mModelSelector->currentIndex() >= 0)
	{
		// get selected model GUID
		const GUID selectedModelGUID = *reinterpret_cast<const GUID*>(mModelSelector->currentData().toByteArray().constData());

		if (scgms::get_model_descriptor_by_id(selectedModelGUID, model))
			return true;
	}

	return false;
}

void CModel_Bounds_Panel::fetch_parameter() {

	scgms::TModel_Descriptor model = scgms::Null_Model_Descriptor;
	if (Get_Current_Selected_Model(model)) {
		//fetch default parameters
		double* lb = const_cast<double*>(model.lower_bound);
		double* def = const_cast<double*>(model.default_values);
		double* ub = const_cast<double*>(model.upper_bound);

		//and try to load custom ones
		HRESULT rc;
		std::vector<double> parameters = mParameter.as_double_array(rc);
		
		if (SUCCEEDED(rc)) {
			if (parameters.size() == model.number_of_parameters * 3) {
				lb = parameters.data();
				def = lb + model.number_of_parameters;
				ub = lb + 2 * model.number_of_parameters;
			}
			else
				//signalize the error!
				QMessageBox::warning(QApplication::activeWindow(), QString::fromWCharArray(dsError), QString::fromWCharArray(dsStored_Parameters_Corrupted_Not_Loaded));
		} else
			if (rc != E_NOT_SET)		//ignore if we know that the parameter was not set yet
				check_rc(rc);

		Reset_UI(model, lb, def, ub);
	}			
}

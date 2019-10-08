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

#include "filter_config_window.h"

#include "../../../common/lang/dstrings.h"
#include "../../../common/rtl/manufactory.h"
#include "../../../common/rtl/referencedImpl.h"
#include "../../../common/rtl/FilterLib.h"
#include "../../../common/rtl/UILib.h"
#include "../../../common/utils/QtUtils.h"

#include "helpers/Select_Time_Segment_Id_Panel.h"
#include "helpers/Model_Bounds_Panel.h"
#include "helpers/Select_Subject_Panel.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>


#include "moc_filter_config_window.cpp"

#include "helpers/filter_config_widgets.h"
	



CFilter_Config_Window::CFilter_Config_Window(glucose::SFilter_Configuration_Link configuration, QWidget *parent) :
	QDialog(parent), mConfiguration(refcnt::make_shared_reference_ext<glucose::SFilter_Configuration, glucose::IFilter_Configuration>(static_cast<glucose::IFilter_Configuration*>(configuration.get()), true)), mDescription(configuration.descriptor()) {


	Setup_UI(configuration);
	for (const auto &edit : mContainer_Edits)
		edit->fetch_parameter();
}

void CFilter_Config_Window::Setup_UI(glucose::SFilter_Configuration_Link configuration) {

	setWindowTitle(QString::fromWCharArray(mDescription.description) + QString(" ") + tr(dsConfiguration));

	QTabWidget *tabs = new QTabWidget{this};

	// model select combobox is directly connected with signal selector; here we store pointer to model selector to supply it to signal selector
	filter_config_window::CContainer_Edit *model_select = nullptr;

	auto create_model_select = [&](glucose::SFilter_Parameter parameter) {
		model_select = new CGUID_Entity_ComboBox<glucose::TModel_Descriptor, glucose::get_model_descriptors>(parameter, nullptr);
	};

	QWidget *main_tab = new QWidget{this};
	{
		const int idxName_col = 0;
		const int idxEdit_col = 1;

		QGridLayout *main_layout = new QGridLayout();
		int ui_row = 0;
		for (int i = 0; i < static_cast<int>(mDescription.parameters_count); i++) {
			//try to obtain the parameter, if is present in the configuration
			glucose::SFilter_Parameter parameter = configuration.Resolve_Parameter(mDescription.config_parameter_name[i]);
			if ((!parameter) && (mDescription.parameter_type[i] != glucose::NParameter_Type::ptNull)) {
				//this particular parameter is not configured, hence we need to create it using its default value
				//unless it is a null parameter
				parameter = configuration.Add_Parameter(mDescription.parameter_type[i], mDescription.config_parameter_name[i]);
				if (!parameter) continue;	//no way to add it, so let's just ignore it and do not configure it
			}

			auto add_edit_control = [&]() {
				filter_config_window::CContainer_Edit *container = nullptr;

				switch (mDescription.parameter_type[i])
				{
					case glucose::NParameter_Type::ptNull:
						container = new filter_config_window::CNull_Container_Edit(this);
						break;

					case glucose::NParameter_Type::ptWChar_Container:
						container = new filter_config_window::CWChar_Container_Edit{ parameter, this };
						break;

					case glucose::NParameter_Type::ptDouble:
						container = new filter_config_window::CDouble_Container_Edit{ parameter, this};
						break;

					case glucose::NParameter_Type::ptRatTime:
						container = new filter_config_window::CRatTime_Container_Edit{ parameter, this };
						break;

					case glucose::NParameter_Type::ptInt64:
						container = new filter_config_window::CInteger_Container_Edit{ parameter, this };
						break;

					case glucose::NParameter_Type::ptBool:
						container = new filter_config_window::CBoolean_Container_Edit{ parameter, this};
						break;
					
					case glucose::NParameter_Type::ptSelect_Time_Segment_ID:
						container = new CSelect_Time_Segment_Id_Panel{ mConfiguration, parameter, this };
						break;

					case glucose::NParameter_Type::ptModel_Id:
						// "lazyload" of model selection; if the filter has model selection, it is very likely that it has signal selection as well
						if (!model_select)
							create_model_select(parameter);

						container = model_select;
						break;

					case glucose::NParameter_Type::ptMetric_Id:
						container = new CGUID_Entity_ComboBox<glucose::TMetric_Descriptor, glucose::get_metric_descriptors>(parameter, this);
						break;

					case glucose::NParameter_Type::ptSolver_Id:
						container = new CGUID_Entity_ComboBox<glucose::TSolver_Descriptor, glucose::get_solver_descriptors>(parameter, this);
						break;

					case glucose::NParameter_Type::ptDevice_Driver_Id:
						container = new CGUID_Entity_ComboBox<glucose::TDevice_Driver_Descriptor, glucose::get_device_driver_descriptors>(parameter, this);
						break;

					case glucose::NParameter_Type::ptModel_Signal_Id:
						// signal selection always requires model selection field
						if (!model_select)
							create_model_select(parameter);

						container = new CModel_Signal_Select_ComboBox(parameter, nullptr, dynamic_cast<QComboBox*>(model_select));
						break;

					case glucose::NParameter_Type::ptSignal_Id:
						container = new CAvailable_Signal_Select_ComboBox(parameter, nullptr);
						break;

					case glucose::NParameter_Type::ptModel_Bounds:
						// model bounds edit always requires model selection field
						if (!model_select)
							create_model_select(parameter);

						container = new CModel_Bounds_Panel(parameter, dynamic_cast<QComboBox*>(model_select), this);
						break;

					case glucose::NParameter_Type::ptSubject_Id:
						container = new CSelect_Subject_Panel{ mConfiguration, parameter, this };
						break;
				}

				if (mDescription.parameter_type[i] != glucose::NParameter_Type::ptNull) mContainer_Edits.push_back(container);
				switch (mDescription.parameter_type[i])
				{
					//special widget, let's add it as a standalone tab
					case glucose::NParameter_Type::ptSelect_Time_Segment_ID:
					case glucose::NParameter_Type::ptModel_Bounds:
					case glucose::NParameter_Type::ptSubject_Id:
					{
						tabs->addTab(dynamic_cast<QWidget*>(container), QString::fromWCharArray(mDescription.ui_parameter_name[i]));
						break;
					}
					default:
					{
						QLabel *label = new QLabel{ QString::fromWCharArray(mDescription.ui_parameter_name[i]) };
						// consider null parameter as separator and make some style adjustments
						if (mDescription.parameter_type[i] == glucose::NParameter_Type::ptNull)
							label->setText("<b>" + label->text() + "</b>");

						if (mDescription.ui_parameter_tooltip && mDescription.ui_parameter_tooltip[i])
							label->setToolTip(QString::fromWCharArray(mDescription.ui_parameter_tooltip[i]));

						main_layout->addWidget(label, ui_row, idxName_col);
						main_layout->addWidget(dynamic_cast<QWidget*>(container), ui_row, idxEdit_col);
						ui_row++;
						break;
					}
				}

			};
			
			add_edit_control();
		}
		main_tab->setLayout(main_layout);
		tabs->insertTab(0, main_tab, tr(dsMain_Parameters));	//insert makes the main edits to be first
		tabs->setCurrentIndex(0);
	}

	QHBoxLayout *button_layout = new QHBoxLayout{};
	{

		button_layout->setAlignment(Qt::AlignRight);

		QPushButton *btnOK = new QPushButton{ tr(dsOK) };
		QPushButton *btnCancel = new QPushButton{ tr(dsCancel) };
		QPushButton *btnApply = new QPushButton{ tr(dsApply) };

		button_layout->addWidget(btnOK);
		button_layout->addWidget(btnCancel);
		button_layout->addWidget(btnApply);

		connect(btnOK, SIGNAL(clicked()), this, SLOT(On_OK()));
		connect(btnCancel, SIGNAL(clicked()), this, SLOT(On_Cancel()));
		connect(btnApply, SIGNAL(clicked()), this, SLOT(On_Apply()));
	}

	QVBoxLayout *final_layout = new QVBoxLayout{};
	final_layout->addWidget(tabs);
	final_layout->addLayout(button_layout);
	
	setLayout(final_layout);


	setWindowModality(Qt::ApplicationModal);
	//set the window to be freed upon closing
	setAttribute(Qt::WA_DeleteOnClose, true);
}

void CFilter_Config_Window::Commit_Parameters() {	
	for (auto &edit : mContainer_Edits) 
		edit->store_parameter();
}

void CFilter_Config_Window::On_OK() {
	Commit_Parameters();
	close();
}

void CFilter_Config_Window::On_Cancel() {
	close();
}

void CFilter_Config_Window::On_Apply() {
	Commit_Parameters();	
}

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

#include "filter_config_widgets.h"

void CModel_Signal_Select_ComboBox::Refresh_Contents() {

	// always clear contents
	clear();

	if (mModelSelector->currentIndex() >= 0) {

		// get selected model GUID
		scgms::TModel_Descriptor model = scgms::Null_Model_Descriptor;
		const GUID selectedModelGUID = *reinterpret_cast<const GUID*>(mModelSelector->currentData().toByteArray().constData());

		// retrieve proper model
		if (scgms::get_model_descriptor_by_id(selectedModelGUID, model)) {
			// add model signals to combobox
			for (size_t i = 0; i < model.number_of_calculated_signals; i++) {
				const std::wstring sig_name = mSignal_Descriptors.Get_Name(model.calculated_signal_ids[i]);
				addItem(StdWStringToQString(sig_name), QVariant{ QByteArray(reinterpret_cast<const char*>(&model.calculated_signal_ids[i]), sizeof(GUID)) });
			}
		}
	}
}

CModel_Signal_Select_ComboBox::CModel_Signal_Select_ComboBox(scgms::SFilter_Parameter parameter, QWidget *parent, QComboBox *modelSelector) :
	filter_config_window::CGUIDCombo_Container_Edit(parameter, parent), mModelSelector(modelSelector) {
	Refresh_Contents();

	connect(mModelSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
		Refresh_Contents();
	});
}

CAvailable_Signal_Select_ComboBox::CAvailable_Signal_Select_ComboBox(scgms::SFilter_Parameter parameter, QWidget *parent) : filter_config_window::CGUIDCombo_Container_Edit(parameter, parent) {

	const scgms::CSignal_Description signal_descriptors{};
	signal_descriptors.for_each([this](const scgms::TSignal_Descriptor& desc) {
		addItem(StdWStringToQString(desc.signal_description), QVariant{ QByteArray{reinterpret_cast<const char*>(&desc.id), sizeof(decltype(desc.id))} });
	});

	model()->sort(0);
	setCurrentIndex(0);
}

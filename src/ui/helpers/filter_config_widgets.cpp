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

#include "filter_config_widgets.h"

void CModel_Signal_Select_ComboBox::Refresh_Contents()
{
	// always clear contents
	clear();

	if (mModelSelector->currentIndex() >= 0)
	{
		// get selected model GUID
		glucose::TModel_Descriptor model = glucose::Null_Model_Descriptor;
		const GUID selectedModelGUID = *reinterpret_cast<const GUID*>(mModelSelector->currentData().toByteArray().constData());

		// retrieve proper model
		if (glucose::get_model_descriptor_by_id(selectedModelGUID, model))
		{
			// add model signals to combobox
			for (size_t i = 0; i < model.number_of_calculated_signals; i++)
				addItem(StdWStringToQString(model.calculated_signal_names[i]), QVariant{ QByteArray(reinterpret_cast<const char*>(&model.calculated_signal_ids[i])) });
		}
	}
}

CModel_Signal_Select_ComboBox::CModel_Signal_Select_ComboBox(glucose::SFilter_Parameter parameter, QWidget *parent, QComboBox *modelSelector) :
	filter_config_window::CGUIDCombo_Container_Edit(parameter, parent), mModelSelector(modelSelector) {	
	Refresh_Contents();

	connect(mModelSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
		Refresh_Contents();
	});
}

CAvailable_Signal_Select_ComboBox::CAvailable_Signal_Select_ComboBox(glucose::SFilter_Parameter parameter, QWidget *parent)	: filter_config_window::CGUIDCombo_Container_Edit(parameter, parent) {
	// append measured signals
	std::wstring measSuffix = dsSignal_Suffix_Measured;
	measSuffix = L" (" + measSuffix + L")";

	//TO DO: enumerate input known signals in some global header
	mSignalVector.push_back({ glucose::signal_BG, dsSignal_Measured_BG + measSuffix });
	mSignalVector.push_back({ glucose::signal_IG, dsSignal_Measured_IG + measSuffix });
	mSignalVector.push_back({ glucose::signal_ISIG, dsSignal_Measured_ISIG + measSuffix });
	mSignalVector.push_back({ glucose::signal_Calibration, dsSignal_Measured_Calibration + measSuffix });
	mSignalVector.push_back({ glucose::signal_Bolus_Insulin, dsSignal_Measured_Bolus_Insulin + measSuffix });
	mSignalVector.push_back({ glucose::signal_Basal_Insulin, dsSignal_Measured_Basal_Insulin + measSuffix });
	mSignalVector.push_back({ glucose::signal_Basal_Insulin_Rate, dsSignal_Measured_Basal_Insulin_Rate + measSuffix });
	mSignalVector.push_back({ glucose::signal_Insulin_Activity, dsSignal_Measured_Insulin_Activity + measSuffix });
	mSignalVector.push_back({ glucose::signal_IOB, dsSignal_Measured_IOB + measSuffix });
	mSignalVector.push_back({ glucose::signal_COB, dsSignal_Measured_COB + measSuffix });
	mSignalVector.push_back({ glucose::signal_Carb_Intake, dsSignal_Measured_Carb_Intake + measSuffix });
	mSignalVector.push_back({ glucose::signal_Physical_Activity, dsSignal_Measured_Health_Physical_Activity + measSuffix });

	// append calculated signals of known models
	std::wstring calcSuffix = dsSignal_Suffix_Calculated;
	calcSuffix = L" (" + calcSuffix + L")";

	auto models = glucose::get_model_descriptors();
	for (auto& model : models)
	{
		for (size_t i = 0; i < model.number_of_calculated_signals; i++)
			mSignalVector.push_back({ model.calculated_signal_ids[i], model.description + std::wstring(L" - ") + model.calculated_signal_names[i] + calcSuffix });
	}

	// append virtual signals
	for (size_t i=0; i<glucose::signal_Virtual.size(); i++)
		mSignalVector.push_back({ glucose::signal_Virtual[i], dsSignal_Prefix_Virtual + std::wstring(L" ") + std::to_wstring(i) });

	// add all signals to combobox

	for (auto const& signal : mSignalVector)
		addItem(StdWStringToQString(signal.second), QVariant{ QByteArray(reinterpret_cast<const char*>(&signal.first)) });
}
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

#include "FilterListItem.h"
#include <scgms/rtl/UILib.h>

#include <QtCore/QObject>

CFilter_List_Item::CFilter_List_Item(scgms::SFilter_Configuration_Link configuration) :
	QListWidgetItem(), mDescriptor(configuration.descriptor()), mConfiguration(configuration) {

	Refresh();
}

CFilter_List_Item::CFilter_List_Item(const scgms::TFilter_Descriptor descriptor) :QListWidgetItem(), mDescriptor(descriptor) {
	Refresh();
}

scgms::SFilter_Configuration_Link CFilter_List_Item::configuration() {
	return mConfiguration;
}

const scgms::TFilter_Descriptor& CFilter_List_Item::description() const {
	return mDescriptor;
}

void CFilter_List_Item::Refresh() {

	QString text = QString::fromWCharArray(mDescriptor.description);

	auto models = scgms::get_model_descriptor_list();

	// splitter appending logic - at first, apply " - " to split name from description, then apply ", " to split description items
	bool splitterAppended = false;
	auto appendSplitter = [&]() {
		if (splitterAppended) {
			text += QString(", ");
			return;
		}

		text += QString(" - ");
		splitterAppended = true;
	};

	//variables for signal mappers, etc.
	bool src_signal_set = false;
	bool dst_signal_set = false;
	bool src_file_set = false;
	bool reference_signal_set = false;
	std::wstring src_signal_str, dst_signal_str, src_file_str;

	// traverse configuration and find items to put into description
	//for (auto& cfg : mConfiguration)
	if (mConfiguration) {
		mConfiguration.for_each([&](scgms::SFilter_Parameter cfg) {

			switch (cfg.type()) {
				
				// model signal - append signal name
				case scgms::NParameter_Type::ptModel_Produced_Signal_Id:
				{
					bool found = false;
					for (auto& model : models) {
						for (size_t i = 0; i < model.number_of_calculated_signals; i++) {
							HRESULT rc;
							if (model.calculated_signal_ids[i] == cfg.as_guid(rc)) {
								if (rc == S_OK) {
									appendSplitter();
									const std::wstring sig_name = mSignal_Descriptors.Get_Name(model.calculated_signal_ids[i]);
									text += QString::fromWCharArray(sig_name.c_str());  //model.calculated_signal_names[i]);
									found = true;
									break;
								}
							}
						}

						if (found) {
							break;
						}
					}
					break;
				}
				// model - append model description
				case scgms::NParameter_Type::ptSignal_Model_Id:
				case scgms::NParameter_Type::ptDiscrete_Model_Id:
				{
					for (auto& model : models) {
						HRESULT rc;
						if (model.id == cfg.as_guid(rc)) {
							if (rc == S_OK) {
								appendSplitter();
								text += QString::fromWCharArray(model.description);
								break;
							}
						}
					}
					break;
				}
				//e.g., masking, mapping and decoupling filters
				case scgms::NParameter_Type::ptSignal_Id:
				{
					auto get_sig_name = [&](bool& ok) {
						HRESULT rc;
						std::wstring sig_name = mSignal_Descriptors.Get_Name(cfg.as_guid(rc));
						ok = rc == S_OK;
						return sig_name;
					};

					const std::wstring cfg_name { cfg.configuration_name() };	//converts from wchar_t*!

					if (cfg_name == rsReference_Signal) {
						reference_signal_set = true;
					}

					if ((cfg_name == rsSignal_Source_Id) || (cfg_name == rsSelected_Signal) || (cfg_name == rsReference_Signal)) {
						src_signal_str = get_sig_name(src_signal_set);
					}
					else if ((cfg_name == rsSignal_Destination_Id) || (cfg_name == rsError_Signal)) {
						dst_signal_str = get_sig_name(dst_signal_set);
					}

					break;
				}
				case scgms::NParameter_Type::ptWChar_Array:
				{
					const std::wstring cfg_name{ cfg.configuration_name() };	//converts from wchar_t*!
					if (cfg_name == L"Source_File") {
						HRESULT rc;
						src_file_str = cfg.as_filepath(rc).filename().wstring();
						src_file_set = (rc == S_OK) && (!src_file_str.empty());
					}

					break;
				}
				default:
					break;	//keepping compiler happy
			}
		});
	}

	if (src_signal_set) {
		appendSplitter();
		text += QString::fromStdWString(src_signal_str);
		if (dst_signal_set) {
			text += QString::fromWCharArray(reference_signal_set ?  L" vs. " : L" -> ");
			text += QString::fromStdWString(dst_signal_str);
		}
	}

	if (src_file_set) {
		appendSplitter();
		text += QString::fromStdWString(src_file_str);
	}

	setText(text);
}

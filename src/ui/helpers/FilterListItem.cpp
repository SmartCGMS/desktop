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

#include "FilterListItem.h"
#include "../../../../common/rtl/UILib.h"

#include <QtCore/QObject>

CFilter_List_Item::CFilter_List_Item(scgms::SFilter_Configuration_Link configuration) :
	QListWidgetItem(), mConfiguration(configuration), mDescriptor(configuration.descriptor()) {

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

void CFilter_List_Item::Refresh()
{
	QString text = QString::fromWCharArray(mDescriptor.description);

	auto models = scgms::get_model_descriptors();

	// splitter appending logic - at first, apply " - " to split name from description, then apply ", " to split description items
	bool splitterAppended = false;
	auto appendSplitter = [&]() {
		if (splitterAppended)
		{
			text += QString(", ");
			return;
		}

		text += QString(" - ");
		splitterAppended = true;
	};

	// traverse configuration and find items to put into description
	//for (auto& cfg : mConfiguration)
	if (mConfiguration) {
		mConfiguration.for_each([&](scgms::SFilter_Parameter cfg)
			{
				// model signal - append signal name
				if (cfg.type() == scgms::NParameter_Type::ptModel_Signal_Id) {
					bool found = false;
					for (auto& model : models)
					{
						for (size_t i = 0; i < model.number_of_calculated_signals; i++) {
							HRESULT rc;
							if (model.calculated_signal_ids[i] == cfg.as_guid(rc))
								if (rc == S_OK) {
									appendSplitter();
									text += QString::fromWCharArray(model.calculated_signal_names[i]);
									found = true;
									break;
								}
						}

						if (found)
							break;
					}
				}
				// model - append model description
				else if (cfg.type() == scgms::NParameter_Type::ptModel_Id) {
					for (auto& model : models) {
						HRESULT rc;
						if (model.id == cfg.as_guid(rc))
							if (rc == S_OK) {
								appendSplitter();
								text += QString::fromWCharArray(model.description);
								break;
							}
					}
				}
			}	
		);
	}

	setText(text);
}

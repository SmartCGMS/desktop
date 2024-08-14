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

#pragma once

#include "../filter_config_window.h"

#include <scgms/lang/dstrings.h>
#include <scgms/rtl/manufactory.h>
#include <scgms/rtl/referencedImpl.h>
#include <scgms/rtl/FilterLib.h>
#include <scgms/rtl/UILib.h>
#include <scgms/utils/QtUtils.h>

#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>

/*
 * Template class for selection of object identified by GUID (id), having description and generic getter function
 * This is suitable for model, solver and metric comboboxes
 */
template <typename TDesc, typename std::vector<TDesc>(*G)(), typename TFilter = std::function<bool(const TDesc&)>>
class CGUID_Entity_ComboBox : public filter_config_window::CGUIDCombo_Container_Edit {
	public:
		CGUID_Entity_ComboBox(scgms::SFilter_Parameter parameter, QWidget *parent, TFilter filter = TFilter()) : CGUIDCombo_Container_Edit(parameter, parent) {
			auto entities = G();

			// add entities retrieved using template function
			for (const auto& entity : entities) {
				if (!filter || filter(entity)) {
					addItem(StdWStringToQString(entity.description), QVariant{ QByteArray(reinterpret_cast<const char*>(&entity.id), sizeof(GUID)) });
				}
			}

			HRESULT rc;
			const GUID selected_id = parameter.as_guid(rc);
			if (Succeeded(rc)) {
				const QVariant data{ QByteArray(reinterpret_cast<const char*>(&selected_id), sizeof(GUID)) };
				setCurrentIndex(findData(data));
			}
			//else it fails later on
		}
};

/*
 * Class for discrete/signal model selection; it specializes generic GUID combobox with a filter
 */
class CModel_Select_ComboBox : public CGUID_Entity_ComboBox<scgms::TModel_Descriptor, scgms::get_model_descriptor_list, bool(*)(const scgms::TModel_Descriptor&)> {
	public:
		CModel_Select_ComboBox(scgms::SFilter_Parameter parameter, QWidget* parent, bool discrete)
			: CGUID_Entity_ComboBox(parameter, parent, discrete ? &CModel_Select_ComboBox::Model_Filter_Discrete : &CModel_Select_ComboBox::Model_Filter_Signal) {
			//
		}

		static bool Model_Filter_Discrete(const scgms::TModel_Descriptor& desc) {
			return (desc.flags & scgms::NModel_Flags::Discrete_Model) != scgms::NModel_Flags::None;
		}

		static bool Model_Filter_Signal(const scgms::TModel_Descriptor& desc) {
			return (desc.flags & scgms::NModel_Flags::Signal_Model) != scgms::NModel_Flags::None;
		}
};


/*
 * Model-dependent signal ID selection combobox; connected with model selector (for appropriate signal selection)
 */
class CModel_Signal_Select_ComboBox : public filter_config_window::CGUIDCombo_Container_Edit {
	private:
		// connected model selector combobox
		const QComboBox *mModelSelector;
		const scgms::CSignal_Description mSignal_Descriptors{};

	protected:
		// refreshes combobox contents using model selector value
		void Refresh_Contents();

	public:
		CModel_Signal_Select_ComboBox(scgms::SFilter_Parameter parameter, QWidget *parent, QComboBox *modelSelector);
};

/*
 * All available signal ID selection combobox; connected with model selector (for appropriate signal selection)
 */
class CAvailable_Signal_Select_ComboBox : public filter_config_window::CGUIDCombo_Container_Edit {
	public:
		CAvailable_Signal_Select_ComboBox(scgms::SFilter_Parameter parameter, QWidget *parent);
};

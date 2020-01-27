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

#pragma once

#include "../filter_config_window.h"

#include "../../../../common/lang/dstrings.h"
#include "../../../../common/rtl/manufactory.h"
#include "../../../../common/rtl/referencedImpl.h"
#include "../../../../common/rtl/FilterLib.h"
#include "../../../../common/rtl/UILib.h"
#include "../../../../common/utils/QtUtils.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>

/*
 * Template class for selection of object identified by GUID (id), having description and generic getter function
 * This is suitable for model, solver and metric comboboxes
 */
template <typename TDesc, typename std::vector<TDesc>(*G)()>
class CGUID_Entity_ComboBox : public virtual filter_config_window::CGUIDCombo_Container_Edit {
public:
	CGUID_Entity_ComboBox(scgms::SFilter_Parameter parameter, QWidget *parent) : CGUIDCombo_Container_Edit(parameter, parent) {		
		auto entities = G();

		// add entities retrieved using template function
		for (auto entity : entities)
			addItem(StdWStringToQString(entity.description), QVariant{ QByteArray(reinterpret_cast<const char*>(&entity.id)) });
	}			
};

/*
 * Model-dependent signal ID selection combobox; connected with model selector (for appropriate signal selection)
 */
class CModel_Signal_Select_ComboBox : public virtual filter_config_window::CGUIDCombo_Container_Edit {
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
class CAvailable_Signal_Select_ComboBox : public virtual filter_config_window::CGUIDCombo_Container_Edit {
private:
	// we need to preserve order, so we use vector of pairs instead of map; we don't use advantages of map anyways
	std::vector<std::pair<GUID, std::wstring>> mSignalVector;    
public:
	CAvailable_Signal_Select_ComboBox(scgms::SFilter_Parameter parameter, QWidget *parent);		
};

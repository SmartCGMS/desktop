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

#include "general_container_edit.h"

#include "../../../../common/rtl/FilterLib.h"

#include <map>

#include <QtWidgets/QWidget>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLineEdit>

/*
 * Panel for selecting and setting model bounds and default parameters
 */
class CModel_Bounds_Panel : public QWidget, public virtual filter_config_window::CContainer_Edit
{
	Q_OBJECT

	protected:
		// connected model selector
		QComboBox *mModelSelector;
		// inner layout
		QGridLayout* mLayout;

		// stored edits
		std::vector<filter_config_window::IAs_Double_Container*> mLowerBoundEdits, mDefaultsEdits, mUpperBoundEdits;

		// resets user interface using given values
		void Reset_UI(const glucose::TModel_Descriptor& model, const double* lower_bounds, const double* defaults, const double* upper_bounds);
		// retrieves currently selected model; returns true on success
		bool Get_Current_Selected_Model(glucose::TModel_Descriptor& model);

		void CModel_Bounds_Panel::Reset_Parameters(const std::vector<filter_config_window::IAs_Double_Container*> &containers, std::function<const double*(const glucose::TModel_Descriptor&)> get_bounds);
	protected slots:
		void On_Reset_Lower();
		void On_Reset_Defaults();
		void On_Reset_Upper();

	public:
		CModel_Bounds_Panel(glucose::SFilter_Parameter parameter, QComboBox* modelSelector, QWidget *parent);

		virtual void fetch_parameter() override;
		virtual void store_parameter() override;
};
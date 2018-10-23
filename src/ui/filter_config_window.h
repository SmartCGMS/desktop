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
 *       monitoring", Proceedings of the 8th International Conference on Current
 *       and Future Trends of Information and Communication Technologies
 *       in Healthcare (ICTH 2018) November 5-8, 2018, Leuven, Belgium
 */

#pragma once

#include "../../../common/rtl/FilterLib.h"
#include "../../../common/desktop-console/filter_chain.h"
#include "helpers/general_container_edit.h"

#include <vector>

#include <QtWidgets/QWidget>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLineEdit>

class CFilter_Config_Window : public QDialog {
	Q_OBJECT
protected:
	CFilter_Configuration &mConfiguration;
	const glucose::TFilter_Descriptor &mDescription;

	using TName_Container_Pair = std::pair<std::wstring, filter_config_window::CContainer_Edit*>;

	std::vector<TName_Container_Pair> mContainer_Edits;

	void Setup_UI();
	void Commit_Parameters();	//from controls to configuration
protected slots:
	void On_OK();
	void On_Cancel();
	void On_Apply();
public:
	CFilter_Config_Window(const glucose::TFilter_Descriptor &description, CFilter_Configuration &configuration, QWidget *parent);
};

class CInteger_Container_Edit : public QLineEdit, public virtual filter_config_window::CContainer_Edit {
	Q_OBJECT
public:
	CInteger_Container_Edit(QWidget *parent);
	glucose::TFilter_Parameter get_parameter();
	void set_parameter(const glucose::TFilter_Parameter &param);
};

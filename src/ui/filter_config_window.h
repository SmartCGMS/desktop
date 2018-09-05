/**
 * SmartCGMS - continuous glucose monitoring and controlling framework
 * https://diabetes.zcu.cz/
 *
 * Contact:
 * diabetes@mail.kiv.zcu.cz
 * Medical Informatics, Department of Computer Science and Engineering
 * Faculty of Applied Sciences, University of West Bohemia
 * Technicka 8
 * 314 06, Pilsen
 *
 * Licensing terms:
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * a) For non-profit, academic research, this software is available under the
 *    GPLv3 license. When publishing any related work, user of this software
 *    must:
 *    1) let us know about the publication,
 *    2) acknowledge this software and respective literature - see the
 *       https://diabetes.zcu.cz/about#publications,
 *    3) At least, the user of this software must cite the following paper:
 *       Parallel software architecture for the next generation of glucose
 *       monitoring, Proceedings of the 8th International Conference on Current
 *       and Future Trends of Information and Communication Technologies
 *       in Healthcare (ICTH 2018) November 5-8, 2018, Leuven, Belgium
 * b) For any other use, especially commercial use, you must contact us and
 *    obtain specific terms and conditions for the use of the software.
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

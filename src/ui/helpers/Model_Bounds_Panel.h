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
		std::vector<QLineEdit*> mLowerBoundEdits, mDefaultsEdits, mUpperBoundEdits;

		// refreshes contents (may use given parameter set)
		void Refresh_Contents(glucose::IModel_Parameter_Vector* inputs = nullptr);
		// resets user interface using given values
		void Reset_UI(const size_t parameter_count, const wchar_t** parameter_names, const double* lower_bounds, const double* defaults, const double* upper_bounds);
		// retrieves currently selected model; returns true on success
		bool Get_Current_Selected_Model(glucose::TModel_Descriptor& model);

	protected slots:
		void On_Reset_Lower();
		void On_Reset_Defaults();
		void On_Reset_Upper();

	public:
		CModel_Bounds_Panel(QComboBox* modelSelector, QWidget *parent);

		virtual glucose::TFilter_Parameter get_parameter();
		virtual void set_parameter(const glucose::TFilter_Parameter &param);
		virtual void apply();
};
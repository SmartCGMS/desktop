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

#include "descriptor.h"
#include "gui_subchain.h"

#include "../../../common/lang/dstrings.h"
#include "../../../common/rtl/manufactory.h"
#include "../../../common/rtl/FilterLib.h"
#include "../../../common/rtl/SolverLib.h"
#include "../../../common/rtl/UILib.h"

#include <vector>

namespace gui
{
	glucose::TFilter_Descriptor GUI_Descriptor = {
		gui_filter_guid,
		glucose::NFilter_Flags::None,
		dsGUI_Filter,

		// parameters of GUI filter will be lazy-loaded from encapsulated filters
		0,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	};

	const std::array<GUID, 3> gui_filters = { {
		glucose::Drawing_Filter,
		glucose::Error_Filter,
		glucose::Log_Filter
	} };

	// lazy-loaded config param types
	std::vector<glucose::NParameter_Type> config_param_types;
	// lazy-loaded config param names
	std::vector<const wchar_t*> config_names;
	// lazy-loaded config UI names
	std::vector<const wchar_t*> ui_names;
	// was GUI filter loaded? (lazyload)
	bool gui_filter_descriptor_initialized = false;

	// this will be filled after initialization of parameter arrays
	static std::vector<glucose::TFilter_Descriptor> filter_descriptions  { };

	// initializes filter descriptor and its parameters
	bool init_filter_descriptor()
	{
		// load filters from gui_filters array
		glucose::TFilter_Descriptor desc = glucose::Null_Filter_Descriptor;
		
		for (const auto &gui_filter : gui_filters) 	{
			// if the filter fails to load, fail whole GUI filter loading routine
			if (!glucose::get_filter_descriptor_by_id(gui_filter, desc))
				return false;

			// append filter name as null parameter if there are some parameters - it will split configuration options in config window
			if (desc.parameters_count > 0)
			{
				config_param_types.push_back(glucose::NParameter_Type::ptNull);
				ui_names.push_back(desc.description);
				config_names.push_back(L"");
			}

			// copy parameters
			for (size_t paramIdx = 0; paramIdx < desc.parameters_count; paramIdx++)
			{
				config_param_types.push_back(desc.parameter_type[paramIdx]);
				ui_names.push_back(desc.ui_parameter_name[paramIdx]);
				config_names.push_back(desc.config_parameter_name[paramIdx]);
			}
		}

		GUI_Descriptor.parameter_type = config_param_types.data();
		GUI_Descriptor.config_parameter_name = config_names.data();
		GUI_Descriptor.ui_parameter_name = ui_names.data();

		*const_cast<size_t*>(&GUI_Descriptor.parameters_count) = config_names.size();

		filter_descriptions.push_back(gui::GUI_Descriptor);
		gui_filter_descriptor_initialized = true;

		return true;
	}

		const std::vector<glucose::TFilter_Descriptor> &get_gui_filter_descriptors() {

		if (!gui::gui_filter_descriptor_initialized)
		{
			if (!gui::init_filter_descriptor()) filter_descriptions.clear();
		}

		return filter_descriptions;
	}

	HRESULT IfaceCalling create_gui_filter(const GUID *id, glucose::IFilter_Pipe_Reader *input, glucose::IFilter_Pipe_Writer *output, glucose::IFilter **filter) {
		if (*id == gui::GUI_Descriptor.id)
			return Manufacture_Object<CGUI_Filter_Subchain>(filter, refcnt::make_shared_reference_ext<glucose::SFilter_Pipe_Reader, glucose::IFilter_Pipe_Reader>( input, true ), refcnt::make_shared_reference_ext<glucose::SFilter_Pipe_Writer, glucose::IFilter_Pipe_Writer>(output, true));
	
		return ENOENT;
	}
}

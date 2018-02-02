#pragma once

#include "../../../../common/iface/FilterIface.h"


namespace filter_config_window {
	class CContainer_Edit {
	public:
		virtual glucose::TFilter_Parameter get_parameter() = 0;
		virtual void set_parameter(const glucose::TFilter_Parameter &param) = 0;
		virtual void apply() {};	//e.g., on click the Apply button - non-mandatory function
	};
}

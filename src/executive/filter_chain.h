#pragma once

#include "../../../common/iface/UIIface.h"

#include <vector>

struct TFilter_Chain_Link {
	glucose::TFilter_Descriptor descriptor;
	std::vector<glucose::TFilter_Parameter> configuration;
};

class CFilter_Chain : public std::vector<TFilter_Chain_Link> {

};
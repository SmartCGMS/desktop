#pragma once

#include "../../../common/iface/UIIface.h"

#include <vector>
#include <functional>

class CFilter_Configuration : public std::vector<glucose::TFilter_Parameter> {
protected:
	void Traverse_Configuration(std::function<void(refcnt::IReferenced *obj)> func);
public:
	CFilter_Configuration(const CFilter_Configuration& other) {};
	~CFilter_Configuration();

	CFilter_Configuration& operator=(const CFilter_Configuration& other);
};

struct TFilter_Chain_Link {
	glucose::TFilter_Descriptor descriptor;
	CFilter_Configuration configuration;
};

class CFilter_Chain : public std::vector<TFilter_Chain_Link> {
};
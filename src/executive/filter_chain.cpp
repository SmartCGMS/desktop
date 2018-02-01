#include "filter_chain.h"


CFilter_Configuration::CFilter_Configuration() : std::vector<glucose::TFilter_Parameter>() {
}

CFilter_Configuration::CFilter_Configuration(const CFilter_Configuration &other) : std::vector<glucose::TFilter_Parameter>(other) {
	Traverse_Configuration([](refcnt::IReferenced *obj) { obj->AddRef(); });
}

CFilter_Configuration::~CFilter_Configuration() {
	Traverse_Configuration([](refcnt::IReferenced *obj) { obj->Release(); });	
}

void CFilter_Configuration::Traverse_Configuration(std::function<void(refcnt::IReferenced *obj)> func) {
	
	for (auto &param : *this) {			
		func(param.config_name);

		switch (param.type) {
			case glucose::NParameter_Type::ptWChar_Container: func(param.wstr);
				break;
			case glucose::NParameter_Type::ptInt64_Container: func(param.vec_int64);
				break;
		}
	}
	
}

CFilter_Configuration& CFilter_Configuration::operator=(const CFilter_Configuration& other) {	
	std::vector<glucose::TFilter_Parameter>::operator=(other);

	Traverse_Configuration([](refcnt::IReferenced *obj) { obj->AddRef(); });

	return *this;
}
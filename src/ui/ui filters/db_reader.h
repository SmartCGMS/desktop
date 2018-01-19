#pragma once

#include "../../../../common/iface/FilterIface.h"
#include "../../../../common/iface/UIIface.h"

extern const glucose::TFilter_Descriptor TDb_Reader_Descriptor;

class CDb_Reader : public glucose::IFilter {
	//class that reads selected segments from the db produces the events
	//i.e., it mimicks CGMS
protected:
public:
	virtual HRESULT configure(const std::string &configuration) final;
};
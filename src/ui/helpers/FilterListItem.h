#pragma once


#include <QtWidgets/QListWidgetItem >

#include "../../../../common/rtl/FilterLib.h"

class CFilter_List_Item : public QListWidgetItem {	
protected:
	const glucose::TFilter_Descriptor mDescriptor;
	std::vector<glucose::TFilter_Parameter> mConfiguration;
public:
	explicit CFilter_List_Item(const glucose::TFilter_Descriptor &descriptor);	
	std::vector<glucose::TFilter_Parameter>& configuration();
	const glucose::TFilter_Descriptor& description() const;

};
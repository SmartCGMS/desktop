#pragma once


#include <QtWidgets/QListWidgetItem >

#include "../../../../common/rtl/FilterLib.h"

class CFilter_List_Item : public QListWidgetItem {	
protected:
	const glucose::TFilter_Descriptor &mDescriptor;
	std::string mConfiguration;
public:
	explicit CFilter_List_Item(const glucose::TFilter_Descriptor &descriptor);
	virtual ~CFilter_List_Item();

	bool Configure();
	std::string Configuration();

};
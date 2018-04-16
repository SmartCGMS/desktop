#pragma once


#include <QtWidgets/QListWidgetItem >

#include "../../../../common/desktop-console/filter_chain.h"

#include "../../../../common/rtl/FilterLib.h"

class CFilter_List_Item : public QListWidgetItem {	
protected:
	const glucose::TFilter_Descriptor mDescriptor;
	CFilter_Configuration mConfiguration;
public:
	explicit CFilter_List_Item(const glucose::TFilter_Descriptor &descriptor);	
	CFilter_Configuration& configuration();
	const glucose::TFilter_Descriptor& description() const;

	void Refresh();
};
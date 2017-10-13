#pragma once


#include <QtWidgets/QListWidgetItem >

#include "../../../../common/rtl/FilterLib.h"

class CFilter_List_Item : public QListWidgetItem {	
protected:
	const glucose::SFilter_Factory &mFactory;
	
public:
	explicit CFilter_List_Item(const glucose::SFilter_Factory &factory);
	virtual ~CFilter_List_Item();
};
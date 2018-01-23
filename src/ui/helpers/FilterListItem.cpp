#include "FilterListItem.h"

#include <QtCore/QObject>

CFilter_List_Item::CFilter_List_Item(const glucose::TFilter_Descriptor &descriptor) :
	mDescriptor(descriptor), QListWidgetItem() {

	setText(QString::fromWCharArray(mDescriptor.description));
}

CFilter_List_Item::~CFilter_List_Item() {


}


bool CFilter_List_Item::Configure() {
	return false;
//	return SuCCEEDED(mDescriptor.configurator(mConfiguration));
}

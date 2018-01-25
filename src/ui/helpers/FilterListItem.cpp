#include "FilterListItem.h"

#include <QtCore/QObject>

CFilter_List_Item::CFilter_List_Item(const glucose::TFilter_Descriptor &descriptor) :
	mDescriptor(descriptor), QListWidgetItem() {

	setText(QString::fromWCharArray(mDescriptor.description));
}

std::vector<glucose::TFilter_Parameter>& CFilter_List_Item::configuration() {
	return mConfiguration;
}

const glucose::TFilter_Descriptor& CFilter_List_Item::description() const {
	return mDescriptor;
}
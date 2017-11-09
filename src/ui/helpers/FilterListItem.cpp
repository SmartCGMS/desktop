#include "FilterListItem.h"

#include <QtCore/QObject>

CFilter_List_Item::CFilter_List_Item(const glucose::SFilter_Factory &factory) :
	mFactory(factory), QListWidgetItem() {

	setText(QString::fromWCharArray(mFactory.description()));
}

CFilter_List_Item::~CFilter_List_Item() {


}

#include "FilterListItem.h"
#include "../../../../common/rtl/UILib.h"

#include <QtCore/QObject>

CFilter_List_Item::CFilter_List_Item(const glucose::TFilter_Descriptor &descriptor) :
	mDescriptor(descriptor), QListWidgetItem() {

	Refresh();
}

CFilter_Configuration& CFilter_List_Item::configuration() {
	return mConfiguration;
}

const glucose::TFilter_Descriptor& CFilter_List_Item::description() const {
	return mDescriptor;
}

void CFilter_List_Item::Refresh()
{
	QString text = QString::fromWCharArray(mDescriptor.description);

	auto models = glucose::get_model_descriptors();

	// splitter appending logic - at first, apply " - " to split name from description, then apply ", " to split description items
	bool splitterAppended = false;
	auto appendSplitter = [&]() {
		if (splitterAppended)
		{
			text += QString(", ");
			return;
		}

		text += QString(" - ");
		splitterAppended = true;
	};

	// traverse configuration and find items to put into description
	for (auto& cfg : mConfiguration)
	{
		// model signal - append signal name
		if (cfg.type == glucose::NParameter_Type::ptModel_Signal_Id)
		{
			bool found = false;
			for (auto& model : models)
			{
				for (size_t i = 0; i < model.number_of_calculated_signals; i++)
				{
					if (model.calculated_signal_ids[i] == cfg.guid)
					{
						appendSplitter();
						text += QString::fromWCharArray(model.calculated_signal_names[i]);
						found = true;
						break;
					}
				}

				if (found)
					break;
			}
		}
		// model - append model description
		else if (cfg.type == glucose::NParameter_Type::ptModel_Id)
		{
			for (auto& model : models)
			{
				if (model.id == cfg.guid)
				{
					appendSplitter();
					text += QString::fromWCharArray(model.description);
					break;
				}
			}
		}
	}

	setText(text);
}

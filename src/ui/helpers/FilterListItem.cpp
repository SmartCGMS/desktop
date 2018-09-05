/**
 * SmartCGMS - continuous glucose monitoring and controlling framework
 * https://diabetes.zcu.cz/
 *
 * Contact:
 * diabetes@mail.kiv.zcu.cz
 * Medical Informatics, Department of Computer Science and Engineering
 * Faculty of Applied Sciences, University of West Bohemia
 * Technicka 8
 * 314 06, Pilsen
 *
 * Licensing terms:
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * a) For non-profit, academic research, this software is available under the
 *    GPLv3 license. When publishing any related work, user of this software
 *    must:
 *    1) let us know about the publication,
 *    2) acknowledge this software and respective literature - see the
 *       https://diabetes.zcu.cz/about#publications,
 *    3) At least, the user of this software must cite the following paper:
 *       Parallel software architecture for the next generation of glucose
 *       monitoring, Proceedings of the 8th International Conference on Current
 *       and Future Trends of Information and Communication Technologies
 *       in Healthcare (ICTH 2018) November 5-8, 2018, Leuven, Belgium
 * b) For any other use, especially commercial use, you must contact us and
 *    obtain specific terms and conditions for the use of the software.
 */

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

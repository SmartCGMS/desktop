#pragma once

#include "../../../common/iface/UIIface.h"
#include "../../../common/rtl/hresult.h"

namespace gui
{
	constexpr GUID gui_filter_guid = { 0x501501bc, 0x4a21, 0x12c9,{ 0xb7, 0x9f, 0xe1, 0x29, 0x44, 0xf8, 0x04, 0xa1 } }; //// {501501BC-4A21-12C9-B79F-E12944F804A1}

	const std::vector<glucose::TFilter_Descriptor> &get_gui_filter_descriptors();
	HRESULT IfaceCalling create_gui_filter(const GUID *id, glucose::IFilter_Pipe *input, glucose::IFilter_Pipe *output, glucose::IFilter **filter);

	extern const std::array<GUID, 3> gui_filters;
}

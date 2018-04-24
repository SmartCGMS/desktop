#pragma once

#include "../../../common/iface/UIIface.h"
#include "../../../common/rtl/hresult.h"

namespace gui
{
	constexpr GUID gui_filter_guid = { 0x501501bc, 0x4a21, 0x12c9,{ 0xb7, 0x9f, 0xe1, 0x29, 0x44, 0xf8, 0x04, 0xa1 } }; //// {501501BC-4A21-12C9-B79F-E12944F804A1}
	constexpr GUID input_filter_guid = { 0xa195121b, 0x13a1, 0x9817,{ 0xac, 0x12, 0x01, 0x44, 0x3f, 0x11, 0xa0, 0xca } }; //// {A195121B-13A1-9817-AC12-01443F11A0CA}

	extern "C" HRESULT IfaceCalling do_get_filter_descriptors(glucose::TFilter_Descriptor **begin, glucose::TFilter_Descriptor **end);
	extern "C" HRESULT IfaceCalling do_create_filter(const GUID *id, glucose::IFilter_Pipe *input, glucose::IFilter_Pipe *output, glucose::IFilter **filter);

	extern const std::array<GUID, 3> gui_filters;
}

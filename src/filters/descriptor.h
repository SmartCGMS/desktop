#pragma once

#include "../../../common/iface/UIIface.h"
#include "../../../common/rtl/hresult.h"

namespace gui
{
	constexpr GUID gui_filter_guid = { 0x501501bc, 0x4a21, 0x12c9,{ 0xb7, 0x9f, 0xe1, 0x29, 0x44, 0xf8, 0x04, 0xa1 } }; //// {501501BC-4A21-12C9-B79F-E12944F804A1}
	constexpr GUID input_filter_guid = { 0xa195121b, 0x13a1, 0x9817,{ 0xac, 0x12, 0x01, 0x44, 0x3f, 0x11, 0xa0, 0xca } }; //// {A195121B-13A1-9817-AC12-01443F11A0CA}

	extern "C" HRESULT IfaceCalling do_get_filter_descriptors(glucose::TFilter_Descriptor **begin, glucose::TFilter_Descriptor **end);
	extern "C" HRESULT IfaceCalling do_create_filter(const GUID *id, glucose::IFilter_Pipe *input, glucose::IFilter_Pipe *output, glucose::IFilter **filter);

	/* GUI filter has full knowledge of what filters we may want to use; if some of filters is not available, then it's simply skipped and not used.
	 * To add a new filter, simply copy its GUID here, increase gui_filters_count constant and add it to gui_filters array */	

	constexpr GUID drawing_filter_guid = { 0x850a122c, 0x8943, 0xa211,{ 0xc5, 0x14, 0x25, 0xba, 0xa9, 0x14, 0x35, 0x74 } };
	constexpr GUID errors_filter_guid = { 0x4a125499, 0x5dc8, 0x128e,{ 0xa5, 0x5c, 0x14, 0x22, 0xbc, 0xac, 0x10, 0x74 } };
	constexpr GUID log_filter_guid = { 0xc0e942b9, 0x3928, 0x4b81,{ 0x9b, 0x43, 0xa3, 0x47, 0x66, 0x82, 0x0, 0xBA } };	

	extern const std::array<GUID, 3> gui_filters;
}

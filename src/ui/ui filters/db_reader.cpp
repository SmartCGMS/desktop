#include "db_reader.h"

#include "../../../../common/rtl/FilterLib.h"

#include "../../../../common/lang/dstrings.h"

namespace db_reader {

	const glucose::TFilter_Descriptor TDb_Reader_Descriptor = {
		{ 0xc0e942b9, 0x3928, 0x4b81,{ 0x9b, 0x43, 0xa3, 0x47, 0x66, 0x82, 0x0, 0x42 } },	//// {C0E942B9-3928-4B81-9B43-A34766820042}	
		dsDb_Reader,
		nullptr
	};

	class CAdvertise_Db_Reader {
	public:
		CAdvertise_Db_Reader() { advertise_filter_descriptors(&TDb_Reader_Descriptor, &TDb_Reader_Descriptor + 1); };
	};

	static CAdvertise_Db_Reader Advertise_Db_Reader{};
}




HRESULT CDb_Reader::configure(const std::string &configuration) {
	return E_NOTIMPL;
};
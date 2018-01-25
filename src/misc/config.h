#pragma once

#include "../../../../common/SimpleIni.h"

#include "../executive/filter_chain.h"

class CConfig {
protected:
	std::wstring mFile_Path;
	CSimpleIniW mIni;
public:
	CConfig();

	void Load(CFilter_Chain &filter_chain);
	void Save(const CFilter_Chain &filter_chain);
};


extern CConfig Configuration;
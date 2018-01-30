#include "config.h"

#include "../../../common/rtl/UILib.h"
#include "../../../common/rtl/FilterLib.h"
#include "../../../common/lang/dstrings.h"
#include "../../../../common/QtUtils.h"

#include <QtWidgets/QApplication>
#include <QtCore/QDir>

#include <vector>
#include <list>
#include <fstream>

CConfig Configuration;


void CConfig::Resolve_And_Load_Config_File() {
	mFile_Path = QStringToStdWString(QApplication::instance()->applicationDirPath() + QDir::separator()) + rsConfig_File_Name;

	std::vector<char> buf;
	std::ifstream configfile;

	try {
		configfile.open(mFile_Path);

		if (configfile.is_open()) {
			buf.assign(std::istreambuf_iterator<char>(configfile), std::istreambuf_iterator<char>());
			mIni.LoadData(buf.data(), buf.size());
		}

		configfile.close();
	}
	catch (...) {
	}
}

void CConfig::Load(CFilter_Chain &filter_chain) {
	std::list<CSimpleIniW::Entry> section_names;
	mIni.GetAllSections(section_names);

	CFilter_Chain new_chain;
	for (auto &section_name : section_names) {
		std::wstring name_str{ section_name.pItem };
		const std::wstring prefix{ rs_Filter_Section_Prefix };
		auto res = std::mismatch(prefix.begin(), prefix.end(), name_str.begin());
		if (res.first == prefix.end()) {
			//OK, this is filter section - extract the guid			
			const GUID id = WString_To_GUID(std::wstring{ name_str.begin() + prefix.size(), name_str.end() });
			//and get the filter descriptor to load the parameters
			
			glucose::TFilter_Descriptor desc;
			CFilter_Configuration filter_config;

			if (glucose::get_filter_descriptors_by_id(id, desc)) {
				//so.. now, try to load the filter parameters - aka filter_config
				
				

				for (size_t i = 0; i < desc.parameters_count; i++) {
					glucose::TFilter_Parameter filter_parameter;
					filter_parameter.type = desc.parameter_type[i];
					filter_parameter.config_name = WString_To_WChar_Container(desc.config_parameter_name[i]);

					//does the value exists?
					const wchar_t* str_value = mIni.GetValue(section_name.pItem, desc.config_parameter_name[i]);
					if (str_value) {
						//yes, there is somethign stored under this key
						switch (filter_parameter.type) {
						case glucose::NParameter_Type::ptWChar_Container:
							filter_parameter.wstr = WString_To_WChar_Container(str_value);
							filter_config.push_back(filter_parameter);
							break;

						case glucose::NParameter_Type::ptDouble:
							filter_parameter.dbl = mIni.GetDoubleValue(section_name.pItem, desc.config_parameter_name[i]);
							filter_config.push_back(filter_parameter);
							break;
						}
					}
				}
			}
			

			//and finally, add the new link into the filter chain
			new_chain.push_back({desc, filter_config});
		}
	}

	//finally, replace the chain with new one
	filter_chain = std::move(new_chain);
}

void CConfig::Save(const CFilter_Chain &filter_chain) {
	for (auto &link : filter_chain) {
		const std::wstring id_str = rs_Filter_Section_Prefix +GUID_To_WString(link.descriptor.id);
		auto section = mIni.GetSection(id_str.c_str());
		if (!section) 
			//if the section does not exist yet, create it by writing a comment there - the filter description
			mIni.SetValue(id_str.c_str(), nullptr, nullptr, std::wstring{ rsIni_Comment_Prefix}.append(link.descriptor.description).c_str());				

		//and store the parameters
		for (const auto &param : link.configuration) {
			switch (param.type) {
				case glucose::NParameter_Type::ptWChar_Container: 
					mIni.SetValue(id_str.c_str(), WChar_Container_To_WString(param.config_name).c_str(), WChar_Container_To_WString(param.wstr).c_str());
					break;

				case glucose::NParameter_Type::ptDouble: 
					mIni.SetDoubleValue(id_str.c_str(), WChar_Container_To_WString(param.config_name).c_str(), param.dbl);
					break;
			}
		}
			
		
	}



	std::string content;
	mIni.Save(content);
	std::ofstream config_file(mFile_Path);
	if (config_file.is_open()) {
		config_file << content;
		config_file.close();
	}

}
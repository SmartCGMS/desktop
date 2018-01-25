#include "config.h"

#include "../../../common/rtl/UILib.h"
#include "../../../common/lang/dstrings.h"
#include "../../../../common/QtUtils.h"

#include <QtWidgets/QApplication>
#include <QtCore/QDir>

#include <vector>
#include <fstream>

CConfig Configuration;


CConfig::CConfig() {
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

}

void CConfig::Save(const CFilter_Chain &filter_chain) {
	for (const auto &link : filter_chain) {
		auto section = mIni.GetSection(GUID_To_WString(link.descriptor.id));
		if (!section) mIni.
				//if the section does not exist yet, create it by writing a comment there - the filter description
	}



	std::string content;
	mIni.Save(content);
	std::ofstream config_file(mFile_Path);
	if (config_file.is_open()) {
		config_file << content;
		config_file.close();
	}

}
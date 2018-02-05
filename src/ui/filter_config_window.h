#pragma once

#include "../../../common/rtl/FilterLib.h"

#include "helpers/general_container_edit.h"

#include <map>

#include <QtWidgets/QWidget>

class CFilter_Config_Window : public QWidget {
	Q_OBJECT
protected:
	std::vector<glucose::TFilter_Parameter> &mConfiguration;
	const glucose::TFilter_Descriptor &mDescription;
	std::map<std::wstring, filter_config_window::CContainer_Edit*> mContainer_Edits;

	void Setup_UI();
	void Commit_Parameters();	//from controls to configuration
protected slots:
	void On_OK();
	void On_Cancel();
	void On_Apply();
public:
	CFilter_Config_Window(const glucose::TFilter_Descriptor &description, std::vector<glucose::TFilter_Parameter> &configuration, QWidget *parent);
};
#pragma once

#include "../../../common/rtl/FilterLib.h"

#include "helpers/general_container_edit.h"

#include <vector>

#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>

class CFilter_Config_Window : public QWidget {
	Q_OBJECT
protected:
	std::vector<glucose::TFilter_Parameter> &mConfiguration;
	const glucose::TFilter_Descriptor &mDescription;

	using TName_Container_Pair = std::pair<std::wstring, filter_config_window::CContainer_Edit*>;

	std::vector<TName_Container_Pair> mContainer_Edits;

	void Setup_UI();
	void Commit_Parameters();	//from controls to configuration
protected slots:
	void On_OK();
	void On_Cancel();
	void On_Apply();
public:
	CFilter_Config_Window(const glucose::TFilter_Descriptor &description, std::vector<glucose::TFilter_Parameter> &configuration, QWidget *parent);
};


class CInteger_Container_Edit : public QLineEdit, public virtual filter_config_window::CContainer_Edit {
	Q_OBJECT
public:
	CInteger_Container_Edit(QWidget *parent);
	glucose::TFilter_Parameter get_parameter();
	void set_parameter(const glucose::TFilter_Parameter &param);
};
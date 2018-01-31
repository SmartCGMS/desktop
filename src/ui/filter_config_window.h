#pragma once

#include "../../../common/rtl/FilterLib.h"

#include <map>

#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>

namespace filter_config_window {
	class CContainer_Edit {
	public:
		virtual glucose::TFilter_Parameter get_parameter() = 0;
		virtual void set_parameter(const glucose::TFilter_Parameter &param) = 0;
	};
}

class CFilter_Config_Window : public QWidget{
	Q_OBJECT
protected:
	std::vector<glucose::TFilter_Parameter> &mConfiguration;
	const glucose::TFilter_Descriptor &mDescription;
	std::map<std::wstring, filter_config_window::CContainer_Edit*> mContainer_Edits;

	void Setup_UI();
	void Apply_Parameters();
protected slots:
	void On_OK();
	void On_Cancel();
	void On_Apply();
public:
	CFilter_Config_Window(const glucose::TFilter_Descriptor &description, std::vector<glucose::TFilter_Parameter> &configuration, QWidget *parent);
};
#include "filter_config_window.h"

#include "../../../common/lang/dstrings.h"
#include "../../../common/rtl/manufactory.h"

#include "helpers/Select_Time_Segment_Id_Panel.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGridLayout>

#include "moc_filter_config_window.cpp"


	
class CWChar_Container_Edit : public QLineEdit, public virtual filter_config_window::CContainer_Edit {
	glucose::TFilter_Parameter CWChar_Container_Edit::get_parameter() {
		glucose::TFilter_Parameter result;
		result.type = glucose::NParameter_Type::ptWChar_Container;
		const std::wstring str = text().toStdWString();
		auto container = glucose::Create_Parameter_Container<wchar_t>(str.data(), str.data() + str.size());
		result.wstr = container.get();
		result.wstr->AddRef();
		return result;
	}

	void set_parameter(const glucose::TFilter_Parameter &param) {
		setText(QString::fromStdWString(WChar_Container_To_WString(param.wstr)));
	}
};





CFilter_Config_Window::CFilter_Config_Window(const glucose::TFilter_Descriptor &description, std::vector<glucose::TFilter_Parameter> &configuration, QWidget *parent) :
	mDescription(description), mConfiguration(configuration), QWidget(parent) {


	Setup_UI();

	//Load configuration, i.e., parameters
	for (auto &parameter : configuration) {
		auto edit = mContainer_Edits.find(WChar_Container_To_WString(parameter.config_name));
		if (edit != mContainer_Edits.end()) edit->second->set_parameter(parameter);
	}
}

void CFilter_Config_Window::Setup_UI() {

	setWindowTitle(QString::fromWCharArray(mDescription.description) + QString(" ") + tr(dsConfiguration));

	QTabWidget *tabs = new QTabWidget{};


	
	QWidget *main_tab = new QWidget{};
	{
		const int idxName_col = 0;
		const int idxEdit_col = 1;

		QGridLayout *main_layout = new QGridLayout();
		int ui_row = 0;
		for (int i = 0; i < static_cast<int>(mDescription.parameters_count); i++) {
			QLabel *label = new QLabel{ QString::fromWCharArray(mDescription.ui_parameter_name[i]) };
			main_layout->addWidget(label, i, idxName_col);

			auto add_edit_control = [&](const size_t parameter_idx) {
				filter_config_window::CContainer_Edit *container = nullptr;

				switch (mDescription.parameter_type[i]) {
					case glucose::NParameter_Type::ptWChar_Container: container = new CWChar_Container_Edit{};
																		break;
					
					case glucose::NParameter_Type::ptSelect_Time_Segment_ID: container = new CSelect_Time_Segment_Id_Panel{};
																			 break;
				}

				if (container != nullptr) {
					mContainer_Edits[mDescription.ui_parameter_name[i]] = container;
					switch (mDescription.parameter_type[i]) {
						case glucose::NParameter_Type::ptSelect_Time_Segment_ID:
								//special widget, let's add it as a standalone tab
							main_tab->addAction widget container via layout
							break;

						default: main_layout->addWidget(dynamic_cast<QWidget*>(container), ui_row, idxEdit_col);
							ui_row++;
							break;
					}
					
				}
			};
			
			add_edit_control(i);
		}

		main_tab->setLayout(main_layout);
		tabs->addTab(main_tab, tr(dsMain_Parameters));
	}

	QHBoxLayout *button_layout = new QHBoxLayout{};
	{

		button_layout->setAlignment(Qt::AlignRight);

		QPushButton *btnOK = new QPushButton{ tr(dsOK) };
		QPushButton *btnCancel = new QPushButton{ tr(dsCancel) };
		QPushButton *btnApply = new QPushButton{ tr(dsApply) };

		button_layout->addWidget(btnOK);
		button_layout->addWidget(btnCancel);
		button_layout->addWidget(btnApply);

		connect(btnOK, SIGNAL(clicked()), this, SLOT(On_OK()));
		connect(btnCancel, SIGNAL(clicked()), this, SLOT(On_Cancel()));
		connect(btnApply, SIGNAL(clicked()), this, SLOT(On_Apply()));
	}

	QVBoxLayout *final_layout = new QVBoxLayout{};
	final_layout->addWidget(tabs);
	final_layout->addLayout(button_layout);
	
	setLayout(final_layout);


	setWindowModality(Qt::ApplicationModal);
	//set the window to be freed upon closing
	setAttribute(Qt::WA_DeleteOnClose, true);
}

void CFilter_Config_Window::Apply_Parameters() {
	std::vector<glucose::TFilter_Parameter> new_parameters;
	for (auto &edit : mContainer_Edits) {
		glucose::TFilter_Parameter param = edit.second->get_parameter();
		param.config_name = WString_To_WChar_Container(edit.first.c_str());

		new_parameters.push_back(param);
	}

	mConfiguration = std::move(new_parameters);
}

void CFilter_Config_Window::On_OK() {
	Apply_Parameters();
	close();
}

void CFilter_Config_Window::On_Cancel() {
	close();
}

void CFilter_Config_Window::On_Apply() {
	Apply_Parameters();
}
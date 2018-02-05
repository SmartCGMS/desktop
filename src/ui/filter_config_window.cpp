#include "filter_config_window.h"

#include "../../../common/lang/dstrings.h"
#include "../../../common/rtl/manufactory.h"
#include "../../../common/rtl/referencedImpl.h"

#include "helpers/Select_Time_Segment_Id_Panel.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLineEdit>
#include <QtGui/QValidator>

#include "moc_filter_config_window.cpp"


	
class CWChar_Container_Edit : public QLineEdit, public virtual filter_config_window::CContainer_Edit {
	glucose::TFilter_Parameter get_parameter() {
		glucose::TFilter_Parameter result;
		result.type = glucose::NParameter_Type::ptWChar_Container;
		const std::wstring str = text().toStdWString();		
		result.wstr = refcnt::WString_To_WChar_Container(str.c_str());
		return result;
	}

	void set_parameter(const glucose::TFilter_Parameter &param) {
		setText(QString::fromStdWString(WChar_Container_To_WString(param.wstr)));
	}
};

class CDouble_Container_Edit : public QLineEdit, public virtual filter_config_window::CContainer_Edit {	
public:
	CDouble_Container_Edit(QWidget *parent) : QLineEdit(parent) {
		setValidator(new QDoubleValidator());
	}

	glucose::TFilter_Parameter get_parameter() {
		glucose::TFilter_Parameter result;
		result.type = glucose::NParameter_Type::ptDouble;
		bool ok;
		result.dbl = text().toDouble(&ok);
		if (!ok)
			result.dbl = 0;
		return result;
	}

	void set_parameter(const glucose::TFilter_Parameter &param) {
		setText(QString::number(param.dbl));
	}
};

class CInteger_Container_Edit : public QLineEdit, public virtual filter_config_window::CContainer_Edit {
public:
	CInteger_Container_Edit(QWidget *parent) : QLineEdit(parent) {
		setValidator(new QIntValidator());
	}

	glucose::TFilter_Parameter get_parameter() {
		glucose::TFilter_Parameter result;
		result.type = glucose::NParameter_Type::ptInt64;
		bool ok;
		result.int64 = text().toLongLong(&ok);
		if (!ok)
			result.int64 = 0;
		return result;
	}

	void set_parameter(const glucose::TFilter_Parameter &param) {
		setText(QString::number(param.int64));
	}
};

class CBoolean_Container_Edit : public QCheckBox, public virtual filter_config_window::CContainer_Edit {
	glucose::TFilter_Parameter get_parameter() {
		glucose::TFilter_Parameter result;
		result.type = glucose::NParameter_Type::ptBool;
		result.boolean = (checkState() == Qt::Checked);
		return result;
	}

	void set_parameter(const glucose::TFilter_Parameter &param) {
		setCheckState(param.boolean ? Qt::Checked : Qt::Unchecked);
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

	//and apply the loaded parameters
	for (auto &edit : mContainer_Edits)
		edit.second->apply();

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
			
			auto add_edit_control = [&]() {
				filter_config_window::CContainer_Edit *container = nullptr;

				switch (mDescription.parameter_type[i])
				{
					case glucose::NParameter_Type::ptWChar_Container:
						container = new CWChar_Container_Edit{};
						break;

					case glucose::NParameter_Type::ptDouble:
						container = new CDouble_Container_Edit{ nullptr};
						break;

					case glucose::NParameter_Type::ptInt64:
						container = new CInteger_Container_Edit{ nullptr};
						break;

					case glucose::NParameter_Type::ptBool:
						container = new CBoolean_Container_Edit{};
						break;
					
					case glucose::NParameter_Type::ptSelect_Time_Segment_ID:
						container = new CSelect_Time_Segment_Id_Panel{ mConfiguration, nullptr };
						break;
				}

				mContainer_Edits[mDescription.config_parameter_name[i]] = container;
				switch (mDescription.parameter_type[i]) {
					case glucose::NParameter_Type::ptSelect_Time_Segment_ID:
							//special widget, let's add it as a standalone tab
						tabs->addTab(dynamic_cast<QWidget*>(container), QString::fromWCharArray(mDescription.ui_parameter_name[i]));
						break;

					default: 
						QLabel * label = new QLabel{ QString::fromWCharArray(mDescription.ui_parameter_name[i]) };
						main_layout->addWidget(label, ui_row, idxName_col);
						main_layout->addWidget(dynamic_cast<QWidget*>(container), ui_row, idxEdit_col);
						ui_row++;
						break;
				}
					
			};			
			
			add_edit_control();
		}		
		main_tab->setLayout(main_layout);
		tabs->insertTab(0, main_tab, tr(dsMain_Parameters));	//insert makes the main edits to be first
		tabs->setCurrentIndex(0);
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

void CFilter_Config_Window::Commit_Parameters() {
	std::vector<glucose::TFilter_Parameter> new_parameters;
	for (auto &edit : mContainer_Edits) {
		glucose::TFilter_Parameter param = edit.second->get_parameter();
		param.config_name = refcnt::WString_To_WChar_Container(edit.first.c_str());

		new_parameters.push_back(param);
	}

	mConfiguration = std::move(new_parameters);
}

void CFilter_Config_Window::On_OK() {
	Commit_Parameters();
	close();
}

void CFilter_Config_Window::On_Cancel() {
	close();
}

void CFilter_Config_Window::On_Apply() {
	Commit_Parameters();
	//and apply the commited parameters
	for (auto &edit : mContainer_Edits)
		edit.second->apply();
}
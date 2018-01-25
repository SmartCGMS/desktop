#include "filter_config_window.h"

#include "../../../common/lang/dstrings.h"
#include "../../../common/rtl/manufactory.h"

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
		container->AddRef();
		return result;
	}
};





CFilter_Config_Window::CFilter_Config_Window(const glucose::TFilter_Descriptor &description, std::vector<glucose::TFilter_Parameter> &configuration, QWidget *parent) :
	mDescription(description), mConfiguration(configuration), QWidget(parent) {


	Setup_UI();
}

void CFilter_Config_Window::Setup_UI() {

	setWindowTitle(QString::fromWCharArray(mDescription.description) + QString(" ") + tr(dsConfiguration));

	QTabWidget *tabs = new QTabWidget{};


	
	QWidget *main_tab = new QWidget{};
	{
		const int idxName_col = 0;
		const int idxEdit_col = 1;

		QGridLayout *main_layout = new QGridLayout();
		for (int i = 0; i < static_cast<int>(mDescription.parameters_count); i++) {
			QLabel *label = new QLabel{ QString::fromWCharArray(mDescription.ui_parameter_name[i]) };
			main_layout->addWidget(label, i, idxName_col);

			auto add_edit_control = [&](const size_t parameter_idx) {
				filter_config_window::CContainer_Edit *container = nullptr;

				switch (mDescription.parameter_type[i]) {
					case glucose::NParameter_Type::ptWChar_Container: container = new CWChar_Container_Edit{};
				}

				if (container != nullptr) {
					mContainer_Edits[parameter_idx] = container;
					main_layout->addWidget(dynamic_cast<QWidget*>(container), i, idxEdit_col);
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

void CFilter_Config_Window::On_OK() {

}

void CFilter_Config_Window::On_Cancel() {
	close();
}

void CFilter_Config_Window::On_Apply() {

}
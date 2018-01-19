#include "filters_window.h"

#include "../../../common/lang/dstrings.h"
#include "../../../common/rtl/FilterLib.h"

#include "helpers/FilterListItem.h"

#include <QtWidgets/QSplitter>
#include <QtWidgets/QVBoxLayout> 
#include <QtWidgets/QHBoxLayout> 
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>

#ifndef MOC_DIR
	#include "moc_filters_window.cpp"
#endif

std::atomic<CFilters_Window*> CFilters_Window::mInstance = nullptr;

CFilters_Window* CFilters_Window::Show_Instance(QWidget *owner) {

	CFilters_Window* tmp = nullptr;
	bool created = mInstance.compare_exchange_strong(tmp, new CFilters_Window(owner));

	if (created) {		
		mInstance.load()->showMaximized();		
	}

	return mInstance;
}

CFilters_Window::CFilters_Window(QWidget *owner) : QMdiSubWindow{ owner } {
	Setup_UI();
}

CFilters_Window::~CFilters_Window() {
	mInstance = nullptr;
}

void CFilters_Window::Setup_UI() {
	setWindowTitle(tr(dsFilters_Window));

	QWidget* wgtApplied_Filters = new QWidget(this);
	QVBoxLayout *lotApplied_Filters = new QVBoxLayout{  };
	lotApplied_Filters->addWidget(new QLabel{ tr(dsApplied_Filters), this });

	lbxApplied_Filters = new QListWidget{ this };		
	lotApplied_Filters->addWidget(lbxApplied_Filters);

	QPushButton *btnUp_Filter = new QPushButton{tr(dsMove_Up)};
	QPushButton *btnDown_Filter = new QPushButton{tr(dsMove_Down)};
	QPushButton *btnRemove_Filter = new QPushButton{tr(dsRemove)};
	QPushButton *btnConfigure_Filter = new QPushButton{ tr(dsConfigure) };

	QHBoxLayout *lotApplied_Buttons = new QHBoxLayout{  };
	QWidget *wgtApplied_Buttons = new QWidget{this};
	lotApplied_Buttons->addWidget(btnUp_Filter);
	lotApplied_Buttons->addWidget(btnDown_Filter);
	lotApplied_Buttons->addWidget(btnRemove_Filter);
	lotApplied_Buttons->addWidget(btnConfigure_Filter);
	wgtApplied_Buttons->setLayout(lotApplied_Buttons);
	lotApplied_Filters->addWidget(wgtApplied_Buttons);
	
	wgtApplied_Filters->setLayout(lotApplied_Filters);

	QWidget* wgtAvailable_Filters = new QWidget(this);

	QVBoxLayout *lotAvailable_Filters = new QVBoxLayout{  };
	QListWidget *lbxAvailable_Filters = new QListWidget{  };	

	//add the widgets
	{
		const auto &filters = glucose::get_filter_descriptors();
		for (const auto &filter : filters) {
			CFilter_List_Item *tmp = new CFilter_List_Item(filter);
			lbxAvailable_Filters->addItem(tmp);
		}		
	}
	

	QPushButton *btnAdd_Filter = new QPushButton{tr(dsAdd)};

	lotAvailable_Filters->addWidget(new QLabel{ tr(dsAvailable_Filters), this });
	lotAvailable_Filters->addWidget(lbxAvailable_Filters);
	
	QHBoxLayout *lotAvailable_Buttons = new QHBoxLayout{  };
	QWidget *wgtAvailable_Buttons = new QWidget{ this };
	lotAvailable_Buttons->addWidget(btnAdd_Filter);
	wgtAvailable_Buttons->setLayout(lotAvailable_Buttons);

	lotAvailable_Filters->addWidget(wgtAvailable_Buttons);

	
	wgtAvailable_Filters->setLayout(lotAvailable_Filters);

	QWidget *content = new QWidget {this};
	QSplitter *splitter = new QSplitter{ this };

	splitter->addWidget(wgtApplied_Filters);
	splitter->addWidget(wgtAvailable_Filters);
	
	setWidget(splitter);


	connect(btnConfigure_Filter, SIGNAL(clicked()), this, SLOT(On_Configure_Filter()));
}

void CFilters_Window::On_Configure_Filter() {
	const auto selection = lbxApplied_Filters->selectedItems();
	bool success = selection.size() == 1;
	if (success) {
		auto *filter = static_cast<CFilter_List_Item*>(selection[0]);
		
		success

	}
}
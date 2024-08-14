/**
 * SmartCGMS - continuous glucose monitoring and controlling framework
 * https://diabetes.zcu.cz/
 *
 * Copyright (c) since 2018 University of West Bohemia.
 *
 * Contact:
 * diabetes@mail.kiv.zcu.cz
 * Medical Informatics, Department of Computer Science and Engineering
 * Faculty of Applied Sciences, University of West Bohemia
 * Univerzitni 8, 301 00 Pilsen
 * Czech Republic
 * 
 * 
 * Purpose of this software:
 * This software is intended to demonstrate work of the diabetes.zcu.cz research
 * group to other scientists, to complement our published papers. It is strictly
 * prohibited to use this software for diagnosis or treatment of any medical condition,
 * without obtaining all required approvals from respective regulatory bodies.
 *
 * Especially, a diabetic patient is warned that unauthorized use of this software
 * may result into severe injure, including death.
 *
 *
 * Licensing terms:
 * Unless required by applicable law or agreed to in writing, software
 * distributed under these license terms is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * a) This file is available under the Apache License, Version 2.0.
 * b) When publishing any derivative work or results obtained using this software, you agree to cite the following paper:
 *    Tomas Koutny and Martin Ubl, "SmartCGMS as a Testbed for a Blood-Glucose Level Prediction and/or 
 *    Control Challenge with (an FDA-Accepted) Diabetic Patient Simulation", Procedia Computer Science,  
 *    Volume 177, pp. 354-362, 2020
 */

#include "filters_window.h"

#include <scgms/lang/dstrings.h>
#include <scgms/rtl/FilterLib.h>

#include "filter_config_window.h"
#include "helpers/FilterListItem.h"
#include "simulation_window.h"

#include <QtWidgets/QSplitter>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QVBoxLayout> 
#include <QtWidgets/QHBoxLayout> 
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>

#include <QtCore/QEventLoop>
#include <QtWidgets/QApplication>
#include <QtGui/QKeyEvent>

#ifndef MOC_DIR
	#include "moc_filters_window.cpp"
#endif

std::atomic<CFilters_Window*> CFilters_Window::mInstance = nullptr;

CFilters_Window* CFilters_Window::Show_Instance(scgms::SPersistent_Filter_Chain_Configuration &filter_chain_configuration, QWidget *owner) {

	if (mInstance) {
		mInstance.load()->showMaximized();
		return mInstance;
	}

	CFilters_Window* tmp = nullptr;
	bool created = mInstance.compare_exchange_strong(tmp, new CFilters_Window(filter_chain_configuration, owner));

	if (created) {
		mInstance.load()->showMaximized();
	}

	return mInstance;
}

CFilters_Window::CFilters_Window(scgms::SPersistent_Filter_Chain_Configuration &filter_chain_configuration, QWidget *owner) : QMdiSubWindow(owner), mFilter_Chain_Configuration(filter_chain_configuration) {
	Setup_UI();
}

CFilters_Window::~CFilters_Window() {
	mInstance = nullptr;
}

void CFilters_Window::Setup_UI() {
	setWindowTitle(tr(dsFilters_Window));
	setWindowIcon(QIcon(":/app/appicon.png"));

	QWidget* wgtApplied_Filters = new QWidget(this);
	QVBoxLayout *lotApplied_Filters = new QVBoxLayout{  };
	lotApplied_Filters->addWidget(new QLabel{ tr(dsApplied_Filters), this });

	lbxApplied_Filters = new QListWidget{ this };
	lbxApplied_Filters->installEventFilter(this);
	lbxApplied_Filters->setAcceptDrops(true);
	lbxApplied_Filters->setDragEnabled(true);
	lbxApplied_Filters->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	lbxApplied_Filters->setDragDropMode(QAbstractItemView::InternalMove);
	lotApplied_Filters->addWidget(lbxApplied_Filters);

	connect(lbxApplied_Filters->model(), SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)), this, SLOT(On_Filter_Drag_Drop(QModelIndex, int, int, QModelIndex, int)));

	QPushButton *btnUp_Filter = new QPushButton{tr(dsMove_Up)};
	QPushButton *btnDown_Filter = new QPushButton{tr(dsMove_Down)};
	QPushButton *btnRemove_Filter = new QPushButton{tr(dsRemove)};
	QPushButton *btnConfigure_Filter = new QPushButton{ tr(dsConfigure) };
	//QPushButton *btnCommit_Filters = new QPushButton{ tr(dsCommit) };

	QHBoxLayout *lotApplied_Buttons = new QHBoxLayout{  };
	QWidget *wgtApplied_Buttons = new QWidget{this};
	lotApplied_Buttons->addWidget(btnUp_Filter);
	lotApplied_Buttons->addWidget(btnDown_Filter);
	lotApplied_Buttons->addWidget(btnRemove_Filter);
	lotApplied_Buttons->addWidget(btnConfigure_Filter);
	//lotApplied_Buttons->addWidget(btnCommit_Filters);
	wgtApplied_Buttons->setLayout(lotApplied_Buttons);
	lotApplied_Filters->addWidget(wgtApplied_Buttons);
	
	wgtApplied_Filters->setLayout(lotApplied_Filters);

	QWidget* wgtAvailable_Filters = new QWidget(this);

	QVBoxLayout *lotAvailable_Filters = new QVBoxLayout{  };
	lbxAvailable_Filters = new QListWidget{ this };

	//add the available filters
	{
		const auto &filters = scgms::get_filter_descriptor_list();
		for (const auto &filter : filters) {
			CFilter_List_Item *tmp = new CFilter_List_Item(filter);
			lbxAvailable_Filters->addItem(tmp);
		}
	}

	lbxAvailable_Filters->sortItems();

	//add the  applied filters
	mFilter_Chain_Configuration.for_each([this](scgms::SFilter_Configuration_Link link) {
		CFilter_List_Item *tmp = new CFilter_List_Item(link);
		tmp->Refresh();
		lbxApplied_Filters->addItem(tmp);
	});

	QPushButton *btnAdd_Filter = new QPushButton{tr(dsAdd)};

	lotAvailable_Filters->addWidget(new QLabel{ tr(dsAvailable_Filters), this });
	lotAvailable_Filters->addWidget(lbxAvailable_Filters);
	
	QHBoxLayout *lotAvailable_Buttons = new QHBoxLayout{  };
	QWidget *wgtAvailable_Buttons = new QWidget{ this };
	lotAvailable_Buttons->addWidget(btnAdd_Filter);
	wgtAvailable_Buttons->setLayout(lotAvailable_Buttons);

	lotAvailable_Filters->addWidget(wgtAvailable_Buttons);

	wgtAvailable_Filters->setLayout(lotAvailable_Filters);

	QSplitter *splitter = new QSplitter{ this };

	splitter->addWidget(wgtApplied_Filters);
	splitter->addWidget(wgtAvailable_Filters);
	
	setWidget(splitter);

	// set the window to be freed upon closing
	setAttribute(Qt::WA_DeleteOnClose, true);

	connect(btnAdd_Filter, SIGNAL(clicked()), this, SLOT(On_Add_Filter()));
	connect(btnUp_Filter, SIGNAL(clicked()), this, SLOT(On_Move_Filter_Up()));
	connect(btnDown_Filter, SIGNAL(clicked()), this, SLOT(On_Move_Filter_Down()));
	connect(btnRemove_Filter, SIGNAL(clicked()), this, SLOT(On_Remove_Filter()));
	connect(btnConfigure_Filter, SIGNAL(clicked()), this, SLOT(On_Configure_Filter()));
	//connect(btnCommit_Filters, SIGNAL(clicked()), this, SLOT(On_Commit_Filters()));
	connect(lbxApplied_Filters, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(On_Applied_Filter_Dbl_Click(QListWidgetItem*)));
	connect(lbxAvailable_Filters, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(On_Available_Filter_Dbl_Click(QListWidgetItem*)));
}

bool CFilters_Window::eventFilter(QObject* object, QEvent* event) {
	if (object == lbxApplied_Filters && event->type() == QEvent::KeyPress) {
		QKeyEvent* ke = static_cast<QKeyEvent*>(event);
		On_Applied_Filter_Key_Press(ke);
		return true;
	}
	else {
		return false;
	}
}

void CFilters_Window::On_Add_Filter() {
	const auto selection = lbxAvailable_Filters->selectedItems();
	for (const auto &selected : selection) {
		const auto &desc = reinterpret_cast<CFilter_List_Item*>(selected)->description();
		CFilter_List_Item *tmp = new CFilter_List_Item{ mFilter_Chain_Configuration.Add_Link(desc.id)};
		lbxApplied_Filters->addItem(tmp);
	}
}

void CFilters_Window::On_Move_Filter_Up() {
	for (int i = 1; i < lbxApplied_Filters->count(); i++) {
		if (lbxApplied_Filters->item(i)->isSelected()) {
			lbxApplied_Filters->insertItem(i, lbxApplied_Filters->takeItem(i - 1));
			mFilter_Chain_Configuration->move(static_cast<size_t>(i), static_cast<size_t>(i) - 1);
		}
	}
}

void CFilters_Window::On_Move_Filter_Down() {
	for (int i = lbxApplied_Filters->count() - 2; i >= 0; i--) {
		if (lbxApplied_Filters->item(i)->isSelected()) {
			lbxApplied_Filters->insertItem(i, lbxApplied_Filters->takeItem(i + 1));
			mFilter_Chain_Configuration->move(static_cast<size_t>(i), static_cast<size_t>(i) + 1);
		}
	}
}

void CFilters_Window::On_Remove_Filter() {
	const auto selection = lbxApplied_Filters->selectedItems();
	for (auto* item : selection) {
		//1. delete the item from the configuration
		const auto row_index = lbxApplied_Filters->row(item);

		if (Succeeded(mFilter_Chain_Configuration->remove(row_index))) {
			//2. and delete the item from the list if everything went OK
			delete lbxApplied_Filters->takeItem(row_index);
		}
	}
}

void CFilters_Window::Configure_Filter(QListWidgetItem *item) {
	CFilter_List_Item* filter = static_cast<CFilter_List_Item*>(item);

	CFilter_Config_Window *config_wnd = new CFilter_Config_Window( filter->configuration(), nullptr );
	connect(config_wnd, SIGNAL(destroyed()), this, SLOT(On_Filter_Configure_Complete()));
	config_wnd->show();
}

void CFilters_Window::On_Applied_Filter_Dbl_Click(QListWidgetItem* item) {
	Configure_Filter(item);
}

void CFilters_Window::On_Available_Filter_Dbl_Click(QListWidgetItem* item) {
	const auto& desc = reinterpret_cast<CFilter_List_Item*>(item)->description();
	CFilter_List_Item* tmp = new CFilter_List_Item{ mFilter_Chain_Configuration.Add_Link(desc.id) };
	lbxApplied_Filters->addItem(tmp);
}

void CFilters_Window::On_Applied_Filter_Key_Press(QKeyEvent* keyevent) {
	if (keyevent->key() == Qt::Key_Delete) {
		On_Remove_Filter();
	}
}

void CFilters_Window::On_Configure_Filter() {
	const auto selection = lbxApplied_Filters->selectedItems();
	bool success = (selection.size() == 1);

	//	success
	if (success) {
		Configure_Filter(selection[0]);
	}
	else {
		QMessageBox::information(this, tr(dsInformation), tr(dsSelect_Just_One_Item));
	}
}

void CFilters_Window::On_Commit_Filters() {

	CSimulation_Window* simWindow = CSimulation_Window::Get_Instance();

	// disallow commit when simulation is in progress
	if (simWindow && simWindow->Is_Simulation_In_Progress()) {
		QMessageBox::information(this, tr(dsInformation), tr(dsSimulation_Is_In_Progress));
		return;
	}
}

void CFilters_Window::On_Filter_Configure_Complete() {
	for (int i = 0; i < lbxApplied_Filters->count(); i++) {
		auto item = reinterpret_cast<CFilter_List_Item*>(lbxApplied_Filters->item(i));
		item->Refresh();
	}
}

void CFilters_Window::On_Filter_Drag_Drop(QModelIndex idx, int start, int end, QModelIndex mdlIdx, int dst) {
	const int realDst = start < dst ? dst - 1 : dst; // moving down needs to consider that the current element is still in place

	mFilter_Chain_Configuration->move(static_cast<size_t>(start), static_cast<size_t>(realDst));
}

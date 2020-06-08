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
 * a) For non-profit, academic research, this software is available under the
 *      GPLv3 license.
 * b) For any other use, especially commercial use, you must contact us and
 *       obtain specific terms and conditions for the use of the software.
 * c) When publishing work with results obtained using this software, you agree to cite the following paper:
 *       Tomas Koutny and Martin Ubl, "Parallel software architecture for the next generation of glucose
 *       monitoring", Procedia Computer Science, Volume 141C, pp. 279-286, 2018
 */

#include "main_window.h"
#include "filters_window.h"
#include "simulation_window.h"
#include "parameters_optimization_dialog.h"

#include "../../../common/lang/dstrings.h"
#include "../../../common/utils/QtUtils.h"
#include "../../../common/utils/string_utils.h"
#include "../../../common/rtl/FilesystemLib.h"

#include <QtWidgets/QMessageBox>
#include <QtCore/QList>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QFileDialog>

#ifndef MOC_DIR
  #include "moc_main_window.cpp"
#endif

CMain_Window::CMain_Window(const std::wstring &experimental_setup_filepath, QWidget *parent) noexcept : QMainWindow(parent) {

	Setup_UI();

	if (experimental_setup_filepath.empty()) On_New_Experimental_Setup();
		else Open_Experimental_Setup(experimental_setup_filepath.c_str()); 

	this->showMaximized();
}

void CMain_Window::Setup_UI() {

	QAction* act_New_Experimental_Setup = new QAction{ tr(dsNew_Experimental_Setup), this };
	//act_New_Experimental_Setup->setShortcut(QKeySequence::New); - not yet as we do not ask to save the work yet!

	QAction* act_Open_Experimental_Setup = new QAction{ tr(dsOpen_Experimental_Setup), this };
	act_Open_Experimental_Setup->setShortcut(QKeySequence::Open);

	QAction *act_Save_Experimental_Setup = new QAction { tr(dsSave_Experimental_Setup), this };
	act_Save_Experimental_Setup->setShortcut(QKeySequence::Save);

	QAction* act_Save_Experimental_Setup_As = new QAction{ tr(dsSave_Experimental_Setup_As), this };
	act_Save_Experimental_Setup_As->setShortcut(QKeySequence::SaveAs);

	QAction *actionQuit = new QAction{tr(dsQuit), this };
	QAction* act_filters = new QAction{ tr(dsFilters), this };
	QAction* act_simulation = new QAction{ tr(dsSimulation), this };
	QAction* actOptimize_Parameters = new QAction{tr(dsOptimize_Parameters), this};

	QWidget *centralWidget;
	QVBoxLayout *verticalLayout;
	QMenuBar *menuBar;
	QMenu *menu_File;
	QMenu *menu_Tools;
	QToolBar *mainToolBar;
	QStatusBar *statusBar;

	centralWidget = new QWidget(this);
	verticalLayout = new QVBoxLayout(centralWidget);
	verticalLayout->setSpacing(0);
	verticalLayout->setContentsMargins(11, 11, 11, 11);
	verticalLayout->setContentsMargins(0, 0, 0, 0);
	pnlMDI_Content = new QMdiArea(centralWidget);
	pnlMDI_Content->setLayoutDirection(Qt::LeftToRight);
	pnlMDI_Content->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	pnlMDI_Content->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	verticalLayout->addWidget(pnlMDI_Content);

	setCentralWidget(centralWidget);
	menuBar = new QMenuBar(this);
	menuBar->setGeometry(QRect(0, 0, 640, 21));
	menu_File = new QMenu(tr(dsFile), menuBar);
	menu_Tools = new QMenu(tr(dsTools), menuBar);

	menu_Tools->addAction(act_filters);
	menu_Tools->addAction(act_simulation);
	menu_Tools->addAction(actOptimize_Parameters);

	setMenuBar(menuBar);
	mainToolBar = new QToolBar();
	addToolBar(Qt::TopToolBarArea, mainToolBar);
	statusBar = new QStatusBar(this);
	setStatusBar(statusBar);

	menuBar->addAction(menu_File->menuAction());
	menuBar->addAction(menu_Tools->menuAction());

	menu_File->addAction(act_New_Experimental_Setup);
	menu_File->addAction(act_Open_Experimental_Setup);
	menu_File->addAction(act_Save_Experimental_Setup);
	menu_File->addAction(act_Save_Experimental_Setup_As);
	menu_File->addSeparator();
	menu_File->addAction(actionQuit);

	actClose_Window = new QAction(tr(dsClose), this);
	actClose_All_Windows = new QAction(tr(dsClose_All), this);
	actTile_Vertically = new QAction(tr(dsTile_Vertically), this);
	actTile_Horizontally = new QAction(tr(dsTile_Horizontally), this);
	actCascade = new QAction(tr(dsCascade), this);
	actNext_Window = new QAction(tr(dsNext), this);	
	actNext_Window->setShortcuts(QKeySequence::NextChild);
	actPrevious_Window = new QAction(tr(dsPrevious), this);
	actPrevious_Window->setShortcuts(QKeySequence::PreviousChild);
	actWindow_Menu_Separator = new QAction(this);
	actWindow_Menu_Separator->setSeparator(true);

	mniWindow = menuBar->addMenu(tr(dsWindow));
	On_Update_Window_Menu();

	QMenu *mnuHelp = menuBar->addMenu(tr(dsHelp));
	QAction *actHelpAbout = new QAction(tr(dsAbout_Amp), this);
	mnuHelp->addAction(actHelpAbout);

	mWindowMapper = new QSignalMapper(this);

	//and connect the actions
	connect(act_New_Experimental_Setup, SIGNAL(triggered()), this, SLOT(On_New_Experimental_Setup()));
	connect(act_Open_Experimental_Setup, SIGNAL(triggered()), this, SLOT(On_Open_Experimental_Setup()));
	connect(act_Save_Experimental_Setup, SIGNAL(triggered()), this, SLOT(On_Save_Experimental_Setup()));
	connect(act_Save_Experimental_Setup_As, SIGNAL(triggered()), this, SLOT(On_Save_Experimental_Setup_As()));
	connect(actionQuit, SIGNAL(triggered()), this, SLOT(On_Quit()));
	connect(pnlMDI_Content, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(On_Update_Actions()));
	connect(actClose_Window, SIGNAL(triggered()), this, SLOT(On_Close_Window()));
	connect(actClose_All_Windows, SIGNAL(triggered()), this, SLOT(On_Close_All()));
	connect(actTile_Vertically, SIGNAL(triggered()), this, SLOT(On_Tile_Vertically()));
	connect(actTile_Horizontally, SIGNAL(triggered()), this, SLOT(On_Tile_Horizontally()));
	connect(actCascade, SIGNAL(triggered()), pnlMDI_Content, SLOT(cascadeSubWindows()));
	connect(actNext_Window, SIGNAL(triggered()), pnlMDI_Content, SLOT(activateNextSubWindow()));
	connect(actPrevious_Window, SIGNAL(triggered()),pnlMDI_Content, SLOT(activatePreviousSubWindow()));
	connect(mniWindow, SIGNAL(aboutToShow()), this, SLOT(On_Update_Window_Menu()));
	connect(actHelpAbout, SIGNAL(triggered()), this, SLOT(On_Help_About()));
	connect(act_filters, SIGNAL(triggered()), this, SLOT(On_Filters_Window()));
	connect(act_simulation, SIGNAL(triggered()), this, SLOT(On_Simulation_Window()));
	connect(actOptimize_Parameters, SIGNAL(triggered()), this, SLOT(On_Optimize_Parameters_Dialog()));

	connect(mWindowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(Set_Active_Sub_Window(QWidget*)));
}

void CMain_Window::Close_Event(QCloseEvent *event) {
	pnlMDI_Content->closeAllSubWindows();

	if (pnlMDI_Content->currentSubWindow()) {
		event->ignore();
	} else {
		event->accept();
	}
}

void CMain_Window::On_Update_Actions() {
	bool hasMdiChild = !pnlMDI_Content->subWindowList().isEmpty();

	actClose_Window->setEnabled(hasMdiChild);
	actClose_All_Windows->setEnabled(hasMdiChild);
	actTile_Vertically->setEnabled(hasMdiChild);
	actTile_Horizontally->setEnabled(hasMdiChild);
	actCascade->setEnabled(hasMdiChild);
	actNext_Window->setEnabled(hasMdiChild);
	actPrevious_Window->setEnabled(hasMdiChild);
}

void CMain_Window::On_Quit() {
    QApplication::instance()->quit();
}

void CMain_Window::On_Close_Window() {
	const auto current = pnlMDI_Content->activeSubWindow();
	if (current)
		current->close();
}

void CMain_Window::On_Close_All() {
	pnlMDI_Content->closeAllSubWindows();
}

void CMain_Window::Tile_Window(std::function<QRect()> rect_fnc) {
	if (pnlMDI_Content->subWindowList().isEmpty())
		return;

	QPoint position(0, 0);
	
	for (auto window : pnlMDI_Content->subWindowList()) {
		window->setGeometry(rect_fnc());
		window->move(position);
		position.setY(position.y() + window->height());
	}
}

void CMain_Window::On_Tile_Horizontally() {
	Tile_Window([this]() {return QRect{ 0, 0, pnlMDI_Content->width(), pnlMDI_Content->height() / pnlMDI_Content->subWindowList().count() }; });
}

void CMain_Window::On_Tile_Vertically() {
	Tile_Window([this]() {return QRect{ 0, 0, pnlMDI_Content->width() / pnlMDI_Content->subWindowList().count(), pnlMDI_Content->height() }; });
}

void CMain_Window::On_Update_Window_Menu() {
	mniWindow->clear();
	mniWindow->addAction(actClose_Window);
	mniWindow->addAction(actClose_All_Windows);
	mniWindow->addSeparator();
	mniWindow->addAction(actTile_Vertically);
	mniWindow->addAction(actTile_Horizontally);
	mniWindow->addAction(actCascade);
	mniWindow->addSeparator();
	mniWindow->addAction(actNext_Window);
	mniWindow->addAction(actPrevious_Window);
	mniWindow->addAction(actWindow_Menu_Separator);

	QList<QMdiSubWindow *> windows = pnlMDI_Content->subWindowList();
	actWindow_Menu_Separator->setVisible(!windows.isEmpty());

	for (int i = 0; i < windows.size(); ++i) {
		QMdiSubWindow *child = windows.at(i);

		QString text;
		if (i < 9) {
			text = tr("&%1 %2").arg(i + 1)
				.arg(child->windowTitle());
		} else {
			text = tr("%1 %2").arg(i + 1)
				.arg(child->windowTitle());
		}
		QAction *action = mniWindow->addAction(text);
		action->setCheckable(true);
		action->setChecked(child == pnlMDI_Content->activeSubWindow());
		connect(action, SIGNAL(triggered()), mWindowMapper, SLOT(map()));
		mWindowMapper->setMapping(action, child);
	}
}

void CMain_Window::Set_Active_Sub_Window(QWidget *window) {
	if (!window)
		return;
	pnlMDI_Content->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}

void CMain_Window::On_Help_About() {
	QMessageBox::about(this, tr(dsAbout), QString::fromUtf8(rsAbout_Text));
}

void CMain_Window::On_Filters_Window() {
	CFilters_Window::Show_Instance(mFilter_Configuration, pnlMDI_Content);
}

void CMain_Window::On_Simulation_Window() {
	CSimulation_Window::Show_Instance(mFilter_Configuration.get(), pnlMDI_Content);
}

void CMain_Window::Check_And_Display_Error_Description(const HRESULT rc, refcnt::Swstr_list errors) {
	QString error_string;

	if (rc != S_OK)
		error_string = tr(dsSave_Experimental_Setup_Failed) + "0x" + QString::number(rc, 16) + "\n ("
		+ QString::fromWCharArray(Describe_Error(rc)) + ")";

	
	if (errors->empty() != S_OK) {
		if (!error_string.isEmpty()) error_string += "\n";

		error_string += dsErrors_Warnings_Hints;
		error_string += "\n";

		errors.for_each([&error_string](auto str) {
			error_string += QString::fromStdWString(str);
			error_string += "\n";
		});
	}

		
	if (!error_string.isEmpty())
		QMessageBox::warning(this, tr(dsWarning), error_string);
}


void CMain_Window::Open_Experimental_Setup(const wchar_t* file_path) {
	pnlMDI_Content->closeAllSubWindows();
	if (pnlMDI_Content->activeSubWindow()) return;	//some window has not closed

	refcnt::Swstr_list errors;
	mFilter_Configuration = scgms::SPersistent_Filter_Chain_Configuration{};	//reset the current configuration
	HRESULT rc = mFilter_Configuration ? mFilter_Configuration->Load_From_File(file_path, errors.get()) : E_FAIL;
	Check_And_Display_Error_Description(rc, errors);

	if (rc == S_OK) {
		setWindowTitle(tr(dsGlucose_Prediction).arg(Native_Slash(file_path)));
		On_Filters_Window();
	} else if (rc == ERROR_FILE_NOT_FOUND)
		On_New_Experimental_Setup();
}


void CMain_Window::On_New_Experimental_Setup() {
	pnlMDI_Content->closeAllSubWindows();
	if (pnlMDI_Content->activeSubWindow()) return;	//some window has not closed

	mFilter_Configuration = scgms::SPersistent_Filter_Chain_Configuration{};
	if (mFilter_Configuration) {
		setWindowTitle(tr(dsGlucose_Prediction).arg(dsUnsaved_Experimental_Setup));
		On_Filters_Window();
	} else 
		Check_And_Display_Error_Description(E_FAIL, refcnt::Swstr_list{});
}

void CMain_Window::On_Open_Experimental_Setup() {
	QString selfilter;
	QString filepath;
	try {
		filepath = QFileDialog::getOpenFileName(this, tr(dsOpen_Experimental_Setup),
			QString::fromStdWString(Get_Application_Dir().wstring()), tr(dsExperimental_Setup_File_Mask), &selfilter);
	}
	catch (...) {

	}

	if (filepath.isEmpty() || selfilter.isEmpty())
		return;
	
	Open_Experimental_Setup(filepath.toStdWString().c_str());		
}

void CMain_Window::On_Save_Experimental_Setup() {
	refcnt::Swstr_list errors;
	HRESULT rc = mFilter_Configuration->Save_To_File(nullptr, errors.get());

	if (rc == E_ILLEGAL_METHOD_CALL) On_Save_Experimental_Setup_As();
		else Check_And_Display_Error_Description(rc, errors);	
}

void CMain_Window::On_Save_Experimental_Setup_As() {
	QString selfilter;
	QString filepath;
	try {
		filepath = QFileDialog::getSaveFileName(this, tr(dsSave_Experimental_Setup_As),
			QString::fromStdWString(Get_Application_Dir().wstring()), tr(dsExperimental_Setup_File_Mask), &selfilter);
	}
	catch (...) {

	}

	if (filepath.isEmpty() || selfilter.isEmpty())
		return;

	refcnt::Swstr_list errors;
	HRESULT rc = mFilter_Configuration->Save_To_File(filepath.toStdWString().c_str(), errors.get());
	if (rc == S_OK)
		setWindowTitle(tr(dsGlucose_Prediction).arg(Native_Slash(filepath.toStdWString().c_str())));

	Check_And_Display_Error_Description(rc, errors);
}


void CMain_Window::On_Optimize_Parameters_Dialog() {
	CParameters_Optimization_Dialog *dlg = new CParameters_Optimization_Dialog{ refcnt::make_shared_reference_ext<scgms::SFilter_Chain_Configuration, scgms::IFilter_Chain_Configuration>(mFilter_Configuration.get(), true), this };
	dlg->show();
}

QString CMain_Window::Native_Slash(const wchar_t* path) {
	return QString::fromStdString(filesystem::path{ path }.string() );
}
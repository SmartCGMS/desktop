#include "../../../common/lang/dstrings.h"

#include "mainwindow.h"

#include <QtWidgets/QMessageBox>
#include <QtCore/QList>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMdiSubWindow>

#ifndef MOC_DIR
  #include "moc_mainwindow.cpp"
#endif

CMain_Window::CMain_Window(QWidget *parent) : QMainWindow(parent) {
	Setup_UI();
      
	this->showMaximized();
}

void CMain_Window::Setup_UI() {	
	QAction *actionQuit;
	QWidget *centralWidget;
	QVBoxLayout *verticalLayout;
	QMenuBar *menuBar;
	QMenu *menu_File;
	QMenu *menu_Tools;
	QToolBar *mainToolBar;
	QStatusBar *statusBar;
		
	actionQuit = new QAction(tr(dsQuit), this);

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

	setMenuBar(menuBar);
	mainToolBar = new QToolBar();
	addToolBar(Qt::TopToolBarArea, mainToolBar);
	statusBar = new QStatusBar(this);
	setStatusBar(statusBar);

	menuBar->addAction(menu_File->menuAction());
	menuBar->addAction(menu_Tools->menuAction());
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

	setWindowTitle(tr(dsGlucose_Prediction));

	//and connect the actions
	connect(actionQuit, SIGNAL(triggered()), this, SLOT(On_Quit()));
	connect(pnlMDI_Content, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(On_Update_Actions()));
	connect(actClose_All_Windows, SIGNAL(triggered()), this, SLOT(On_Close_All()));
	connect(actTile_Vertically, SIGNAL(triggered()), this, SLOT(On_Tile_Vertically()));
	connect(actTile_Horizontally, SIGNAL(triggered()), this, SLOT(On_Tile_Horizontally()));
	connect(actCascade, SIGNAL(triggered()), pnlMDI_Content, SLOT(cascadeSubWindows()));
	connect(actNext_Window, SIGNAL(triggered()), pnlMDI_Content, SLOT(activateNextSubWindow()));
	connect(actPrevious_Window, SIGNAL(triggered()),pnlMDI_Content, SLOT(activatePreviousSubWindow()));
	connect(mniWindow, SIGNAL(aboutToShow()), this, SLOT(On_Update_Window_Menu()));
	connect(actHelpAbout, SIGNAL(triggered()), this, SLOT(On_Help_About()));

	connect(mWindowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));
}

void CMain_Window::Close_Event(QCloseEvent *event) {
	pnlMDI_Content->closeAllSubWindows();

	if (pnlMDI_Content->currentSubWindow())  {
		event->ignore();
	}
	else  {		
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
    qApp->quit();
}

template <typename CWindow, typename... Args>
void CMain_Window::Show_Window(Args... args) {
	
	bool created;
	CWindow* wnd = CWindow::Instance(pnlMDI_Content, &created, args);

	if (created) {
		wnd->show();
	}
}


void CMain_Window::On_Close_All() {
	
}

void CMain_Window::Tile_Window(std::function<QRect()> rect_fnc) {
	if (pnlMDI_Content->subWindowList().isEmpty())
		return;

	QPoint position(0, 0);

	foreach(QMdiSubWindow *window, pnlMDI_Content->subWindowList()) {		
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

	for (int i = 0; i < windows.size(); ++i)  {
		QMdiSubWindow *child = windows.at(i);

		QString text;
		if (i < 9)  {
			text = tr("&%1 %2").arg(i + 1)
				.arg(child->windowTitle ());
		}
		else  {
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
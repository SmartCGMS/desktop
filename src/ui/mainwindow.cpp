#include "../../../../common/iface/DataDictionary.h"

#include "mainwindow.h"
#include "patientlist.h"
#include "generalinfowindow.h"
#include "measuredvalueswindow.h"
#include "exportstatswindow.h"
#include "../lang/dstrings.h"
#include "../misc/help.h"

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QList>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMdiSubWindow>

#ifndef MOC_DIR
  #include "moc_mainwindow.cc"
#endif

MainWindow::MainWindow(QSqlDatabase *db, QWidget *parent) : mDb(db),
    QMainWindow(parent) {
	SetupUI();

    
    mSubjectDbId = InvalidDbId;

    this->showMaximized();

    CPatientList::Instance(this, mDb);
}

MainWindow::~MainWindow() {    
}

void MainWindow::SetupUI() {		
	QAction *actionClose;
	QAction *actionQuit;
	QAction *action_Patients;
	QAction *action_General;
	QAction *action_Measured_Values;
	QWidget *centralWidget;
	QVBoxLayout *verticalLayout;
	QMenuBar *menuBar;
	QMenu *menu_File;
	QMenu *menu_Tools;
	QToolBar *mainToolBar;
	QStatusBar *statusBar;


	resize(640, 480);
	actionClose = new QAction(tr(dsClose), this);
	actionQuit = new QAction(tr(dsQuit), this);
	action_Patients = new QAction(tr(dsPatients), this);
	action_General = new QAction(tr(dsGeneralInfo), this);
	action_Measured_Values = new QAction(tr(dsMeasuredValues), this);

	QAction *actExportStatistics = new QAction(tr(dsExportStatistics), this);

	centralWidget = new QWidget(this);
	verticalLayout = new QVBoxLayout(centralWidget);
	verticalLayout->setSpacing(0);
	verticalLayout->setContentsMargins(11, 11, 11, 11);
	verticalLayout->setContentsMargins(0, 0, 0, 0);
	mMdiArea = new QMdiArea(centralWidget);
	mMdiArea->setLayoutDirection(Qt::LeftToRight);
	mMdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mMdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	verticalLayout->addWidget(mMdiArea);

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
	menu_File->addAction(actionClose);
	menu_File->addSeparator();
	menu_File->addAction(actionQuit);

	menu_Tools->addAction(action_General);
	menu_Tools->addAction(action_Measured_Values);
	menu_Tools->addAction(actExportStatistics);
	menu_Tools->addSeparator();
	menu_Tools->addAction(action_Patients);

	mCloseMDIChildAction = new QAction(tr(dsClose), this);
	mCloseAllAction = new QAction(tr(dsCloseAll), this);
	mTileActionVertically = new QAction(tr(dsTileVertically), this);
	mTileActionHorizontally = new QAction(tr(dsTileHorizontally), this);
	mCascadeAction = new QAction(tr(dsCascade), this);
	mNextAction = new QAction(tr(dsNext), this);	
	mNextAction->setShortcuts(QKeySequence::NextChild);
	mPreviousAction = new QAction(tr(dsPrevious), this);
	mPreviousAction->setShortcuts(QKeySequence::PreviousChild);
	mWindowMenuseparatorAction = new QAction(this);
	mWindowMenuseparatorAction->setSeparator(true);

	mWindowMenu = menuBar->addMenu(tr(dsWindow));
	OnUpdateWindowMenu();
		
	QMenu *mnuHelp = menuBar->addMenu(tr(dsHelp));
	QAction *actViewHelp = new QAction(tr(dsViewHelp), this);
	actViewHelp->setShortcut(QKeySequence::HelpContents);
	mnuHelp->addAction(actViewHelp);
	QAction *actHelpAbout = new QAction(tr(dsAboutWAmp), this);
	mnuHelp->addAction(actHelpAbout);

	mWindowMapper = new QSignalMapper(this);

	setWindowTitle(tr(dsGlucosePrediction).arg(mDb->databaseName()));

	//and connect the actions
	connect(actionQuit, SIGNAL(triggered()), this, SLOT(on_actionQuit_triggered()));
	connect(actionClose, SIGNAL(triggered()), this, SLOT(OnFileClose()));
	connect(action_Patients, SIGNAL(triggered()), this, SLOT(on_action_Patients_triggered()));
	connect(action_General, SIGNAL(triggered()), this, SLOT(on_action_General_triggered()));
	connect(action_Measured_Values, SIGNAL(triggered()), this, SLOT(on_action_Measured_Values_triggered()));
	connect(actExportStatistics, SIGNAL(triggered()), this, SLOT(OnExportStatistics()));
	connect(mMdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(OnUpdateActions()));
	connect(mCloseMDIChildAction, SIGNAL(triggered()), mMdiArea, SLOT(closeActiveSubWindow()));
	connect(mCloseAllAction, SIGNAL(triggered()), this, SLOT(OnCloseAll()));
	connect(mTileActionVertically, SIGNAL(triggered()), this, SLOT(OnTileVertically()));
	connect(mTileActionHorizontally, SIGNAL(triggered()), this, SLOT(OnTileHorizontally()));
	connect(mCascadeAction, SIGNAL(triggered()), mMdiArea, SLOT(cascadeSubWindows()));
	connect(mNextAction, SIGNAL(triggered()), mMdiArea, SLOT(activateNextSubWindow()));
	connect(mPreviousAction, SIGNAL(triggered()),mMdiArea, SLOT(activatePreviousSubWindow()));
	connect(mWindowMenu, SIGNAL(aboutToShow()), this, SLOT(OnUpdateWindowMenu()));
	connect(actViewHelp, SIGNAL(triggered()), this, SLOT(OnViewHelp()));
	connect(actHelpAbout, SIGNAL(triggered()), this, SLOT(OnHelpAbout()));

	connect(mWindowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));
}

void MainWindow::closeEvent(QCloseEvent *event) {
	mMdiArea->closeAllSubWindows();

	if (mMdiArea->currentSubWindow())  {
		event->ignore();
	}
	else  {		
		event->accept();
	}
}

void MainWindow::OnUpdateActions() {
	bool hasMdiChild = !mMdiArea->subWindowList().isEmpty();
	

	mCloseMDIChildAction->setEnabled(hasMdiChild);
	mCloseAllAction->setEnabled(hasMdiChild);
	mTileActionVertically->setEnabled(hasMdiChild);
	mTileActionHorizontally->setEnabled(hasMdiChild);
	mCascadeAction->setEnabled(hasMdiChild);
	mNextAction->setEnabled(hasMdiChild);
	mPreviousAction->setEnabled(hasMdiChild);

}

void MainWindow::CloseCurrentSubject() {
    mMdiArea->closeAllSubWindows();

    mSubjectDbId = InvalidDbId;
}

void MainWindow::OpenSubject(int dbid) {
    if (mSubjectDbId != InvalidDbId) {
        CloseCurrentSubject();
        qApp->processEvents();
    }

    mSubjectDbId = dbid;
   
	ShowMDISubWindow<CMeasuredValues>();
}

void MainWindow::DeletingSubject(int dbid) {
    if (dbid == mSubjectDbId) {
        CloseCurrentSubject();
    }
}

void MainWindow::OnFileClose() {
	CloseCurrentSubject();
}

void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
}

void MainWindow::on_action_Patients_triggered() {
    CPatientList::Instance(this, mDb);
}

void MainWindow::on_action_General_triggered() {
    //let's show a general information about the current subject
    //http://symfony-world.blogspot.cz/2013/02/qt-menu-mdi-child-window-example.html
    ShowMDISubWindow<CGeneralInfo>();
}


void MainWindow::on_action_Measured_Values_triggered() {
    ShowMDISubWindow<CMeasuredValues>();
}


template <typename WIDGET>
void MainWindow::ShowMDISubWindow() {
    if (mSubjectDbId == InvalidDbId) {
        QMessageBox::information(this, tr(dsError), tr(dsOpenSubjectFirst));
        return;
    }


    bool created;
	WIDGET* wnd = WIDGET::Instance(mMdiArea, &created, mDb, mSubjectDbId);		

    if (created) {
        wnd->show();
    }
}


void MainWindow::OnCloseAll() {
	CloseCurrentSubject();
}

void MainWindow::OnTileHorizontally() {
	if (mMdiArea->subWindowList().isEmpty())
		return;

	QPoint position(0, 0);

	foreach(QMdiSubWindow *window, mMdiArea->subWindowList()) {
		QRect rect(0, 0, mMdiArea->width(), mMdiArea->height() / mMdiArea->subWindowList().count());
		window->setGeometry(rect);
		window->move(position);
		position.setY(position.y() + window->height());
	}

}
void MainWindow::OnTileVertically() {
	if (mMdiArea->subWindowList().isEmpty())
		return;

	QPoint position(0, 0);

	foreach(QMdiSubWindow *window, mMdiArea->subWindowList()) {
		QRect rect(0, 0, mMdiArea->width() / mMdiArea->subWindowList().count(), mMdiArea->height());
		window->setGeometry(rect);
		window->move(position);
		position.setX(position.x() + window->width());
	}

}

void MainWindow::OnUpdateWindowMenu() {
	mWindowMenu->clear();
	mWindowMenu->addAction(mCloseMDIChildAction);
	mWindowMenu->addAction(mCloseAllAction);
	mWindowMenu->addSeparator();
	mWindowMenu->addAction(mTileActionVertically);
	mWindowMenu->addAction(mTileActionHorizontally);
	mWindowMenu->addAction(mCascadeAction);
	mWindowMenu->addSeparator();
	mWindowMenu->addAction(mNextAction);
	mWindowMenu->addAction(mPreviousAction);
	mWindowMenu->addAction(mWindowMenuseparatorAction);

	QList<QMdiSubWindow *> windows = mMdiArea->subWindowList();
	mWindowMenuseparatorAction->setVisible(!windows.isEmpty());

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
		QAction *action = mWindowMenu->addAction(text);
		action->setCheckable(true);
		action->setChecked(child == mMdiArea->activeSubWindow());
		connect(action, SIGNAL(triggered()), mWindowMapper, SLOT(map()));
		mWindowMapper->setMapping(action, child);
	}
}

void MainWindow::setActiveSubWindow(QWidget *window) {
	if (!window)
		return;
	mMdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}

void MainWindow::OnHelpAbout() {
	QMessageBox::about(this, tr(dsAbout), QString::fromUtf8(rsAboutText));
}

void MainWindow::OnViewHelp() {
	Help.ShowHelp();
}

void MainWindow::OnExportStatistics() {
	CExportStatsWindow *wnd = new CExportStatsWindow(mDb, 0);
	wnd->show();
}

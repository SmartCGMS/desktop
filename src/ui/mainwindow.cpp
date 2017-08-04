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
	QAction *actionClose;
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
	mMdiArea = new QMdiArea(centralWidget);
	mMdiArea->setLayoutDirection(Qt::LeftToRight);
	mMdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mMdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	verticalLayout->addWidget(mMdiArea);

	setCentralWidget(centralWidget);
	menuBar = new QMenuBar(this);
	menuBar->setGeometry(QRect(0, 0, 640, 21));
	menu_File = new QMenu(tr(dsFile), menuBar);
	setMenuBar(menuBar);
	mainToolBar = new QToolBar();
	addToolBar(Qt::TopToolBarArea, mainToolBar);
	statusBar = new QStatusBar(this);
	setStatusBar(statusBar);

	menuBar->addAction(menu_File->menuAction());
	menuBar->addAction(menu_Tools->menuAction());
	menu_File->addAction(actionQuit);


	mCloseMDIChildAction = new QAction(tr(dsClose), this);
	mCloseAllAction = new QAction(tr(dsClose_All), this);
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
	QAction *actHelpAbout = new QAction(tr(dsAboutWAmp), this);
	mnuHelp->addAction(actHelpAbout);

	mWindowMapper = new QSignalMapper(this);

	setWindowTitle(tr(dsGlucosePrediction));

	//and connect the actions
	connect(actionQuit, SIGNAL(triggered()), this, SLOT(on_actionQuit_triggered()));
	connect(mMdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(OnUpdateActions()));
	connect(mCloseMDIChildAction, SIGNAL(triggered()), mMdiArea, SLOT(closeActiveSubWindow()));
	connect(mCloseAllAction, SIGNAL(triggered()), this, SLOT(OnCloseAll()));
	connect(mTileActionVertically, SIGNAL(triggered()), this, SLOT(OnTileVertically()));
	connect(mTileActionHorizontally, SIGNAL(triggered()), this, SLOT(OnTileHorizontally()));
	connect(mCascadeAction, SIGNAL(triggered()), mMdiArea, SLOT(cascadeSubWindows()));
	connect(mNextAction, SIGNAL(triggered()), mMdiArea, SLOT(activateNextSubWindow()));
	connect(mPreviousAction, SIGNAL(triggered()),mMdiArea, SLOT(activatePreviousSubWindow()));
	connect(mWindowMenu, SIGNAL(aboutToShow()), this, SLOT(OnUpdateWindowMenu()));
	connect(actHelpAbout, SIGNAL(triggered()), this, SLOT(OnHelpAbout()));

	connect(mWindowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));
}

void CMain_Window::closeEvent(QCloseEvent *event) {
	mMdiArea->closeAllSubWindows();

	if (mMdiArea->currentSubWindow())  {
		event->ignore();
	}
	else  {		
		event->accept();
	}
}

void CMain_Window::OnUpdateActions() {
	bool hasMdiChild = !mMdiArea->subWindowList().isEmpty();
	

	mCloseMDIChildAction->setEnabled(hasMdiChild);
	mCloseAllAction->setEnabled(hasMdiChild);
	mTileActionVertically->setEnabled(hasMdiChild);
	mTileActionHorizontally->setEnabled(hasMdiChild);
	mCascadeAction->setEnabled(hasMdiChild);
	mNextAction->setEnabled(hasMdiChild);
	mPreviousAction->setEnabled(hasMdiChild);

}


void CMain_Window::On_Quit() {
    qApp->quit();
}


void CMain_Window::OnCloseAll() {
	CloseCurrentSubject();
}

void CMain_Window::OnTileHorizontally() {
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
void CMain_Window::OnTileVertically() {
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

void CMain_Window::OnUpdateWindowMenu() {
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

void CMain_Window::setActiveSubWindow(QWidget *window) {
	if (!window)
		return;
	mMdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}

void CMain_Window::OnHelpAbout() {
	QMessageBox::about(this, tr(dsAbout), QString::fromUtf8(rsAboutText));
}
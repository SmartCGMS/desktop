#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMdiArea>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QtCore/QSignalMapper>

class CMain_Window : public QMainWindow {
    Q_OBJECT
private:    
	QMdiArea *mMdiArea;
	QMenu* mWindowMenu;
	QAction *mCloseMDIChildAction, *mCloseAllAction,
	        *mTileActionVertically, *mTileActionHorizontally,
	        *mCascadeAction, *mNextAction, 
			*mPreviousAction, *mWindowMenuseparatorAction;
	QSignalMapper *mWindowMapper;
	
	void SetupUI();
	
	void closeEvent(QCloseEvent *event);
private slots:
	void On_actQuit();

	void OnExportStatistics();

	void OnFileClose();	
	void OnUpdateActions();
	void OnCloseAll();
	void OnTileVertically();
	void OnTileHorizontally();	
	void OnUpdateWindowMenu();

	void OnViewHelp();
	void OnHelpAbout();

	void setActiveSubWindow(QWidget *window);
public:
    CMain_Window (QWidget *parent = nullptr);
    ~CMain_Window ();
};
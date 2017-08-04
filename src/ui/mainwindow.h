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
	
	void Setup_UI();
	
	void Close_Event(QCloseEvent *event);
private slots:
	void On_Quit();
	void On_Update_Actions();
	void On_Close_All();
	void On_Tile_Vertically();
	void On_Tile_Horizontally();	
	void On_Update_Window_Menu();	
	void On_Help_About();

	void Set_Active_Sub_Window(QWidget *window);
public:
    CMain_Window (QWidget *parent = nullptr);  
};
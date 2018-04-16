#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMdiArea>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#include <QtCore/QSignalMapper>

#include "../../../common/desktop-console/filter_chain.h"

class CMain_Window : public QMainWindow {
    Q_OBJECT
protected:
	CFilter_Chain mFilter_Configuration;
private:    
	QMdiArea *pnlMDI_Content;
	QMenu* mniWindow;
	QAction *actClose_Window, *actClose_All_Windows,
	        *actTile_Vertically, *actTile_Horizontally,
	        *actCascade, *actNext_Window, 
			*actPrevious_Window, *actWindow_Menu_Separator;
	QSignalMapper *mWindowMapper;
	
	void Setup_UI();	
	void Close_Event(QCloseEvent *event);
protected:
	void Tile_Window(std::function<QRect()> rect_fnc);
private slots:
	void On_Save_Configuration();
	void On_Quit();
	void On_Update_Actions();
	void On_Close_All();
	void On_Tile_Vertically();
	void On_Tile_Horizontally();	
	void On_Update_Window_Menu();	
	void On_Help_About();
	void On_Filters_Window();
	void On_Simulation_Window();

	void Set_Active_Sub_Window(QWidget *window);
public:
    CMain_Window (QWidget *parent = nullptr);  
};
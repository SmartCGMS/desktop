/**
 * SmartCGMS - continuous glucose monitoring and controlling framework
 * https://diabetes.zcu.cz/
 *
 * Contact:
 * diabetes@mail.kiv.zcu.cz
 * Medical Informatics, Department of Computer Science and Engineering
 * Faculty of Applied Sciences, University of West Bohemia
 * Technicka 8
 * 314 06, Pilsen
 *
 * Licensing terms:
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * a) For non-profit, academic research, this software is available under the
 *    GPLv3 license. When publishing any related work, user of this software
 *    must:
 *    1) let us know about the publication,
 *    2) acknowledge this software and respective literature - see the
 *       https://diabetes.zcu.cz/about#publications,
 *    3) At least, the user of this software must cite the following paper:
 *       Parallel software architecture for the next generation of glucose
 *       monitoring, Proceedings of the 8th International Conference on Current
 *       and Future Trends of Information and Communication Technologies
 *       in Healthcare (ICTH 2018) November 5-8, 2018, Leuven, Belgium
 * b) For any other use, especially commercial use, you must contact us and
 *    obtain specific terms and conditions for the use of the software.
 */

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
    CMain_Window (QWidget *parent = nullptr) noexcept;  
};
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

#pragma once

#include <array>
#include <vector>

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMdiArea>
#include <QtGui/QCloseEvent>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtWidgets/QAction>
#else
#include <QtGui/QAction>
#endif
#include <QtWidgets/QMenu>
#include <QtCore/QSignalMapper>
#include <QtGui/QDragEnterEvent>
#include <QtCore/QMimeData>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>

#include <scgms/rtl/FilterLib.h>
#include <scgms/rtl/FilesystemLib.h>

class CMain_Window : public QMainWindow {
	Q_OBJECT

	protected:
		std::wstring mFilter_Configuration_File_Path;
		scgms::SPersistent_Filter_Chain_Configuration mFilter_Configuration;

		filesystem::path mStorage_Path;

		static constexpr size_t Max_Recent_File_Count = 15;
		std::vector<filesystem::path> mRecent_Files;

	private:
		QMdiArea *pnlMDI_Content = nullptr;
		QMenu* mniWindow = nullptr;
		QMenu* mniRecent_Files = nullptr;
		QAction *actClose_Window = nullptr, *actClose_All_Windows,
				*actTile_Vertically, *actTile_Horizontally,
				*actCascade, *actNext_Window,
				*actPrevious_Window, *actWindow_Menu_Separator;
		QSignalMapper *mWindowMapper;
	
		void Setup_UI();
		void Setup_Storage();
		void Close_Event(QCloseEvent *event);
		void Update_Recent_Files();
		void Save_Recent_Files();
		void Push_Recent_File(const filesystem::path& path);

	protected:
		void Check_And_Display_Error_Description(const HRESULT rc, refcnt::Swstr_list errors);	
		void Open_Experimental_Setup(const std::wstring &file_path);	
		QString Native_Slash(const std::wstring& path);

		void Tile_Window(std::function<QRect()> rect_fnc);

	private slots:
		void On_New_Experimental_Setup();
		void On_Open_Experimental_Setup();
		void On_Save_Experimental_Setup();
		void On_Save_Experimental_Setup_As();
		void On_Quit();
		void On_Update_Actions();
		void On_Close_Window();
		void On_Close_All();
		void On_Tile_Vertically();
		void On_Tile_Horizontally();
		void On_Update_Window_Menu();
		void On_Help_About();
		void On_Filters_Window();
		void On_Simulation_Window();
		void On_Optimize_Parameters_Dialog();
		void On_Open_Recent_Experimental_Setup(QAction* action);

		void Set_Active_Sub_Window(QWidget *window);

	protected:
		void dragEnterEvent(QDragEnterEvent* event) override;
		void dragMoveEvent(QDragMoveEvent* event) override;
		void dragLeaveEvent(QDragLeaveEvent* event) override;
		void dropEvent(QDropEvent* event) override;
	public:
		CMain_Window(const std::wstring &experimental_setup_filepath, QWidget *parent = nullptr) noexcept;
};

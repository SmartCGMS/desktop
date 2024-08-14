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


#include <scgms/rtl/FilterLib.h>
#include "helpers/FilterListItem.h"
#include <atomic>

#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QListWidget>

class CFilters_Window : public QMdiSubWindow {
	Q_OBJECT
	protected:
		scgms::SPersistent_Filter_Chain_Configuration& mFilter_Chain_Configuration;
		QListWidget* lbxApplied_Filters = nullptr;
		QListWidget* lbxAvailable_Filters = nullptr;

	private:
		static std::atomic<CFilters_Window*> mInstance;

	protected:
		void Setup_UI();
		void Configure_Filter(QListWidgetItem *item);
		bool eventFilter(QObject* object, QEvent* event);

	protected slots:
		void On_Add_Filter();
		void On_Move_Filter_Up();
		void On_Move_Filter_Down();
		void On_Remove_Filter();
		void On_Configure_Filter();
		void On_Commit_Filters();
		void On_Applied_Filter_Dbl_Click(QListWidgetItem* item);
		void On_Available_Filter_Dbl_Click(QListWidgetItem* item);
		void On_Applied_Filter_Key_Press(QKeyEvent* keyevent);
		void On_Filter_Configure_Complete();
		void On_Filter_Drag_Drop(QModelIndex idx, int start, int end, QModelIndex mdlIdx, int dst);

	public:
		static CFilters_Window* Show_Instance(scgms::SPersistent_Filter_Chain_Configuration &filter_chain_configuration, QWidget *owner);
		CFilters_Window(scgms::SPersistent_Filter_Chain_Configuration &filter_chain_configuration, QWidget *owner);
		virtual ~CFilters_Window();
};

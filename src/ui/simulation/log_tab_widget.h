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

#include <QtWidgets/QListWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTableView>
#include <QtCore/QAbstractTableModel>

#include "abstract_simulation_tab.h"
#include <scgms/rtl/referencedImpl.h>

/*
* Log display subtab widget - raw view
*/
class CLog_Subtab_Raw_Widget : public CAbstract_Simulation_Tab_Widget {
	Q_OBJECT

	signals:
		void On_Log_Message(QString msg);

	protected slots:
		void Slot_Log_Message(QString msg);

	protected:
		// log contents display - text edit
		QTextEdit * mLogContents;

	public:
		explicit CLog_Subtab_Raw_Widget(QWidget *parent = 0);

		virtual CAbstract_Simulation_Tab_Widget* Clone() override;

		// when a new log message is available
		void Log_Message(const std::wstring &msg);
		// sets contents
		void Set_Contents(const QString& contents);
};

/*
 * QTableView model for log lines
 */
class CLog_Table_Model : public QAbstractTableModel {
	Q_OBJECT
	protected:
		std::vector<std::wstring> mHeaderTitles;
		std::vector<std::vector<QString>> mLogRows;

	public:
		explicit CLog_Table_Model(QObject *parent = 0) noexcept;
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
		bool setData(const QModelIndex &index, const QVariant &value, int role);
		QVariant headerData(int section, Qt::Orientation orientation, int role) const;
		bool insertRows(int position, int rows, const QModelIndex &index);

		// appends parsed log line to table view
		void Log_Message(const std::wstring &msg);
};

/*
* Log display subtab widget - table view
*/
class CLog_Subtab_Table_Widget : public CAbstract_Simulation_Tab_Widget {
	Q_OBJECT

	signals:
		void On_Log_Message(QString msg);

	protected slots:
		void Slot_Log_Message(QString msg);

	protected:
		// table view for log messages
		QTableView* mTableView;
		// table model for log messages
		CLog_Table_Model* mModel;

	public:
		explicit CLog_Subtab_Table_Widget(QWidget *parent = 0);

		virtual CAbstract_Simulation_Tab_Widget* Clone() override;

		// when a new log message is available
		void Log_Message(const std::wstring &msg);

		void Append_From_Model(CLog_Table_Model* source);
};

/*
 * Log display widget
 */
class CLog_Tab_Widget : public CAbstract_Simulation_Tab_Widget {
	Q_OBJECT

	protected:
		QTabWidget *mTabWidget;
		CLog_Subtab_Raw_Widget *mRawLogWidget, *mConfig_Errors;
		CLog_Subtab_Table_Widget *mTableLogWidget;

	protected:
		void Init_Layout();

	public:
		explicit CLog_Tab_Widget(QWidget *parent = 0);
		// clone constructor
		CLog_Tab_Widget(CLog_Subtab_Raw_Widget* cloned_raw, CLog_Subtab_Table_Widget* cloned_table);

		virtual CAbstract_Simulation_Tab_Widget* Clone() override;

		// when a new log message is available
		void Log_Message(const std::wstring &msg);
		void Log_Config_Errors(refcnt::Swstr_list errors);
};

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
 * Univerzitni 8
 * 301 00, Pilsen
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
 * a) For non-profit, academic research, this software is available under the
 *      GPLv3 license.
 * b) For any other use, especially commercial use, you must contact us and
 *       obtain specific terms and conditions for the use of the software.
 * c) When publishing work with results obtained using this software, you agree to cite the following paper:
 *       Tomas Koutny and Martin Ubl, "Parallel software architecture for the next generation of glucose
 *       monitoring", Procedia Computer Science, Volume 141C, pp. 279-286, 2018
 */

#pragma once

#include <QtWidgets/QListWidget>
#include <QtWidgets/QTextEdit>

#include "../../../../common/iface/UIIface.h"
#include "../../../../common/iface/FilterIface.h"

#include "../../../../common/rtl/FilterLib.h"

#include "abstract_simulation_tab.h"

#include <QtWidgets/QTableView>
#include <QtCore/QAbstractTableModel>
#include <array>

/*
 * QTableView model for error values
 */

namespace CErrors_Tab_Widget_internal {

	struct TSignal_Error_Inspection{
		std::wstring description;
		scgms::SSignal_Error_Inspection signal_error;
		scgms::TSignal_Error recent_abs_error;
		scgms::TSignal_Error recent_rel_error;
		double r5, r10, r25, r50;	//inverse ECDF for relative errors
	};

	class CError_Table_Model : public QAbstractTableModel {
		Q_OBJECT
	protected:
		std::vector<TSignal_Error_Inspection> mSignal_Errors;
	public:
		explicit CError_Table_Model(QObject *parent = 0) noexcept;

		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role) const;
		
		CError_Table_Model* Clone(QObject *parent = 0);
		
		void On_Filter_Configured(scgms::IFilter *filter);
		void Update_Errors();
		void Clear_Filters();
	};

}

/*
 * Error metrics display widget
 */
class CErrors_Tab_Widget : public CAbstract_Simulation_Tab_Widget {
	Q_OBJECT
protected:
	// table view for error metrics
	QTableView* mTableView;
	// table model for error metrics
	CErrors_Tab_Widget_internal::CError_Table_Model* mModel;
	// stored signal names
	std::map<GUID, std::wstring> mSignalNames;

public slots:
	void Export_CSV_Button_Clicked();

public:
	explicit CErrors_Tab_Widget(QWidget *parent = 0) noexcept;
	virtual CAbstract_Simulation_Tab_Widget* Clone() override; 		
	void Refresh();	
	void On_Filter_Configured(scgms::IFilter *filter);
	void Clear_Filters();
};

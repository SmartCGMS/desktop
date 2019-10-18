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
		glucose::SSignal_Error_Inspection signal_error;
		glucose::TSignal_Error recent_abs_error;
		glucose::TSignal_Error recent_rel_error;
	};

	class CError_Table_Model : public QAbstractTableModel {
		Q_OBJECT
	protected:
		// signal row mapping (for fast lookup)
		std::map<GUID, int> mSignalRow;
		// signal name vector; the order matches with errors vector below and with display order
		std::vector<std::wstring> mSignalNameList;
		// errors container; the order matches with signal name list above and with display order
		std::vector<std::array<glucose::TError_Markers, static_cast<size_t>(glucose::NError_Type::count)>> mErrors;

		// next row to be assigned to signal
		int mMaxSignalRow;

	protected:
		std::vector<TSignal_Error_Inspection> mSignal_Errors;
	public:
		explicit CError_Table_Model(QObject *parent = 0) noexcept;
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
		bool setData(const QModelIndex &index, const QVariant &value, int role);
		QVariant headerData(int section, Qt::Orientation orientation, int role) const;
		bool insertRows(int position, int rows, const QModelIndex &index);

		// sets error values from given container for given signal and type
		void Set_Error(const GUID& signal_id, std::wstring signal_name, const glucose::TError_Markers& errors, const glucose::NError_Type type); //remove
		// clones signal map and stores errors from another model
		void Set_From_Model(const std::map<GUID, int>& srcSignalMap, const std::vector<std::wstring>& srcSignalNameMap, CError_Table_Model* source); //remove
		// just calls Set_From_Model from within
		void Clone_To_Model(CError_Table_Model* dest); //remove?

		void On_Filter_Configured(glucose::IFilter *filter);
		void Update_Errors();
	};

}

/*
 * Error metrics display widget
 */
class CErrors_Tab_Widget : public CAbstract_Simulation_Tab_Widget
{
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

		virtual CAbstract_Simulation_Tab_Widget* Clone() override; //should we remove this one as well?
		// updates error metrics of given signal - legacy method to be removed
		void Update_Error_Metrics(const GUID& signal_id, glucose::TError_Markers& container, glucose::NError_Type type);

		void Refresh();

		// resets the model (i.e. on simulation start)
		void Reset();

		void Clone_From_Model(CErrors_Tab_Widget_internal::CError_Table_Model* source);

		void On_Filter_Configured(glucose::IFilter *filter);
};

#pragma once

#include <QtWidgets/QListWidget>
#include <QtWidgets/QTextEdit>

#include "../../../../common/iface/UIIface.h"

#include "abstract_simulation_tab.h"

#include <QtWidgets/QTableView>
#include <QtCore/QAbstractTableModel>
#include <array>

/*
 * QTableView model for error values
 */
class CError_Table_Model : public QAbstractTableModel
{
		Q_OBJECT

	protected:
		// signal row mapping (for fast lookup)
		std::map<GUID, int> mSignalRow;
		// signal name vector; the order matches with errors vector below and with display order
		std::vector<std::wstring> mSignalNameList;
		// errors container; the order matches with signal name list above and with display order
		std::vector<std::array<glucose::TError_Container, glucose::Error_Type_Count>> mErrors;

		// next row to be assigned to signal
		int mMaxSignalRow;

	public:
		CError_Table_Model(QObject *parent);
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
		bool setData(const QModelIndex &index, const QVariant &value, int role);
		QVariant headerData(int section, Qt::Orientation orientation, int role) const;
		bool insertRows(int position, int rows, const QModelIndex &index);

		// sets error values from given container for given signal and type
		void Set_Error(const GUID& signal_id, std::wstring signal_name, const glucose::TError_Container& errors, const glucose::NError_Type type);
};

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
		CError_Table_Model* mModel;
		// stored signal names
		std::map<GUID, std::wstring> mSignalNames;

	public:
		explicit CErrors_Tab_Widget(QWidget *parent = 0);

		// updates error metrics of given signal
		void Update_Error_Metrics(const GUID& signal_id, glucose::TError_Container& container, glucose::NError_Type type);
		// resets the model (i.e. on simulation start)
		void Reset();
};

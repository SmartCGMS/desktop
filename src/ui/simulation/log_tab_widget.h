#pragma once

#include <QtWidgets/QListWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTableView>
#include <QtCore/QAbstractTableModel>

#include "../../../../common/desktop-console/filter_chain.h"
#include "abstract_simulation_tab.h"

/*
* Log display subtab widget - raw view
*/
class CLog_Subtab_Raw_Widget : public CAbstract_Simulation_Tab_Widget
{
		Q_OBJECT

	protected:
		// log contents display - text edit
		QTextEdit * mLogContents;

	public:
		explicit CLog_Subtab_Raw_Widget(QWidget *parent = 0);

		// when a new log message is available
		void Log_Message(const std::wstring &msg);
};


/*
 * QTableView model for log lines
 */
class CLog_Table_Model : public QAbstractTableModel
{
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
class CLog_Subtab_Table_Widget : public CAbstract_Simulation_Tab_Widget
{
		Q_OBJECT

	protected:
		// table view for log messages
		QTableView* mTableView;
		// table model for log messages
		CLog_Table_Model* mModel;

	public:
		explicit CLog_Subtab_Table_Widget(QWidget *parent = 0);

		// when a new log message is available
		void Log_Message(const std::wstring &msg);
};

/*
 * Log display widget
 */
class CLog_Tab_Widget : public CAbstract_Simulation_Tab_Widget
{
		Q_OBJECT

	protected:
		QTabWidget* mTabWidget;
		CLog_Subtab_Raw_Widget* mRawLogWidget;
		CLog_Subtab_Table_Widget* mTableLogWidget;

	public:
		explicit CLog_Tab_Widget(QWidget *parent = 0);

		// when a new log message is available
		void Log_Message(const std::wstring &msg);
};

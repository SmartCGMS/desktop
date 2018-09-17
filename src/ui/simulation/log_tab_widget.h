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

		virtual CAbstract_Simulation_Tab_Widget* Clone() override;

		// when a new log message is available
		void Log_Message(const std::wstring &msg);
		// sets contents
		void Set_Contents(const QString& contents);
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

		virtual CAbstract_Simulation_Tab_Widget* Clone() override;

		// when a new log message is available
		void Log_Message(const std::wstring &msg);

		void Append_From_Model(CLog_Table_Model* source);
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

	protected:
		void Init_Layout();

	public:
		explicit CLog_Tab_Widget(QWidget *parent = 0);
		// clone constructor
		CLog_Tab_Widget(CLog_Subtab_Raw_Widget* cloned_raw, CLog_Subtab_Table_Widget* cloned_table);

		virtual CAbstract_Simulation_Tab_Widget* Clone() override;

		// when a new log message is available
		void Log_Message(const std::wstring &msg);
};

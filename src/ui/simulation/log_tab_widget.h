#pragma once

#include <QtWidgets/QListWidget>
#include <QtWidgets/QTextEdit>

#include "../../../../common/desktop-console/filter_chain.h"
#include "abstract_simulation_tab.h"

/*
 * Log display widget
 */
class CLog_Tab_Widget : public CAbstract_Simulation_Tab_Widget
{
		Q_OBJECT

	protected:
		// log contents display - text edit
		QTextEdit* mLogContents;

	public:
		explicit CLog_Tab_Widget(QWidget *parent = 0);

		// when a new log message is available
		void Log_Message(const std::wstring &msg);
};

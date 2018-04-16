#include "log_tab_widget.h"

#include "../../../../common/lang/dstrings.h"
#include "../../../../../common/QtUtils.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QScrollBar>
#include <fstream>

#include <QtCore/QTimer>
#include <QtCore/QEventLoop>

#include "moc_log_tab_widget.cpp"

CLog_Tab_Widget::CLog_Tab_Widget(QWidget *parent)
	: CAbstract_Simulation_Tab_Widget(parent)
{
	QLabel *logLabel = new QLabel(tr(dsLog_Tab));
	mLogContents = new QTextEdit();

	QGridLayout *mainLayout = new QGridLayout;
	mainLayout->addWidget(logLabel);
	mainLayout->addWidget(mLogContents);
	setLayout(mainLayout);
}

void CLog_Tab_Widget::Log_Message(const wchar_t* msg)
{
	// copy, then pass by value
	std::wstring logmsg(msg);

	QEventLoop loop;
	Q_UNUSED(loop);
	QTimer::singleShot(0, this, [this, logmsg]()
	{
		mLogContents->append(StdWStringToQString(logmsg));
	});
}

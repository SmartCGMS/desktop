#pragma once

#include <QtWidgets/QListWidget>

class CAbstract_Simulation_Tab_Widget : public QWidget
{
		Q_OBJECT
	public:
		explicit CAbstract_Simulation_Tab_Widget(QWidget *parent = 0);

		virtual void Update_View_Size();
};

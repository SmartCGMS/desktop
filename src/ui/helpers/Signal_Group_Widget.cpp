#include "Signal_Group_Widget.h"
#include "../../../../../common/QtUtils.h"
#include "../simulation_window.h"

#include <QtWidgets/QGridLayout>

CSignal_Group_Widget::CSignal_Group_Widget(const GUID& signalId, QWidget* parent)
	: QGroupBox(parent), mSignal_Id(signalId)
{
	QGridLayout *layout = new QGridLayout();
	setLayout(layout);

	std::wstring signalName;
	CSimulation_Window* simwin = CSimulation_Window::Get_Instance();
	if (simwin)
		signalName = simwin->Get_Signal_Name(mSignal_Id);

	mCheckBox = new QCheckBox(StdWStringToQString(signalName));
	mCheckBox->setChecked(true);
	layout->addWidget(mCheckBox, 0, 0);
}

CSignal_Group_Widget::~CSignal_Group_Widget()
{
	//
}

bool CSignal_Group_Widget::Is_Checked() const
{
	return mCheckBox->isChecked();
}

const GUID& CSignal_Group_Widget::Get_Signal_Id() const
{
	return mSignal_Id;
}

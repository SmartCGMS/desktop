#include "Time_Segment_Group_Widget.h"
#include "../../../../../common/QtUtils.h"
#include "../../../../common/lang/dstrings.h"

#include <QtWidgets/QGridLayout>

CTime_Segment_Group_Widget::CTime_Segment_Group_Widget(uint64_t segmentId, QWidget* parent)
	: QGroupBox(parent), mSegment_Id(segmentId)
{
	QGridLayout *layout = new QGridLayout();
	setLayout(layout);

	mCheckBox = new QCheckBox(tr(dsTime_Segments_Panel_Segment_Name).arg(mSegment_Id));
	mCheckBox->setChecked(true);
	layout->addWidget(mCheckBox, 0, 0);
}

CTime_Segment_Group_Widget::~CTime_Segment_Group_Widget()
{
	//
}

bool CTime_Segment_Group_Widget::Is_Checked() const
{
	return mCheckBox->isChecked();
}

void CTime_Segment_Group_Widget::Set_Checked(bool state)
{
	mCheckBox->setChecked(state);
}

uint64_t CTime_Segment_Group_Widget::Get_Segment_Id() const
{
	return mSegment_Id;
}

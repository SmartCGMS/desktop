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

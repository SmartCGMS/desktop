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

#include "Signal_Group_Widget.h"
#include "../../../../../common/QtUtils.h"
#include "../simulation_window.h"
#include "../../../../common/rtl/UILib.h"

#include <QtWidgets/QGridLayout>

CSignal_Group_Widget::CSignal_Group_Widget(const GUID& signalId, QWidget* parent)
	: QGroupBox(parent), mSignal_Id(signalId)
{
	QGridLayout *layout = new QGridLayout();
	setLayout(layout);

	std::wstring signalName;
	CSimulation_Window* simwin = CSimulation_Window::Get_Instance();
	if (simwin) {
		glucose::CSignal_Names names;
		signalName = names.Get_Name(mSignal_Id);
	}

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
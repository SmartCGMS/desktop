/**
 * SmartCGMS - continuous glucose monitoring and controlling framework
 * https://diabetes.zcu.cz/
 *
 * Copyright (c) since 2018 University of West Bohemia.
 *
 * Contact:
 * diabetes@mail.kiv.zcu.cz
 * Medical Informatics, Department of Computer Science and Engineering
 * Faculty of Applied Sciences, University of West Bohemia
 * Univerzitni 8, 301 00 Pilsen
 * Czech Republic
 * 
 * 
 * Purpose of this software:
 * This software is intended to demonstrate work of the diabetes.zcu.cz research
 * group to other scientists, to complement our published papers. It is strictly
 * prohibited to use this software for diagnosis or treatment of any medical condition,
 * without obtaining all required approvals from respective regulatory bodies.
 *
 * Especially, a diabetic patient is warned that unauthorized use of this software
 * may result into severe injure, including death.
 *
 *
 * Licensing terms:
 * Unless required by applicable law or agreed to in writing, software
 * distributed under these license terms is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * a) For non-profit, academic research, this software is available under the
 *      GPLv3 license.
 * b) For any other use, especially commercial use, you must contact us and
 *       obtain specific terms and conditions for the use of the software.
 * c) When publishing work with results obtained using this software, you agree to cite the following paper:
 *       Tomas Koutny and Martin Ubl, "Parallel software architecture for the next generation of glucose
 *       monitoring", Procedia Computer Science, Volume 141C, pp. 279-286, 2018
 */

#include "Signal_Group_Widget.h"
#include "../../../../common/utils/QtUtils.h"
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
		scgms::CSignal_Description names;
		signalName = names.Get_Name(mSignal_Id);
		names.Get_Reference_Signal_Id(mSignal_Id, mReference_Signal_Id);
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

const GUID& CSignal_Group_Widget::Get_Reference_Signal_Id() const
{
	return mReference_Signal_Id;
}

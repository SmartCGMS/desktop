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
 * a) This file is available under the Apache License, Version 2.0.
 * b) When publishing any derivative work or results obtained using this software, you agree to cite the following paper:
 *    Tomas Koutny and Martin Ubl, "SmartCGMS as a Testbed for a Blood-Glucose Level Prediction and/or 
 *    Control Challenge with (an FDA-Accepted) Diabetic Patient Simulation", Procedia Computer Science,  
 *    Volume 177, pp. 354-362, 2020
 */

#include "Time_Segment_Group_Widget.h"
#include <scgms/utils/QtUtils.h>
#include <scgms/lang/dstrings.h>

#include <QtWidgets/QGridLayout>

CTime_Segment_Group_Widget::CTime_Segment_Group_Widget(uint64_t segmentId, QWidget* parent)
	: QGroupBox(parent), mSegment_Id(segmentId) {

	QGridLayout *layout = new QGridLayout();
	setLayout(layout);

	mCheckBox = new QCheckBox(tr(dsTime_Segments_Panel_Segment_Name).arg(mSegment_Id));
	mCheckBox->setChecked(true);
	layout->addWidget(mCheckBox, 0, 0);
}

CTime_Segment_Group_Widget::~CTime_Segment_Group_Widget() {
	//
}

bool CTime_Segment_Group_Widget::Is_Checked() const {
	return mCheckBox->isChecked();
}

void CTime_Segment_Group_Widget::Set_Checked(bool state) {
	mCheckBox->setChecked(state);
}

uint64_t CTime_Segment_Group_Widget::Get_Segment_Id() const {
	return mSegment_Id;
}

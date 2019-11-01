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
 * Univerzitni 8
 * 301 00, Pilsen
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

#pragma once

#include "parameters_optimization_dialog.h"

#include "..\..\..\common\lang\dstrings.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QPushButton>

#include "moc_parameters_optimization_dialog.cpp"

CParameters_Optimization_Dialog::CParameters_Optimization_Dialog(glucose::SFilter_Chain_Configuration configuration, QWidget *parent) :
	mConfiguration(configuration), QDialog(parent) {


	Setup_UI(configuration);
}

void CParameters_Optimization_Dialog::Setup_UI(glucose::SFilter_Chain_Configuration configuration) {
	setWindowTitle(dsOptimize_Parameters);


	QVBoxLayout* vertical_layout = new QVBoxLayout();
	setLayout(vertical_layout);

	QWidget* edits = new QWidget();	
	QGridLayout *edits_layout = new QGridLayout();
	edits->setLayout(edits_layout);


	QWidget* progress = new QWidget();
	QHBoxLayout *progress_layout = new QHBoxLayout();
	progress->setLayout(progress_layout);

	QWidget* buttons = new QWidget();
	QHBoxLayout *buttons_layout = new QHBoxLayout();
	buttons->setLayout(buttons_layout);


	auto add_separator = [](QWidget *parent) {QFrame *line;
										 line = new QFrame(parent);
										 line->setFrameShape(QFrame::HLine);
										 line->setFrameShadow(QFrame::Sunken);
										 return line; };

	vertical_layout->addWidget(edits);	
	vertical_layout->addWidget(add_separator(this));
	vertical_layout->addWidget(progress);
	vertical_layout->addWidget(add_separator(this));
	vertical_layout->addWidget(buttons);
	vertical_layout->addStretch();

}


void CParameters_Optimization_Dialog::On_Solve() {

}

void CParameters_Optimization_Dialog::On_Cancel() {

}

void CParameters_Optimization_Dialog::On_Close() {

}


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
#include "..\..\..\common\rtl\UILib.h"
#include "..\..\..\common\utils\QtUtils.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QPushButton>

#include "moc_parameters_optimization_dialog.cpp"

CParameters_Optimization_Dialog::CParameters_Optimization_Dialog(glucose::SFilter_Chain_Configuration configuration, QWidget *parent) :
	mConfiguration(configuration), QDialog(parent) {

	Populate_Parameters_Info(configuration);
	Setup_UI();
}

void CParameters_Optimization_Dialog::Populate_Parameters_Info(glucose::SFilter_Chain_Configuration configuration) {

	size_t filter_index = 0;
	configuration.for_each([this, &filter_index](glucose::SFilter_Configuration_Link link) {

		link.for_each([this, filter_index, &link](glucose::SFilter_Parameter parameter) {
			if (parameter.type() == glucose::NParameter_Type::ptDouble_Array) {
				TParameters_Info info;
				
				info.filter_name = link.descriptor().description;
				info.filter_index = filter_index;
				info.parameters_name = parameter.configuration_name();

				mParameters_Info.push_back(info);
			}

		});


		filter_index++;
	});
}

void CParameters_Optimization_Dialog::Setup_UI() {
	setWindowTitle(dsOptimize_Parameters);

	
	QVBoxLayout* vertical_layout = new QVBoxLayout();
	setLayout(vertical_layout);

	QWidget* edits = new QWidget();
	
	cmbParameters = new QComboBox{ edits };
	for (size_t i = 0; i < mParameters_Info.size(); i++)
		cmbParameters->addItem(QString::fromStdWString(mParameters_Info[i].filter_name + L"/" + mParameters_Info[i].parameters_name), QVariant(i));

	cmbSolver = new QComboBox{ edits };
	for (const auto &item : glucose::get_solver_descriptors())
		cmbSolver->addItem(QString::fromStdWString(item.description), QVariant(GUID_To_QUuid(item.id)));
	
	
	edtMax_Generations = new QLineEdit{ edits };
	edtMax_Generations->setValidator(new QIntValidator(edits));
	edtMax_Generations->setText("10000");
	edtPopulation_Size = new QLineEdit{ edits };
	edtPopulation_Size->setValidator(new QIntValidator(edits));
	edtPopulation_Size->setText("100");
	
	{
		QGridLayout *edits_layout = new QGridLayout();
		edits->setLayout(edits_layout);

		edits_layout->addWidget(new QLabel{ QString::fromStdWString(dsParameters), edits }, 0, 0);			edits_layout->addWidget(cmbParameters, 0, 1);
		edits_layout->addWidget(new QLabel{ QString::fromStdWString(dsSelected_Solver), edits }, 1, 0);		edits_layout->addWidget(cmbSolver, 1, 1);
		edits_layout->addWidget(new QLabel{ dsMax_Generations, edits }, 2, 0);								edits_layout->addWidget(edtMax_Generations, 2, 1);
		edits_layout->addWidget(new QLabel{ dsPopulation_Size, edits }, 3, 0);								edits_layout->addWidget(edtPopulation_Size, 3, 1);
	}

	

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

	setMinimumSize(200, 200);	//keeping Qt happy although it should already calculate from the controls
}


void CParameters_Optimization_Dialog::On_Solve() {

}

void CParameters_Optimization_Dialog::On_Abort() {

}

void CParameters_Optimization_Dialog::On_Close() {

}


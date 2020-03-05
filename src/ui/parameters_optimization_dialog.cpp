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

#include "parameters_optimization_dialog.h"

#include "../../../common/lang/dstrings.h"
#include "../../../common/rtl/SolverLib.h"
#include "../../../common/rtl/UILib.h"
#include "../../../common/rtl/qdb_connector.h"
#include "../../../common/utils/QtUtils.h"
#include "../../../common/utils/string_utils.h"

#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QPushButton>

#include <thread>
#include <chrono>
#include <cmath>

#include "moc_parameters_optimization_dialog.cpp"

CParameters_Optimization_Dialog::CParameters_Optimization_Dialog(scgms::SFilter_Chain_Configuration configuration, QWidget *parent)
	: QDialog(parent), mConfiguration(configuration) {

	mIs_Solving = false;
	Populate_Parameters_Info(configuration);
	Setup_UI();
}

CParameters_Optimization_Dialog::~CParameters_Optimization_Dialog() {
	Stop_Threads();
}

void CParameters_Optimization_Dialog::Populate_Parameters_Info(scgms::SFilter_Chain_Configuration configuration) {
	auto models = scgms::get_model_descriptors();
	const scgms::CSignal_Description signal_descriptors;

	auto complete_description = [&models, &signal_descriptors](std::wstring &description, scgms::SFilter_Configuration_Link link) {

		link.for_each([&models, &description, &signal_descriptors](scgms::SFilter_Parameter parameter) {
			// model signal - append signal name
			if (parameter.type() == scgms::NParameter_Type::ptModel_Produced_Signal_Id) {
				bool found = false;
				for (auto& model : models)
				{
					for (size_t i = 0; i < model.number_of_calculated_signals; i++) {
						HRESULT rc;
						if (model.calculated_signal_ids[i] == parameter.as_guid(rc))
							if (rc == S_OK) {
								description += L" - ";
								description += signal_descriptors.Get_Name(model.calculated_signal_ids[i]);
								found = true;
								break;
							}
					}

					if (found)
						break;
				}
			}
			// model - append model description
			else if (parameter.type() == scgms::NParameter_Type::ptSignal_Model_Id || parameter.type() == scgms::NParameter_Type::ptDiscrete_Model_Id) {
				for (auto& model : models) {
					HRESULT rc;
					if (model.id == parameter.as_guid(rc))
						if (rc == S_OK) {
							description += L" - ";
							description += model.description;
							break;
						}
				}
			}
		});
	};


	size_t filter_index = 0;
	configuration.for_each([this, &filter_index, &complete_description](scgms::SFilter_Configuration_Link link) {

		link.for_each([this, filter_index, &link, &complete_description](scgms::SFilter_Parameter parameter) {
			if (parameter.type() == scgms::NParameter_Type::ptDouble_Array) {
				TParameters_Info info;
				
				info.filter_name = link.descriptor().description;
				complete_description(info.filter_name, link);

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
		cmbParameters->addItem(QString::fromStdWString(mParameters_Info[i].filter_name + L" / " + mParameters_Info[i].parameters_name), QVariant(static_cast<int>(i)));

	cmbSolver = new QComboBox{ edits };
	for (const auto &item : scgms::get_solver_descriptors())
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

		edits_layout->addWidget(new QLabel{ tr(Narrow_WChar(dsParameters).c_str()), edits }, 0, 0);			edits_layout->addWidget(cmbParameters, 0, 1);
		edits_layout->addWidget(new QLabel{ tr(Narrow_WChar(dsSelected_Solver).c_str()), edits }, 1, 0);	edits_layout->addWidget(cmbSolver, 1, 1);
		edits_layout->addWidget(new QLabel{ dsMax_Generations, edits }, 2, 0);								edits_layout->addWidget(edtMax_Generations, 2, 1);
		edits_layout->addWidget(new QLabel{ dsPopulation_Size, edits }, 3, 0);								edits_layout->addWidget(edtPopulation_Size, 3, 1);
	}

	

	QWidget* progress = new QWidget();
	{
		QVBoxLayout *progress_layout = new QVBoxLayout();
		progress->setLayout(progress_layout);

		lblSolver_Info = new QLabel{ QString::fromStdWString(dsSolver_Progress_Box_Title), progress };
		barProgress = new QProgressBar{ progress };
		barProgress->setMinimum(0);
		barProgress->setMaximum(100);		

		progress_layout->addWidget(lblSolver_Info);
		progress_layout->addWidget(barProgress);
	}

	QWidget* buttons = new QWidget();
	{
		QHBoxLayout *buttons_layout = new QHBoxLayout();
		buttons->setLayout(buttons_layout);

		btnSolve = new QPushButton{ dsSolve, buttons };
		btnStop = new QPushButton{ dsStop, buttons };
		btnClose = new QPushButton{ dsClose, buttons };

		buttons_layout->addWidget(btnSolve);	buttons_layout->addWidget(btnStop);	buttons_layout->addWidget(btnClose);
		connect(btnSolve, SIGNAL(clicked()), this, SLOT(On_Solve()));
		connect(btnStop, SIGNAL(clicked()), this, SLOT(On_Stop()));
		connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
	}


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

	connect(this, SIGNAL(Update_Progress_Signal()), this, SLOT(On_Update_Progress()), Qt::BlockingQueuedConnection);
}


void CParameters_Optimization_Dialog::On_Solve() {
	if (!mIs_Solving) {
		const QVariant solver_variant = cmbSolver->currentData();
		const size_t filter_info_index = static_cast<size_t>(cmbParameters->currentIndex());

		if (!solver_variant.isNull() && (filter_info_index < mParameters_Info.size())) {
			mIs_Solving = true;

			Stop_Threads();

			mProgress = solver::Null_Solver_Progress;
			mSolver_Thread = std::make_unique<std::thread>(
				[this, &solver_variant, filter_info_index]() {
					refcnt::Swstr_list error_description;
					HRESULT res = scgms::Optimize_Parameters(mConfiguration, mParameters_Info[filter_info_index].filter_index, mParameters_Info[filter_info_index].parameters_name.c_str(),
						Setup_Filter_DB_Access, nullptr,
						QUuid_To_GUID(solver_variant.toUuid()),
						edtPopulation_Size->text().toInt(),
						edtMax_Generations->text().toInt(),
						mProgress,
						error_description);

					mIs_Solving = false;
					mProgress.cancelled = TRUE;	//stops mProgress_Update_Thread

					if (res != S_OK)
						lblSolver_Info->setText(tr(dsSolver_Status_Failed));
					else
						emit Update_Progress_Signal();
			});

			mProgress_Update_Thread = std::make_unique<std::thread>([this]() {
				while (mProgress.cancelled == FALSE) {
					if (mIs_Solving) emit Update_Progress_Signal();
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				}
			});
		
		}
		
	}
}

void CParameters_Optimization_Dialog::On_Update_Progress() {
	if (mIs_Solving) {
		if (mProgress.max_progress > 0) {
			barProgress->setValue(static_cast<int>(std::round(100.0*mProgress.current_progress / mProgress.max_progress)));
			lblSolver_Info->setText(QString(tr(dsBest_Metric_Label)).arg(mProgress.best_metric));
		} else
			lblSolver_Info->setText(QString(tr(dsSolver_Status_In_Progress)));
	} else {
		lblSolver_Info->setText(QString(tr(dsSolver_Status_Stopped)) + ", "+ QString(tr(dsBest_Metric_Label)).arg(mProgress.best_metric));
		barProgress->setValue(0);
	}
}

void CParameters_Optimization_Dialog::On_Stop() {
	if (mIs_Solving) {
		mIs_Solving = false;
		Stop_Threads();
	}
	On_Update_Progress();
}


void CParameters_Optimization_Dialog::Stop_Threads() {
	auto wait_for_thread = [](std::unique_ptr<std::thread> &thread) {
		if (thread) {
			if (thread->joinable())
				thread->join();
			thread.reset();
		}
	};

	mProgress.cancelled = FALSE;
	wait_for_thread(mSolver_Thread);
	wait_for_thread(mProgress_Update_Thread);
}
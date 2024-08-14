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

#include "parameters_optimization_dialog.h"

#include <scgms/lang/dstrings.h>
#include <scgms/rtl/SolverLib.h>
#include <scgms/rtl/UILib.h>
#include <scgms/rtl/qdb_connector.h>
#include <scgms/utils/QtUtils.h>
#include <scgms/utils/string_utils.h>
#include <scgms/iface/SolverIface.h>
#include <scgms/utils/math_utils.h>

#include <QtWidgets/QApplication>
#include <QtCore/QDateTime>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>

#include <thread>
#include <chrono>
#include <cmath>
#include <future>

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
	auto models = scgms::get_model_descriptor_list();
	const scgms::CSignal_Description signal_descriptors;

	auto complete_description = [&models, &signal_descriptors](std::wstring &description, scgms::SFilter_Configuration_Link link) {

		link.for_each([&models, &description, &signal_descriptors](scgms::SFilter_Parameter parameter) {
			// model signal - append signal name
			if (parameter.type() == scgms::NParameter_Type::ptModel_Produced_Signal_Id) {
				bool found = false;
				for (auto& model : models) {
					for (size_t i = 0; i < model.number_of_calculated_signals; i++) {
						HRESULT rc;
						if (model.calculated_signal_ids[i] == parameter.as_guid(rc)) {
							if (rc == S_OK) {
								description += L" - ";
								description += signal_descriptors.Get_Name(model.calculated_signal_ids[i]);
								found = true;
								break;
							}
						}
					}

					if (found) {
						break;
					}
				}
			}
			// model - append model description
			else if (parameter.type() == scgms::NParameter_Type::ptSignal_Model_Id || parameter.type() == scgms::NParameter_Type::ptDiscrete_Model_Id) {
				for (auto& model : models) {
					HRESULT rc;
					if (model.id == parameter.as_guid(rc)) {
						if (rc == S_OK) {
							description += L" - ";
							description += model.description;
							break;
						}
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

	QHBoxLayout* main_layout = new QHBoxLayout();
	setLayout(main_layout);
	
	QWidget* params_box = new QWidget();
	{
		QVBoxLayout* vertical_layout = new QVBoxLayout();
		params_box->setLayout(vertical_layout);

		QWidget* edits = new QWidget();
	
		QStandardItemModel* model = new QStandardItemModel{ static_cast<int>(mParameters_Info.size()), static_cast<int>(1), edits }; // x rows, 1 col
	
		for (size_t i = 0; i < mParameters_Info.size(); i++) {

			QStandardItem* item = new QStandardItem{ QString::fromStdWString(L"[%1] " + mParameters_Info[i].filter_name + L" / " + mParameters_Info[i].parameters_name ).arg(mParameters_Info[i].filter_index)};
			item->setData(QVariant(static_cast<int>(i)));
			item->setEditable(false);

			model->setItem(static_cast<int>(i), item);
		}

		cmbParameters = new QListView{ edits };
		cmbParameters->setModel(model);
		cmbParameters->setSelectionMode(QAbstractItemView::SelectionMode::MultiSelection);

		cmbSolver = new QComboBox{ edits };
		{
			int default_solver_pos = -1;
			constexpr GUID default_solver_id = { 0x1b21b62f, 0x7c6c, 0x4027,{ 0x89, 0xbc, 0x68, 0x7d, 0x8b, 0xd3, 0x2b, 0x3c } };	// let mt metade be a default solver
			for (const auto& item : scgms::get_solver_descriptor_list()) {
				cmbSolver->addItem(QString::fromStdWString(item.description), QVariant(GUID_To_QUuid(item.id)));
			}
			cmbSolver->model()->sort(0, Qt::AscendingOrder); 

			default_solver_pos = cmbSolver->findData(GUID_To_QUuid(default_solver_id));

			if (default_solver_pos != -1) {
				cmbSolver->setCurrentIndex(default_solver_pos);
			}
		}

		edtMax_Generations = new QLineEdit{ edits };
		edtMax_Generations->setValidator(new QIntValidator(edits));
		edtMax_Generations->setText("10000");
		edtPopulation_Size = new QLineEdit{ edits };
		edtPopulation_Size->setValidator(new QIntValidator(edits));
		edtPopulation_Size->setText("100");
	
		{
			QGridLayout *edits_layout = new QGridLayout();
			edits->setLayout(edits_layout);

			const auto params = Narrow_WChar(dsParameters);
			const auto selected_solver = Narrow_WChar(dsSelected_Solver);
			edits_layout->addWidget(new QLabel{ tr(params.c_str()), edits }, 0, 0);				edits_layout->addWidget(cmbParameters, 0, 1);
			edits_layout->addWidget(new QLabel{ tr(selected_solver.c_str()), edits }, 1, 0);	edits_layout->addWidget(cmbSolver, 1, 1);
			edits_layout->addWidget(new QLabel{ dsMax_Generations, edits }, 2, 0);				edits_layout->addWidget(edtMax_Generations, 2, 1);
			edits_layout->addWidget(new QLabel{ dsPopulation_Size, edits }, 3, 0);				edits_layout->addWidget(edtPopulation_Size, 3, 1);
		}

		QWidget* progress = new QWidget();
		{
			QVBoxLayout *progress_layout = new QVBoxLayout();
			progress->setLayout(progress_layout);

			lblSolver_Info = new QLabel{ QString::fromStdWString(dsSolver_Progress_Box_Title), progress };
			barProgress = new QProgressBar{ progress };
			barProgress->setTextVisible(false);
			barProgress->setMinimum(0);
			barProgress->setMaximum(100);

			QWidget* progressLabels = new QWidget();
			{
				QHBoxLayout* labels_layout = new QHBoxLayout();
				progressLabels->setLayout(labels_layout);

				progressLabel1 = new QLabel("N/A");
				progressLabel2 = new QLabel("0 %");

				labels_layout->addWidget(progressLabel1, 0, Qt::AlignCenter);
				labels_layout->addWidget(progressLabel2, 0, Qt::AlignCenter);
			}

			progress_layout->addWidget(lblSolver_Info);
			progress_layout->addWidget(barProgress);
			progress_layout->addWidget(progressLabels);
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

		auto add_separator = [](QWidget *parent) {
			QFrame *line;
			line = new QFrame(parent);
			line->setFrameShape(QFrame::HLine);
			line->setFrameShadow(QFrame::Sunken);
			return line;
		};

		vertical_layout->addWidget(edits);
		vertical_layout->addWidget(new QLabel{ QString::fromUtf8(dsParameters_Optimization_Use).arg(solver::Maximum_Objectives_Count) , this });
		vertical_layout->addWidget(add_separator(this));
		vertical_layout->addWidget(progress);
		vertical_layout->addWidget(add_separator(this));
		vertical_layout->addWidget(buttons);
		vertical_layout->addStretch();
	}

	QWidget* metric_history_box = new QWidget();
	{
		QVBoxLayout* mhb_layout = new QVBoxLayout();
		metric_history_box->setLayout(mhb_layout);

		QWidget* history = new QWidget();

		lstMetricHistory = new QTableWidget{ history };
		lstMetricHistory->setColumnCount(solver::Maximum_Objectives_Count + 2); // 2 for current and maximum generation count

		QStringList headers;
		headers << "Gen." << "Max.";
		for (size_t i = 0; i < solver::Maximum_Objectives_Count; i++) {
			headers << QString::number(i + 1);
		}

		lstMetricHistory->setShowGrid(true);
		lstMetricHistory->setSelectionMode(QAbstractItemView::SingleSelection);
		lstMetricHistory->setSelectionBehavior(QAbstractItemView::SelectRows);
		lstMetricHistory->setHorizontalHeaderLabels(headers);
		lstMetricHistory->resizeColumnsToContents();
		lstMetricHistory->verticalHeader()->hide();

		for (size_t i = 0; i < solver::Maximum_Objectives_Count; i++) {
			lstMetricHistory->hideColumn(static_cast<int>(i + 2));
		}

		QWidget* startLbl = new QWidget();
		{
			QHBoxLayout* ly = new QHBoxLayout();
			startLbl->setLayout(ly);

			timestampLabelStart = new QLabel("N/A");

			ly->addWidget(new QLabel("Start:"));
			ly->addWidget(timestampLabelStart);
		}

		QWidget* endLbl = new QWidget();
		{
			QHBoxLayout* ly = new QHBoxLayout();
			endLbl->setLayout(ly);

			timestampLabelEnd = new QLabel("N/A");

			ly->addWidget(new QLabel("End:"));
			ly->addWidget(timestampLabelEnd);
		}

		mhb_layout->addWidget(new QLabel("Metric history"));
		mhb_layout->addWidget(history, 1);
		mhb_layout->addWidget(startLbl);
		mhb_layout->addWidget(endLbl);

		metric_history_box->setMinimumSize(300, 300);
	}

	main_layout->addWidget(params_box);
	main_layout->addWidget(metric_history_box);

	setMinimumSize(300, 200);	//keeping Qt happy although it should already calculate from the controls

	connect(this, SIGNAL(Update_Progress_Signal()), this, SLOT(On_Update_Progress()), Qt::BlockingQueuedConnection);
}

void CParameters_Optimization_Dialog::On_Solve() {
	if (!mIs_Solving) {
		const QVariant solver_variant = cmbSolver->currentData();
		if (!solver_variant.isNull()) {
			mChosen_Solver_Id = QUuid_To_GUID(solver_variant.toUuid());
		}

		mSolve_filter_info_indices.clear();
		mSolve_filter_parameter_names.clear();

		auto model = cmbParameters->selectionModel();
		QStandardItemModel* casted_model = dynamic_cast<QStandardItemModel*>(model->model());
		foreach(const QModelIndex & index, model->selectedIndexes()) {
			const size_t filter_info_index = casted_model->itemFromIndex(index)->data().toInt();
			mSolve_filter_info_indices.push_back(mParameters_Info[filter_info_index].filter_index);
			mSolve_filter_parameter_names.push_back(mParameters_Info[filter_info_index].parameters_name.c_str());
		}

		lstMetricHistory->clearContents();
		lstMetricHistory->setRowCount(0);

		timestampLabelStart->setText("N/A");
		timestampLabelEnd->setText("N/A");

		if (!solver_variant.isNull() && (!mSolve_filter_info_indices.empty())) {
			mIs_Solving = true;

			Stop_Threads();

			const int popSize = edtPopulation_Size->text().toInt();
			const int maxGens = edtMax_Generations->text().toInt();

			lastMetric = solver::Nan_Fitness;
			lastProgress = 0;
			startDateTime = QDateTime::currentDateTime();
			timestampLabelStart->setText(startDateTime.toLocalTime().toString());

			mProgress = solver::Null_Solver_Progress;
			mSolver_Thread = std::make_unique<std::thread>(
				[this, popSize, maxGens]() {
					refcnt::Swstr_list error_description;
					HRESULT res = scgms::Optimize_Parameters(mConfiguration, mSolve_filter_info_indices.data(), const_cast<const wchar_t**>(mSolve_filter_parameter_names.data()), mSolve_filter_info_indices.size(),
						Setup_Filter_DB_Access, nullptr,
						mChosen_Solver_Id,
						popSize,
						maxGens,
						nullptr, 0,	//additional hints
						mProgress,
						error_description);

					mIs_Solving = false;
					mProgress.cancelled = TRUE;	//stops mProgress_Update_Thread

					if (res != S_OK) {
						lblSolver_Info->setText(tr(dsSolver_Status_Failed));
					}
					else {
						emit Update_Progress_Signal();
					}
			});

			mProgress_Update_Thread = std::make_unique<std::thread>([this]() {
				while (mProgress.cancelled == FALSE) {
					if (mIs_Solving) {
						emit Update_Progress_Signal();
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
				}
			});
		}
	}
}

void CParameters_Optimization_Dialog::On_Update_Progress() {
	if (mIs_Solving) {
		if (mProgress.max_progress > 0) {

			int progressValue = static_cast<int>(std::round(100.0 * mProgress.current_progress / mProgress.max_progress));

			barProgress->setValue(progressValue);
			progressLabel1->setText(QString("%1 / %2").arg(mProgress.current_progress).arg(mProgress.max_progress));
			progressLabel2->setText(QString("%1 %").arg(progressValue));
			lblSolver_Info->setText(QString(tr(dsBest_Metric_Label)).arg(mProgress.best_metric[0]));

			if (lastProgress != mProgress.current_progress) {
				lastProgress = mProgress.current_progress;

				if (mProgress.current_progress > 0) {
					const auto msPerUnit = (QDateTime::currentDateTime().toMSecsSinceEpoch() - startDateTime.toMSecsSinceEpoch()) / mProgress.current_progress;
					const QDateTime etaEnd = startDateTime.addMSecs(msPerUnit * mProgress.max_progress);

					timestampLabelEnd->setText(QString("%1 (ETA)").arg(etaEnd.toLocalTime().toString()));
				}
				else {
					timestampLabelEnd->setText("N/A");
				}
			}

			bool changed = false;
			for (size_t i = 0; i < solver::Maximum_Objectives_Count; i++) {
				if (Is_Any_NaN(mProgress.best_metric[i])) {
					continue;
				}
				if (Is_Any_NaN(lastMetric[i]) || lastMetric[i] != mProgress.best_metric[i]) {
					changed = true;
					break;
				}
			}

			if (changed) {
				for (size_t i = 0; i < solver::Maximum_Objectives_Count; i++) {
					lastMetric[i] = mProgress.best_metric[i];
				}

				int i = lstMetricHistory->rowCount();

				lstMetricHistory->insertRow(i);
				lstMetricHistory->setItem(i, 0, new QTableWidgetItem(QString::number(mProgress.current_progress)));
				lstMetricHistory->setItem(i, 1, new QTableWidgetItem(QString::number(mProgress.max_progress)));
				for (size_t j = 0; j < solver::Maximum_Objectives_Count; j++) {
					lstMetricHistory->setItem(i, static_cast<int>(2 + j), new QTableWidgetItem(QString::number(mProgress.best_metric[j])));
					if (!Is_Any_NaN(mProgress.best_metric[j])) {
						lstMetricHistory->showColumn(static_cast<int>(j + 2));
					}
				}

				lstMetricHistory->resizeColumnsToContents();
				lstMetricHistory->scrollToBottom();
			}
		}
		else {
			lblSolver_Info->setText(QString(tr(dsSolver_Status_In_Progress)));
		}
	}
	else {
		progressLabel1->setText(QString("N/A"));
		progressLabel2->setText(QString("0 %"));
		lblSolver_Info->setText(QString(tr(dsSolver_Status_Stopped)) + ", "+ QString(tr(dsBest_Metric_Label)).arg(mProgress.best_metric[0]));
		timestampLabelEnd->setText(QDateTime::currentDateTime().toLocalTime().toString());
		barProgress->setValue(0);
	}
}

void CParameters_Optimization_Dialog::On_Stop() {
	Stop_Async();
	On_Update_Progress();
}

void CParameters_Optimization_Dialog::reject() {

	if (mIs_Solving) {
		QMessageBox::StandardButton resBtn = QMessageBox::Yes;
		resBtn = QMessageBox::question(this, tr("Solver still running"), tr("The solver is still running. Do you want to stop the solver and reject results?\n"), QMessageBox::Cancel | QMessageBox::Yes, QMessageBox::Yes);

		if (resBtn == QMessageBox::Yes) {
			Stop_Async();
			QDialog::reject();
		}
	}
	else {
		QDialog::reject();
	}
}

void CParameters_Optimization_Dialog::Stop_Threads() {
	auto wait_for_thread = [](std::unique_ptr<std::thread> &thread) {
		if (thread && thread->joinable()) {
			thread->join();
			thread.reset();
		}
	};

	mProgress.cancelled = TRUE;
	wait_for_thread(mSolver_Thread);
	wait_for_thread(mProgress_Update_Thread);
	mProgress.cancelled = FALSE;
}

void CParameters_Optimization_Dialog::Stop_Async() {
	QProgressDialog progress("Stopping the solver...", "Cancel", 0, 0, this);
	progress.setWindowTitle("Working...");
	progress.setWindowModality(Qt::ApplicationModal);
	progress.setAutoClose(false);
	progress.setBar(nullptr);
	progress.setWindowFlags(progress.windowFlags() & ~(Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint));

	progress.show();

	QList<QPushButton*> L = progress.findChildren<QPushButton*>();
	if (L.size() > 0) {
		L[0]->setDisabled(true);
	}

	auto r = std::async(std::launch::async, [this]() {
		mProgress.cancelled = TRUE;
		mIs_Solving = false;
		Stop_Threads();
	});

	while (r.wait_for(std::chrono::milliseconds(10)) == std::future_status::timeout) {
		mProgress.cancelled = TRUE;
		mIs_Solving = false;
		QApplication::processEvents();
	}
	mProgress.cancelled = FALSE;

	progress.close();
}

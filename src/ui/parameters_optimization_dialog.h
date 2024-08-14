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

#pragma once

#include <scgms/iface/SolverIface.h>
#include <scgms/rtl/FilterLib.h>

#include <QtWidgets/QDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QListView>
#include <QtWidgets/QTableWidget>
#include <QtGui/QStandardItem>
#include <QtCore/QDateTime>

#include <vector>
#include <thread>

class CParameters_Optimization_Dialog : public QDialog {
	Q_OBJECT
	protected:
		scgms::SFilter_Chain_Configuration mConfiguration;
	
		struct TParameters_Info {
			std::wstring filter_name;
			size_t filter_index = std::numeric_limits<size_t>::max();
			std::wstring parameters_name;
		};

		std::vector<TParameters_Info> mParameters_Info;

		QListView* cmbParameters = nullptr;
		QTableWidget* lstMetricHistory = nullptr;
		QComboBox* cmbSolver = nullptr;
		QLineEdit *edtMax_Generations, *edtPopulation_Size;
		QLabel *lblSolver_Info;
		QProgressBar *barProgress;
		QLabel* progressLabel1, *progressLabel2;
		QPushButton *btnSolve, *btnStop, *btnClose;
		QLabel* timestampLabelStart, *timestampLabelEnd;
		QDateTime startDateTime;
		solver::TFitness lastMetric = solver::Nan_Fitness;
		size_t lastProgress = 0;

		std::vector<size_t> mSolve_filter_info_indices;
		std::vector<const wchar_t*> mSolve_filter_parameter_names;
		GUID mChosen_Solver_Id;
	
		std::unique_ptr<std::thread> mSolver_Thread, mProgress_Update_Thread;
		solver::TSolver_Progress mProgress;
		bool mIs_Solving = false;

	protected:
		void Setup_UI();
		void Populate_Parameters_Info(scgms::SFilter_Chain_Configuration configuration);

		void Stop_Threads();
		void Stop_Async();

	signals:
		void Update_Progress_Signal();

	protected slots:
		void On_Solve();
		void On_Stop();	
		void On_Update_Progress();

		void reject();

	public:
		CParameters_Optimization_Dialog(scgms::SFilter_Chain_Configuration configuration, QWidget *parent);
		~CParameters_Optimization_Dialog();
};

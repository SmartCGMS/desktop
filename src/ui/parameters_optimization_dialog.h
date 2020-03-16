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

#pragma once

#include "../../../common/iface/SolverIface.h"
#include "../../../common/rtl/FilterLib.h"

#include <QtWidgets/QDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>

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
	void Populate_Parameters_Info(scgms::SFilter_Chain_Configuration configuration);
protected:
	QComboBox *cmbParameters = nullptr, *cmbSolver;
	QLineEdit *edtMax_Generations, *edtPopulation_Size;
	QLabel *lblSolver_Info;
	QProgressBar *barProgress;
	QPushButton *btnSolve, *btnStop, *btnClose;
	void Setup_UI();
protected:
	std::unique_ptr<std::thread> mSolver_Thread, mProgress_Update_Thread;
	solver::TSolver_Progress mProgress;
	bool mIs_Solving;

	void Stop_Threads();
signals:
	void Update_Progress_Signal();
protected slots:
	void On_Solve();
	void On_Stop();	
	void On_Update_Progress();
public:
	CParameters_Optimization_Dialog(scgms::SFilter_Chain_Configuration configuration, QWidget *parent);
	~CParameters_Optimization_Dialog();
};

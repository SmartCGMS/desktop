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

#include <atomic>
#include <vector>
#include <memory>

#include <QtCore/QSignalMapper>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QCheckBox>

#include <scgms/rtl/FilterLib.h>
#include <scgms/rtl/UILib.h>

#include "simulation/log_tab_widget.h"
#include "simulation/drawing_tab_widget.h"
#include "simulation/drawing_v2_tab_widget.h"
#include "simulation/errors_tab_widget.h"
#include "helpers/Time_Segment_Group_Widget.h"
#include "helpers/Signal_Group_Widget.h"
#include "helpers/gui_subchain.h"

class CGUI_Terminal_Filter;

/*
 * Simulation control and results window
 */
class CSimulation_Window : public QMdiSubWindow {
	Q_OBJECT
	private:
		static std::atomic<CSimulation_Window*> mInstance;

		// stored log widget
		CLog_Tab_Widget* mLogWidget = nullptr;
		// stored drawing widgets
		std::vector<CDrawing_Tab_Widget*> mDrawingWidgets;
		// stored drawing_v2 widgets
		std::vector<std::vector<std::pair<CDrawing_v2_Tab_Widget*, int>>> mDrawing_v2_Widgets;
		// stored errors widget
		CErrors_Tab_Widget* mErrorsWidget = nullptr;

		// is simulation in progress?
		bool mSimulationInProgress;
		

		scgms::SFilter_Executor mFilter_Executor;
		refcnt::SReferenced<scgms::IFilter_Chain_Configuration> mConfiguration;

		std::vector<scgms::SCalculate_Filter_Inspection> mSolver_Filters;
		scgms::CSignal_Description mSignal_Descriptors;

		CGUI_Filter_Subchain mGUI_Filter_Subchain;
		int mBase_Tab_Count = 0;

		std::unique_ptr<CGUI_Terminal_Filter> mTerminal_Filter;
		
	protected:
		// tab widget for filter outputs
		QTabWidget* mTabWidget;
		// progress bar layout
		QGroupBox* mProgressGroup;
		// time segments layout
		QWidget* mSegmentsGroup;
		// signals layout
		QGroupBox* mSignalsGroup;

		// start button instance
		QPushButton* mStartButton;
		// stop button instance
		QPushButton* mStopButton;
		// simulation control buttons
		QPushButton *mSolveParamsButton, *mSolveAndResetParamsButton;
		// signal mapper for solve dropdown menu
		QSignalMapper* mSolveSignalMapper;

		// checkbox for drawing at the end of simulation
		QCheckBox* mDrawAtShutdownCheckBox;

		typedef struct {
			size_t progress;
			double bestMetric;
			scgms::TSolver_Status status;
		} TProgress_Status_Internal;

		std::map<GUID, QProgressBar*> mProgressBars;
		std::map<GUID, QLabel*> mSolverStatusLabels;
		std::map<GUID, TProgress_Status_Internal> mSolverProgress;
		std::map<GUID, QLabel*> mBestMetricLabels;
		std::map<uint64_t, CTime_Segment_Group_Widget*> mSegmentWidgets;
		std::map<GUID, CSignal_Group_Widget*> mSignalWidgets;
		std::map<GUID, QAction*> mSignalSolveActions;

		std::vector<QWidget*> mCompletedSolverWidgets;

	signals:
		void On_Start_Time_Segment(quint64 id);
		void On_Add_Signal(QUuid id);
		void On_Update_Solver_Progress(QUuid solver);
		void On_Shut_Down_Received();

	protected slots:
		void On_Start();
		void On_Stop();
		void On_Tab_Change(int index);

		void On_Reset_And_Solve_Params();

		void On_Segments_Draw_Request();

		void On_Select_Segments_All();
		void On_Select_Segments_None();

		void On_Solve_Signal(QString);

		void Show_Tab_Context_Menu(const QPoint &point);

		void Slot_Start_Time_Segment(quint64 id);
		void Slot_Add_Signal(QUuid id);
		void Slot_Update_Solver_Progress(QUuid solver);

		void On_Draw_Shut_Down_State_Change(int state);

	protected:
		void Setup_UI();
		void Setup_Solve_Button_Menu();

		void resizeEvent(QResizeEvent* evt) override;

		void Update_Tab_View();

		void Close_Tab(int index);
		void Save_Tab_State(int index);
		void Inject_Event(const scgms::NDevice_Event_Code& code, const GUID& signal_id, const wchar_t* info, const uint64_t segment_id = scgms::Invalid_Segment_Id);
		static HRESULT IfaceCalling On_Filter_Configured(scgms::IFilter* filter, const void* data);

	public:
		// factory method for singleton initialization
		static CSimulation_Window* Show_Instance(refcnt::SReferenced<scgms::IFilter_Chain_Configuration> configuration, QWidget *owner);
		CSimulation_Window(refcnt::SReferenced<scgms::IFilter_Chain_Configuration> configuration, QWidget *owner);
		virtual ~CSimulation_Window();

		static CSimulation_Window* Get_Instance();

		bool Is_Simulation_In_Progress() const;

		void Drawing_Callback(const scgms::TDrawing_Image_Type type, const scgms::TDiagnosis diagnosis, const std::string &svg);
		void Drawing_v2_Callback(size_t filterIdx, size_t drawingIdx, const std::string& svg);
		void Update_Preferred_Drawing_Dimensions(size_t filterIdx, size_t drawingIdx, int& width, int& height);

		void Log_Callback(std::shared_ptr<refcnt::wstr_list> messages);
		void Update_Solver_Progress(const GUID& solver, size_t progress, double bestMetric, scgms::TSolver_Status status);
		void Update_Errors();
		void Update_Solver_Progress();

		void Start_Time_Segment(uint64_t segmentId);
		void Add_Signal(const GUID& signalId);
		
		void Stop_Simulation();
};

#pragma warning( push )
#pragma warning( disable : 4250 ) // C4250 - 'class1' : inherits 'class2::member' via dominance

class CGUI_Terminal_Filter : public virtual scgms::IFilter, public virtual refcnt::CNotReferenced
{
	public:
		HRESULT IfaceCalling Configure(scgms::IFilter_Configuration* configuration, refcnt::wstr_list* error_description) override;
		HRESULT IfaceCalling Execute(scgms::IDevice_Event* event) override;
};

#pragma warning( pop )

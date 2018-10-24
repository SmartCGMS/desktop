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
 *       monitoring", Proceedings of the 8th International Conference on Current
 *       and Future Trends of Information and Communication Technologies
 *       in Healthcare (ICTH 2018) November 5-8, 2018, Leuven, Belgium
 */

#pragma once

#include <atomic>
#include <vector>
#include <memory>

#include <QtCore/QUuid>
#include <QtCore/QSignalMapper>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>

#include "../../../common/iface/FilterIface.h"
#include "../../../common/desktop-console/filter_chain.h"
#include "../../../common/desktop-console/filter_chain_manager.h"
#include "../../../common/rtl/UILib.h"

#include "../filters/descriptor.h"
#include "../filters/gui_subchain.h"

#include "simulation/log_tab_widget.h"
#include "simulation/drawing_tab_widget.h"
#include "simulation/errors_tab_widget.h"
#include "helpers/Time_Segment_Group_Widget.h"
#include "helpers/Signal_Group_Widget.h"

/*
 * Simulation control and results window
 */
class CSimulation_Window : public QMdiSubWindow {
		Q_OBJECT
	private:
		static std::atomic<CSimulation_Window*> mInstance;

		// stored log widget
		CLog_Tab_Widget* mLogWidget;
		// stored drawing widgets
		std::vector<CDrawing_Tab_Widget*> mDrawingWidgets;
		// stored errors widget
		CErrors_Tab_Widget* mErrorsWidget;

		// is simulation in progress?
		bool mSimulationInProgress;

		SGUI_Filter_Subchain m_guiSubchain;
		std::vector<glucose::SCalculate_Filter_Inspection> mSolver_Filters;
		glucose::CSignal_Names mSignal_Names;

		int mBase_Tab_Count;
	protected:
		// chain holder retaining filter configuration
		std::unique_ptr<CFilter_Chain_Manager> mFilter_Chain_Manager;

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

		typedef struct {
			size_t progress;
			double bestMetric;
			glucose::TSolver_Status status;
		} TProgress_Status_Internal;

		std::map<GUID, QProgressBar*> mProgressBars;
		std::map<GUID, QLabel*> mSolverStatusLabels;
		std::map<GUID, TProgress_Status_Internal> mSolverProgress;
		std::map<GUID, QLabel*> mBestMetricLabels;
		std::map<uint64_t, CTime_Segment_Group_Widget*> mSegmentWidgets;
		std::map<GUID, CSignal_Group_Widget*> mSignalWidgets;
		std::map<GUID, QAction*> mSignalSolveActions;

		std::vector<QWidget*> mCompletedSolverWidgets;

		void Setup_UI();
		void Setup_Solve_Button_Menu();

		void resizeEvent(QResizeEvent* evt) override;

		void Update_Tab_View();

		void Close_Tab(int index);
		void Save_Tab_State(int index);

	signals:
		void On_Start_Time_Segment(quint64 id);
		void On_Add_Signal(QUuid id);
		void On_Update_Solver_Progress(QUuid solver);
		void On_Simulation_Terminate();

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
		void Slot_Simulation_Terminate();

	protected:
		void Inject_Event(const glucose::NDevice_Event_Code &code, const GUID &signal_id, const wchar_t *info, const uint64_t segment_id = glucose::Invalid_Segment_Id);
	protected:
		QUuid GUID_To_QUuid(const GUID& guid);
		GUID QUuid_To_GUID(const QUuid& uuid);
	public:
		// factory method for singleton initialization
		static CSimulation_Window* Show_Instance(CFilter_Chain &filter_chain, QWidget *owner);
		CSimulation_Window(CFilter_Chain &filter_chain, QWidget *owner);
		virtual ~CSimulation_Window();

		static CSimulation_Window* Get_Instance();

		bool Is_Simulation_In_Progress() const;
		void Update_Filter_Chain(CFilter_Chain& filter_chain);

		void Drawing_Callback(const glucose::TDrawing_Image_Type type, const glucose::TDiagnosis diagnosis, const std::string &svg);
		void Log_Callback(std::shared_ptr<refcnt::wstr_list> messages);
		void Update_Solver_Progress(const GUID& solver, size_t progress, double bestMetric, glucose::TSolver_Status status);
		void Update_Error_Metrics(const GUID& signal_id, glucose::TError_Markers& container, glucose::NError_Type type);
		void Update_Solver_Progress();

		void Start_Time_Segment(uint64_t segmentId);
		void Add_Signal(const GUID& signalId);

		void Stop_Simulation();
};

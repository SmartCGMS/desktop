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

#include "../../../common/iface/FilterIface.h"
#include "../../../common/desktop-console/filter_chain.h"
#include "../../../common/desktop-console/filter_chain_manager.h"
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

	protected:
		// chain holder retaining filter configuration
		std::unique_ptr<CFilter_Chain_Manager> mFilter_Chain_Manager;

		// tab widget for filter outputs
		QTabWidget* mTabWidget;
		// progress bar layout
		QGroupBox* mProgressGroup;
		// time segments layout
		QGroupBox* mSegmentsGroup;
		// signals layout
		QGroupBox* mSignalsGroup;

		// start button instance
		QPushButton* mStartButton;
		// stop button instance
		QPushButton* mStopButton;
		// simulation control buttons
		QPushButton *mSolveParamsButton, *mSolveAndResetParamsButton, *mSuspendSolveButton, *mResumeSolveButton, *mSimulationStepButton;
		// step amount spinbox
		QSpinBox *mStepAmountSpinBox;

		std::map<GUID, std::wstring> mSignalNames;
		std::map<GUID, QProgressBar*> mProgressBars;
		std::map<uint64_t, CTime_Segment_Group_Widget*> mSegmentWidgets;
		std::map<GUID, CSignal_Group_Widget*> mSignalWidgets;

		void Setup_UI();

		void resizeEvent(QResizeEvent* evt) override;

		void Update_Tab_View();

	protected slots:
		void On_Start();
		void On_Stop();
		void On_Tab_Change(int index);

		void On_Solve_Params();
		void On_Reset_And_Solve_Params();
		void On_Suspend_Solve();
		void On_Resume_Solve();
		void On_Simulation_Step();

		void On_Segments_Draw_Request();

	protected:
		void Inject_Event(const glucose::NDevice_Event_Code &code, const GUID &signal_id, const wchar_t *info);
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
		void Update_Solver_Progress(GUID& solver, size_t progress);
		void Update_Error_Metrics(const GUID& signal_id, glucose::TError_Markers& container, glucose::NError_Type type);

		void Start_Time_Segment(uint64_t segmentId);
		void Add_Signal(const GUID& signalId);

		std::wstring Get_Signal_Name(const GUID& guid) const;
};

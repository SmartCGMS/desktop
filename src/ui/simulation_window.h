#pragma once

#include <atomic>
#include <vector>
#include <memory>

#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>

#include "../../../common/desktop-console/filter_chain.h"
#include "../../../common/desktop-console/filter_chain_holder.h"
#include "../filters/descriptor.h"
#include "../filters/input.h"

#include "simulation/log_tab_widget.h"
#include "simulation/drawing_tab_widget.h"
#include "simulation/errors_tab_widget.h"

/*
 * Simulation control and results window
 */
class CSimulation_Window : public QMdiSubWindow
{
		Q_OBJECT

	private:
		static std::atomic<CSimulation_Window*> mInstance;

		// stored log widget
		CLog_Tab_Widget* mLogWidget;
		// stored drawing widgets
		std::vector<CDrawing_Tab_Widget*> mDrawingWidgets;
		// stored errors widget
		CErrors_Tab_Widget* mErrorsWidget;

		// input filter in chain (if present)
		std::shared_ptr<CInput_Filter> mInput_Filter;

		// is simulation in progress?
		bool mSimulationInProgress;

	protected:
		// chain holder retaining filter configuration
		std::unique_ptr<CFilter_Chain_Holder> mFilterChainHolder;

		// tab widget for filter outputs
		QTabWidget* mTabWidget;
		// progress bar layout
		QGroupBox* mProgressGroup;

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

	public:
		// factory method for singleton initialization
		static CSimulation_Window* Show_Instance(CFilter_Chain &filter_chain, QWidget *owner);
		CSimulation_Window(CFilter_Chain &filter_chain, QWidget *owner);
		virtual ~CSimulation_Window();

		static CSimulation_Window* Get_Instance();

		bool Is_Simulation_In_Progress() const;
		void Update_Filter_Chain(CFilter_Chain& filter_chain);

		void Drawing_Callback(const wchar_t* type, const wchar_t* image_data);
		void Log_Callback(const wchar_t* message);
		void Update_Solver_Progress(GUID& solver, size_t progress);
		void Update_Error_Metrics(const GUID& signal_id, glucose::TError_Container& container, glucose::NError_Type type);
};

#include "simulation_window.h"

#include "../../../common/lang/dstrings.h"
#include "../../../common/rtl/FilterLib.h"
#include "../../../common/rtl/UILib.h"
#include "../../../common/rtl/referencedImpl.h"
#include "../../../../common/QtUtils.h"

#include <QtWidgets/QSplitter>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QFrame>

#include <QtCore/QTimer>
#include <QtCore/QEventLoop>

#include "simulation/abstract_simulation_tab.h"
#include "../filters/descriptor.h"

#ifndef MOC_DIR
#include "moc_simulation_window.cpp"
#endif

std::atomic<CSimulation_Window*> CSimulation_Window::mInstance = nullptr;

CSimulation_Window* CSimulation_Window::Show_Instance(CFilter_Chain &filter_chain, QWidget *owner)
{
	if (mInstance)
	{
		mInstance.load()->showMaximized();
		return mInstance;
	}

	CSimulation_Window* tmp = nullptr;
	bool created = mInstance.compare_exchange_strong(tmp, new CSimulation_Window(filter_chain, owner));

	if (created)
		mInstance.load()->showMaximized();

	return mInstance;
}

CSimulation_Window::CSimulation_Window(CFilter_Chain &filter_chain, QWidget *owner) : mTabWidget(nullptr), QMdiSubWindow{ owner }
{
	mFilterChainHolder = std::make_unique<CFilter_Chain_Manager>(filter_chain);

	Setup_UI();

	mStopButton->setEnabled(false);
	mStartButton->setEnabled(true);
	mSimulationInProgress = false;

	auto models = glucose::get_model_descriptors();
	for (auto& model : models)
	{
		for (size_t i = 0; i < model.number_of_calculated_signals; i++)
			mSignalNames[model.calculated_signal_ids[i]] = model.description + std::wstring(L" - ") + model.calculated_signal_names[i];
	}

	for (size_t i = 0; i < glucose::signal_Virtual.size(); i++)
		mSignalNames[glucose::signal_Virtual[i]] = dsSignal_Prefix_Virtual + std::wstring(L" ") + std::to_wstring(i);
}

CSimulation_Window::~CSimulation_Window()
{
	On_Stop();
	mInstance = nullptr;
}

bool CSimulation_Window::Is_Simulation_In_Progress() const
{
	return mSimulationInProgress;
}

void CSimulation_Window::Update_Filter_Chain(CFilter_Chain& filter_chain)
{
	mFilterChainHolder = std::make_unique<CFilter_Chain_Manager>(filter_chain);
}

void CSimulation_Window::Setup_UI()
{
	setWindowTitle(tr(dsSimulation_Window));

	QGridLayout *layout = new QGridLayout();
	for (int i = 0; i < 10; i++)
		layout->setColumnStretch(i, 1);

	mStartButton = new QPushButton{ tr(dsStart) };
	layout->addWidget(mStartButton, 0, 0);
	mStopButton = new QPushButton{ tr(dsStop) };
	layout->addWidget(mStopButton, 0, 1);

	mSolveParamsButton = new QPushButton{ tr(dsSolve) };
	layout->addWidget(mSolveParamsButton, 0, 2);
	mSolveAndResetParamsButton = new QPushButton{ tr(dsSolve_Reset) };
	layout->addWidget(mSolveAndResetParamsButton, 0, 3);
	mSuspendSolveButton = new QPushButton{ tr(dsSuspend) };
	layout->addWidget(mSuspendSolveButton, 0, 4);
	mResumeSolveButton = new QPushButton{ tr(dsResume) };
	layout->addWidget(mResumeSolveButton, 0, 5);

	QWidget* simstepBox = new QWidget();
	{
		QHBoxLayout* gpBoxLayout = new QHBoxLayout();
		simstepBox->setLayout(gpBoxLayout);
		layout->addWidget(simstepBox, 0, 6);

		mStepAmountSpinBox = new QSpinBox();
		mStepAmountSpinBox->setMinimum(1);
		mStepAmountSpinBox->setMaximum(100);
		mStepAmountSpinBox->setValue(1);
		mStepAmountSpinBox->setSingleStep(1);
		gpBoxLayout->addWidget(mStepAmountSpinBox);

		mSimulationStepButton = new QPushButton{ tr(dsStep) };
		gpBoxLayout->addWidget(mSimulationStepButton);
	}

	mProgressGroup = new QGroupBox();
	QVBoxLayout *progLayout = new QVBoxLayout();
	mProgressGroup->setLayout(progLayout);
	mProgressGroup->setTitle(StdWStringToQString(dsSolver_Progress_Box_Title));

	layout->addWidget(mProgressGroup, 1, 0);

	// main tab widget, span to 11 columns for now
	mTabWidget = new QTabWidget();
	layout->addWidget(mTabWidget, 1, 1, 1, 9);

	// TODO: verify if GUI filter is actually present in filter chain
	{
		// drawing tab

		CDrawing_Tab_Widget* tab;

		tab = new CDrawing_Tab_Widget(glucose::TDrawing_Image_Type::Graph);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_Graph));

		tab = new CDrawing_Tab_Widget(glucose::TDrawing_Image_Type::Day);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_Day));

		tab = new CDrawing_Tab_Widget(glucose::TDrawing_Image_Type::Clark);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_Clark));

		tab = new CDrawing_Tab_Widget(glucose::TDrawing_Image_Type::Parkes);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_Parkes));

		tab = new CDrawing_Tab_Widget(glucose::TDrawing_Image_Type::Agp);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_AGP));

		// log tab

		mLogWidget = new CLog_Tab_Widget(this);
		mTabWidget->addTab(mLogWidget, tr(dsLog_Tab));

		// errors tab

		mErrorsWidget = new CErrors_Tab_Widget(this);
		mTabWidget->addTab(mErrorsWidget, tr(dsErrors_Tab));
	}

	QWidget *content = new QWidget{ this };
	content->setLayout(layout);

	setWidget(content);
	
	// set the window to be freed upon closing
	setAttribute(Qt::WA_DeleteOnClose, true);

	connect(mStartButton, SIGNAL(clicked()), this, SLOT(On_Start()));
	connect(mStopButton, SIGNAL(clicked()), this, SLOT(On_Stop()));
	connect(mSolveParamsButton, SIGNAL(clicked()), this, SLOT(On_Solve_Params()));
	connect(mSolveAndResetParamsButton, SIGNAL(clicked()), this, SLOT(On_Reset_And_Solve_Params()));
	connect(mSuspendSolveButton, SIGNAL(clicked()), this, SLOT(On_Suspend_Solve()));
	connect(mResumeSolveButton, SIGNAL(clicked()), this, SLOT(On_Resume_Solve()));
	connect(mSimulationStepButton, SIGNAL(clicked()), this, SLOT(On_Simulation_Step()));
	connect(mTabWidget, SIGNAL(currentChanged(int)), this, SLOT(On_Tab_Change(int)));
}

void CSimulation_Window::Update_Tab_View()
{
	if (mTabWidget)
	{
		auto tab = dynamic_cast<CAbstract_Simulation_Tab_Widget*>(mTabWidget->currentWidget());
		if (tab)
			tab->Update_View_Size();
	}
}

void CSimulation_Window::On_Tab_Change(int index)
{
	Update_Tab_View();
}

void CSimulation_Window::resizeEvent(QResizeEvent* evt)
{
	QMdiSubWindow::resizeEvent(evt);

	Update_Tab_View();
}

void CSimulation_Window::On_Start()
{
	// clean progress bars and progress bar group box
	QLayoutItem *wItem;
	while ((wItem = mProgressGroup->layout()->takeAt(0)) != nullptr)
	{
		delete wItem->widget();
		delete wItem;
	}
	mProgressBars.clear();

	QVBoxLayout* lay = dynamic_cast<QVBoxLayout*>(mProgressGroup->layout());
	if (lay)
		lay->addStretch();

	// initialize and start filter holder, this will start filters
	if (mFilterChainHolder->Init_And_Start_Filters() != S_OK)
	{
		// TODO: error message
		mFilterChainHolder->Terminate_Filters();
		return;
	}

	mSimulationInProgress = true;
	mStopButton->setEnabled(true);
	mStartButton->setEnabled(false);

	if (mErrorsWidget)
		mErrorsWidget->Reset();

	// "find and remember" input filter
	size_t filterCount = mFilterChainHolder->Get_Filter_Chain().size();
	for (size_t i = 0; i < filterCount; i++)
	{
		auto filter_id = mFilterChainHolder->Get_Filter_Id(i);
		if (filter_id == gui::input_filter_guid)
			mInput_Filter = std::dynamic_pointer_cast<CInput_Filter>(mFilterChainHolder->Get_Filter(i));
	}
}

void CSimulation_Window::On_Stop()
{
	mFilterChainHolder->Terminate_Filters();
	mSimulationInProgress = false;
	mInput_Filter = nullptr;
	mStopButton->setEnabled(false);
	mStartButton->setEnabled(true);
}

void CSimulation_Window::On_Solve_Params()
{
	if (mInput_Filter)
		mInput_Filter->Send_Force_Solve_Parameters(Invalid_GUID, false);
}

void CSimulation_Window::On_Reset_And_Solve_Params()
{
	if (mInput_Filter)
		mInput_Filter->Send_Force_Solve_Parameters(Invalid_GUID, true);
}

void CSimulation_Window::On_Suspend_Solve()
{
	if (mInput_Filter)
		mInput_Filter->Send_Suspend_Solve_Parameters(Invalid_GUID);
}

void CSimulation_Window::On_Resume_Solve()
{
	if (mInput_Filter)
		mInput_Filter->Send_Resume_Solve_Parameters(Invalid_GUID);
}

void CSimulation_Window::On_Simulation_Step()
{
	if (mInput_Filter)
		mInput_Filter->Send_Simulation_Step((size_t)mStepAmountSpinBox->value());
}

CSimulation_Window* CSimulation_Window::Get_Instance()
{
	return mInstance;
}

void CSimulation_Window::Drawing_Callback(const glucose::TDrawing_Image_Type type, const glucose::TDiagnosis diagnosis, const std::string &image_data)
{
	for (CDrawing_Tab_Widget* wg : mDrawingWidgets)
		wg->Drawing_Callback(type, diagnosis, image_data);
}

void CSimulation_Window::Log_Callback(const wchar_t* message)
{
	mLogWidget->Log_Message(message);
}

void CSimulation_Window::Update_Solver_Progress(GUID& solver, size_t progress)
{
	QEventLoop loop;
	Q_UNUSED(loop);
	QTimer::singleShot(0, this, [this, solver, progress]()
	{
		auto itr = mProgressBars.find(solver);

		if (itr == mProgressBars.end())
		{
			QProgressBar* pbar = new QProgressBar();
			mProgressBars[solver] = pbar;

			auto sigNameItr = mSignalNames.find(solver);
			if (sigNameItr == mSignalNames.end())
				mSignalNames[solver] = dsSignal_Unknown;

			QLabel* plabel = new QLabel(StdWStringToQString(mSignalNames[solver]));

			pbar->setValue((int)progress);

			QVBoxLayout* lay = dynamic_cast<QVBoxLayout*>(mProgressGroup->layout());
			if (lay)
			{
				lay->insertWidget(0, pbar);
				lay->insertWidget(0, plabel);
			}
		}
		else
			itr->second->setValue((int)progress);
	});
}

void CSimulation_Window::Update_Error_Metrics(const GUID& signal_id, glucose::TError_Markers& container, glucose::NError_Type type)
{
	mErrorsWidget->Update_Error_Metrics(signal_id, container, type);
}

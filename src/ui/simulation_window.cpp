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

#include "simulation_window.h"

#include <scgms/lang/dstrings.h>
#include <scgms/rtl/FilterLib.h>
#include <scgms/rtl/UILib.h>
#include <scgms/rtl/qdb_connector.h>
#include <scgms/rtl/referencedImpl.h>
#include <scgms/rtl/rattime.h>
#include <scgms/utils/QtUtils.h>
#include <scgms/utils/string_utils.h>

#include <QtWidgets/QSplitter>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QFrame>
#include <QtWidgets/QMenu>

#include <QtCore/QTimer>
#include <QtCore/QEventLoop>

#include "simulation/abstract_simulation_tab.h"

#ifndef MOC_DIR
	#include "moc_simulation_window.cpp"
#endif

constexpr size_t Invalid_Value = static_cast<size_t>(-1);

constexpr bool Is_Visibility_Panel_Enabled = true;

std::atomic<CSimulation_Window*> CSimulation_Window::mInstance = nullptr;

HRESULT IfaceCalling CGUI_Terminal_Filter::Configure(scgms::IFilter_Configuration* configuration, refcnt::wstr_list* error_description) {
	return S_OK;
}

HRESULT IfaceCalling CGUI_Terminal_Filter::Execute(scgms::IDevice_Event* event) {
	if (!event) {
		return E_INVALIDARG;
	}

	scgms::TDevice_Event* raw_event;
	HRESULT rc = event->Raw(&raw_event);
	if (rc != S_OK) {
		event->Release();
		return rc;
	}

	CSimulation_Window* simwin = CSimulation_Window::Get_Instance();

	if (raw_event->signal_id != Invalid_GUID) {
		simwin->Add_Signal(raw_event->signal_id);
	}

	if (raw_event->event_code == scgms::NDevice_Event_Code::Time_Segment_Start) {
		simwin->Start_Time_Segment(raw_event->segment_id);
	}
	else if (raw_event->event_code == scgms::NDevice_Event_Code::Shut_Down) {
		simwin->Stop_Simulation();
	}

	event->Release();

	return S_OK;
}

CSimulation_Window* CSimulation_Window::Show_Instance(refcnt::SReferenced<scgms::IFilter_Chain_Configuration> configuration, QWidget *owner) {
	if (mInstance) {
		mInstance.load()->showMaximized();
		return mInstance;
	}

	CSimulation_Window* tmp = nullptr;
	bool created = mInstance.compare_exchange_strong(tmp, new CSimulation_Window(configuration, owner));

	if (created) {
		mInstance.load()->showMaximized();
	}

	return mInstance;
}

CSimulation_Window::CSimulation_Window(refcnt::SReferenced<scgms::IFilter_Chain_Configuration> configuration, QWidget *owner) : 
	QMdiSubWindow{ owner }, mConfiguration(configuration), mTabWidget(nullptr) {

	Setup_UI();

	mStopButton->setEnabled(false);
	mStartButton->setEnabled(true);
	mSimulationInProgress = false;
}

CSimulation_Window::~CSimulation_Window() {
	On_Stop();

	mInstance = nullptr;
}

bool CSimulation_Window::Is_Simulation_In_Progress() const {
	return mSimulationInProgress;
}

void CSimulation_Window::Setup_Solve_Button_Menu() {
	mSolveSignalMapper = new QSignalMapper(this);
	connect(mSolveSignalMapper, SIGNAL(mapped(QString)), this, SLOT(On_Solve_Signal(QString)));

	QMenu* menu = new QMenu(this);

	auto sig_name = mSignal_Descriptors.Get_Name(scgms::signal_All);
	QAction* anyaction = menu->addAction(StdWStringToQString(sig_name.c_str()));
	connect(anyaction, SIGNAL(triggered()), mSolveSignalMapper, SLOT(map()));
	mSolveSignalMapper->setMapping(anyaction, StdWStringToQString(GUID_To_WString(scgms::signal_All)));

	menu->addSeparator();

	auto models = scgms::get_model_descriptor_list();
	for (const auto& model : models) {
		for (size_t i = 0; i < model.number_of_calculated_signals; i++) {
			auto& action = mSignalSolveActions[model.calculated_signal_ids[i]];

			sig_name = mSignal_Descriptors.Get_Name(model.calculated_signal_ids[i]);
			action = menu->addAction(StdWStringToQString(sig_name.c_str()));
			action->setVisible(false);
			connect(action, SIGNAL(triggered()), mSolveSignalMapper, SLOT(map()));
			mSolveSignalMapper->setMapping(action, StdWStringToQString(GUID_To_WString(model.calculated_signal_ids[i])));
		}
	}

	mSolveParamsButton->setMenu(menu);
}

void CSimulation_Window::Setup_UI() {
	setWindowTitle(tr(dsSimulation_Window));
	setWindowIcon(QIcon(":/app/appicon.png"));

	QGridLayout *layout = new QGridLayout();
	for (int i = 0; i < 10; i++) {
		layout->setColumnStretch(i, 1);
	}

	constexpr int IconSize = 16;
	constexpr int SimButtonHeight = 32;

	mStartButton = new QPushButton{ " " + tr(dsStart) };
	mStartButton->setMinimumHeight(SimButtonHeight);
	mStartButton->setIcon(QIcon(":/app/simulation-start.png"));
	mStartButton->setIconSize(QSize(IconSize, IconSize));
	layout->addWidget(mStartButton, 0, 0);

	mStopButton = new QPushButton{ " " + tr(dsStop)};
	mStopButton->setMinimumHeight(SimButtonHeight);
	mStopButton->setIcon(QIcon(":/app/simulation-stop.png"));
	mStopButton->setIconSize(QSize(IconSize, IconSize));
	layout->addWidget(mStopButton, 0, 1);

	mSolveParamsButton = new QPushButton{ " " + tr(dsSolve) };
	mSolveParamsButton->setMinimumHeight(SimButtonHeight);
	mSolveParamsButton->setIcon(QIcon(":/app/simulation-solve.png"));
	mSolveParamsButton->setIconSize(QSize(IconSize, IconSize));
	layout->addWidget(mSolveParamsButton, 0, 2);

	mSolveAndResetParamsButton = new QPushButton{ " " + tr(dsSolve_Reset) };
	mSolveAndResetParamsButton->setMinimumHeight(SimButtonHeight);
	mSolveAndResetParamsButton->setIcon(QIcon(":/app/simulation-recalculate.png"));
	mSolveAndResetParamsButton->setIconSize(QSize(IconSize, IconSize));
	layout->addWidget(mSolveAndResetParamsButton, 0, 3);

	Setup_Solve_Button_Menu();

	QWidget* leftPanel = new QWidget();
	QVBoxLayout* leftPanelLayout = new QVBoxLayout();
	{
		leftPanel->setLayout(leftPanelLayout);

		mProgressGroup = new QGroupBox();
		QVBoxLayout* progLayout = new QVBoxLayout();
		mProgressGroup->setLayout(progLayout);
		mProgressGroup->setTitle(StdWStringToQString(dsSolver_Progress_Box_Title));

		leftPanelLayout->addWidget(mProgressGroup, 1);

		QGroupBox* miscSettings = new QGroupBox();
		QVBoxLayout* miscLayout = new QVBoxLayout();
		miscSettings->setLayout(miscLayout);
		miscSettings->setTitle(StdWStringToQString(L"Miscellanous settings"));

		mDrawAtShutdownCheckBox = new QCheckBox(tr("Draw on shut-down only"));
		miscLayout->addWidget(mDrawAtShutdownCheckBox);

		leftPanelLayout->addWidget(miscSettings, 0);
	}

	layout->addWidget(leftPanel, 1, 0);

	QWidget* segmentsParentBox = new QWidget();
	{
		QVBoxLayout *segLayout = new QVBoxLayout();
		segmentsParentBox->setLayout(segLayout);

		QVBoxLayout* grlayout;

		QGroupBox* segmentGrpBox = new QGroupBox();
		segmentGrpBox->setTitle(tr(dsTime_Segments_Panel_Title));
		grlayout = new QVBoxLayout();
		segmentGrpBox->setLayout(grlayout);
		{
			QWidget* btnContainer = new QWidget();
			QHBoxLayout* hlayout = new QHBoxLayout();
			btnContainer->setLayout(hlayout);
			{
				QPushButton* btnAll = new QPushButton(dsSelect_All_Segments);
				hlayout->addWidget(btnAll);
				connect(btnAll, SIGNAL(clicked()), this, SLOT(On_Select_Segments_All()));

				QPushButton* btnNone = new QPushButton(dsSelect_No_Segments);
				hlayout->addWidget(btnNone);
				connect(btnNone, SIGNAL(clicked()), this, SLOT(On_Select_Segments_None()));
			}
			grlayout->addWidget(btnContainer);

			QScrollArea* scrollArea = new QScrollArea;
			scrollArea->setWidgetResizable(true);
			scrollArea->setFrameShape(QFrame::NoFrame);
			{
				mSegmentsGroup = new QWidget();

				QVBoxLayout* gr2layout = new QVBoxLayout();
				gr2layout->addStretch();
				mSegmentsGroup->setLayout(gr2layout);

				scrollArea->setWidget(mSegmentsGroup);
			}
			grlayout->addWidget(scrollArea);
		}
		segLayout->addWidget(segmentGrpBox);

		mSignalsGroup = new QGroupBox();
		mSignalsGroup->setTitle(tr(dsSignals_Panel_Title));
		{
			QVBoxLayout* grlayout = new QVBoxLayout();
			grlayout->addStretch();
			mSignalsGroup->setLayout(grlayout);

			segLayout->addWidget(mSignalsGroup);
		}

		QPushButton* redrawBtn = new QPushButton(tr(dsRedraw_Button_Title));
		segLayout->addWidget(redrawBtn);
		connect(redrawBtn, SIGNAL(clicked()), this, SLOT(On_Segments_Draw_Request()));
	}

	layout->addWidget(segmentsParentBox, 1, 9);

	// main tab widget, span to 11 columns for now
	mTabWidget = new QTabWidget();

	if constexpr (Is_Visibility_Panel_Enabled) {
		layout->addWidget(mTabWidget, 1, 1, 1, 8);
	}
	else {
		segmentsParentBox->hide();
		layout->addWidget(mTabWidget, 1, 1, 1, 10);
	}

	// TODO: verify if GUI filter is actually present in filter chain
	{
		// drawing tab

		CDrawing_Tab_Widget* tab;

		tab = new CDrawing_Tab_Widget(scgms::TDrawing_Image_Type::Graph);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_Graph));

		tab->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

		tab = new CDrawing_Tab_Widget(scgms::TDrawing_Image_Type::Day);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_Day));

		tab = new CDrawing_Tab_Widget(scgms::TDrawing_Image_Type::Clark);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_Clark));

		tab = new CDrawing_Tab_Widget(scgms::TDrawing_Image_Type::Parkes);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_Parkes));

		tab = new CDrawing_Tab_Widget(scgms::TDrawing_Image_Type::AGP);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_AGP));

		tab = new CDrawing_Tab_Widget(scgms::TDrawing_Image_Type::ECDF);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_ECDF));

		// log tab

		mLogWidget = new CLog_Tab_Widget(this);
		mTabWidget->addTab(mLogWidget, tr(dsLog_Tab));

		// errors tab

		mErrorsWidget = new CErrors_Tab_Widget(this);
		mTabWidget->addTab(mErrorsWidget, tr(dsErrors_Tab));

		// profile drawing tabs

		tab = new CDrawing_Tab_Widget(scgms::TDrawing_Image_Type::Profile_Glucose);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_Profile_Glucose));

		tab = new CDrawing_Tab_Widget(scgms::TDrawing_Image_Type::Profile_Carbs);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_Profile_Carbs));

		tab = new CDrawing_Tab_Widget(scgms::TDrawing_Image_Type::Profile_Insulin);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_Profile_Insulin));

		mBase_Tab_Count = mTabWidget->count();
	}

	QWidget *content = new QWidget{ this };
	content->setLayout(layout);

	setWidget(content);
	
	// set the window to be freed upon closing
	setAttribute(Qt::WA_DeleteOnClose, true);

	connect(mStartButton, SIGNAL(clicked()), this, SLOT(On_Start()));
	connect(mStopButton, SIGNAL(clicked()), this, SLOT(On_Stop()));
	connect(mSolveAndResetParamsButton, SIGNAL(clicked()), this, SLOT(On_Reset_And_Solve_Params()));
	connect(mTabWidget, SIGNAL(currentChanged(int)), this, SLOT(On_Tab_Change(int)));
	connect(mDrawAtShutdownCheckBox, SIGNAL(stateChanged(int)), this, SLOT(On_Draw_Shut_Down_State_Change(int)));

	mTabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(mTabWidget->tabBar(), SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(Show_Tab_Context_Menu(const QPoint &)));

	// GUI asynchronous updaters
	connect(this, SIGNAL(On_Start_Time_Segment(quint64)), this, SLOT(Slot_Start_Time_Segment(quint64)), Qt::QueuedConnection);
	connect(this, SIGNAL(On_Add_Signal(QUuid)), this, SLOT(Slot_Add_Signal(QUuid)), Qt::QueuedConnection);
	connect(this, SIGNAL(On_Update_Solver_Progress(QUuid)), this, SLOT(Slot_Update_Solver_Progress(QUuid)), Qt::QueuedConnection);
	connect(this, SIGNAL(On_Shut_Down_Received()), this, SLOT(On_Stop()));
}

void CSimulation_Window::Show_Tab_Context_Menu(const QPoint &point)
{
	if (point.isNull()) {
		return;
	}

	// "close" is displayed after the dynamically added widgets
	size_t sumDrawingV2 = 0;
	for (const auto& v : mDrawing_v2_Widgets) {
		sumDrawingV2 += v.size();
	}

	int tabIndex = mTabWidget->tabBar()->tabAt(point);
	QMenu menu(this);
	if (tabIndex >= mBase_Tab_Count + sumDrawingV2) {
		menu.addAction(tr(dsClose_Tab), std::bind(&CSimulation_Window::Close_Tab, this, tabIndex));
	}
	else {
		menu.addAction(tr(dsSave_Tab_State), std::bind(&CSimulation_Window::Save_Tab_State, this, tabIndex));
	}

	menu.exec(mTabWidget->tabBar()->mapToGlobal(point));
}

void CSimulation_Window::Close_Tab(int index) {
	CAbstract_Simulation_Tab_Widget* widget = dynamic_cast<CAbstract_Simulation_Tab_Widget*>(mTabWidget->widget(index));
	if (!widget) {
		return;
	}

	mTabWidget->removeTab(index);
}

void CSimulation_Window::Save_Tab_State(int index) {
	CAbstract_Simulation_Tab_Widget* widget = dynamic_cast<CAbstract_Simulation_Tab_Widget*>(mTabWidget->widget(index));
	if (!widget) {
		return;
	}

	CAbstract_Simulation_Tab_Widget* cloned = widget->Clone();
	if (!cloned) {
		return;
	}

	mTabWidget->addTab(cloned, mTabWidget->tabBar()->tabText(index) + dsSaved_State_Tab_Suffix);
}

void CSimulation_Window::Update_Tab_View() {
	if (mTabWidget) {
		auto tab = dynamic_cast<CAbstract_Simulation_Tab_Widget*>(mTabWidget->currentWidget());
		if (tab) {
			tab->Update_View_Size();
		}
	}
}

void CSimulation_Window::On_Tab_Change(int index) {
	Update_Tab_View();
}

void CSimulation_Window::On_Draw_Shut_Down_State_Change(int state) {
	if (state == Qt::Unchecked) {
		mGUI_Filter_Subchain.Set_Redraw_Mode(NRedraw_Mode::Periodic);
	}
	else {
		mGUI_Filter_Subchain.Set_Redraw_Mode(NRedraw_Mode::Shut_Down_Only);
	}
}

void CSimulation_Window::resizeEvent(QResizeEvent* evt) {
	QMdiSubWindow::resizeEvent(evt);

	Update_Tab_View();
}

void CSimulation_Window::On_Start() {
	On_Stop();

	mErrorsWidget->Clear_Filters(true);

	// clean progress bars and progress bar group box
	QLayoutItem *wItem;
	while ((wItem = mProgressGroup->layout()->takeAt(0)) != nullptr) {
		delete wItem->widget();
		delete wItem;
	}
	mProgressBars.clear();
	mBestMetricLabels.clear();

	QVBoxLayout* lay = dynamic_cast<QVBoxLayout*>(mProgressGroup->layout());
	if (lay) {
		lay->addStretch();
	}

	// clean segments
	while ((wItem = mSegmentsGroup->layout()->takeAt(0)) != nullptr) {
		delete wItem->widget();
		delete wItem;
	}
	mSegmentWidgets.clear();

	lay = dynamic_cast<QVBoxLayout*>(mSegmentsGroup->layout());
	if (lay) {
		lay->addStretch();
	}

	// clean signals
	while ((wItem = mSignalsGroup->layout()->takeAt(0)) != nullptr) {
		delete wItem->widget();
		delete wItem;
	}
	mSignalWidgets.clear();

	lay = dynamic_cast<QVBoxLayout*>(mSignalsGroup->layout());
	if (lay) {
		lay->addStretch();
	}

	mTerminal_Filter = std::make_unique<CGUI_Terminal_Filter>();

	// initialize and start filter holder, this will start filters
	refcnt::Swstr_list error_description;
	mFilter_Executor = scgms::SFilter_Executor{ mConfiguration, CSimulation_Window::On_Filter_Configured, this, error_description, mTerminal_Filter.get() };
	mLogWidget->Log_Config_Errors(error_description);
	if (!mFilter_Executor)	{
		mGUI_Filter_Subchain.Relase_Filter_Bindings();
		mErrorsWidget->Clear_Filters(true);
		mSolver_Filters.clear();
		mTerminal_Filter.reset();
		QMessageBox::information(this, tr(dsInformation), tr(dsFilter_Executor_Failed_Review_Config_Errors));

		return;
	}

	mSimulationInProgress = true;
	mStopButton->setEnabled(true);
	mStartButton->setEnabled(false);

	// hide all signal solve actions
	for (auto& action : mSignalSolveActions) {
		action.second->setVisible(false);
	}

	if (mFilter_Executor) {
		mGUI_Filter_Subchain.Start();

		// store old index of selected tab
		int curIdx = mTabWidget->currentIndex();

		// remove all dynamically added widgets from drawing v2
		for (auto& i : mDrawing_v2_Widgets) {
			for (auto& j : i) {
				mTabWidget->removeTab(j.second);
				delete j.first;
			}
		}

		// clear the original vector
		mDrawing_v2_Widgets.clear();

		// counter - so we could add them directly after the base tab set (before "saved" tabs)
		int tabOffset = 0; // mBase_Tab_Count; let's insert it at the begining as the preferred views

		// create tabs/widgets
		auto drawings = mGUI_Filter_Subchain.Get_Drawing_v2_Drawings();
		mDrawing_v2_Widgets.resize(drawings.size());
		for (size_t i = 0; i < drawings.size(); i++) {
			mDrawing_v2_Widgets[i].resize(drawings[i].size());

			for (size_t j = 0; j < drawings[i].size(); j++) {
				auto tab = new CDrawing_v2_Tab_Widget(mTabWidget);

				mDrawing_v2_Widgets[i][j] = {
					tab,
					mTabWidget->insertTab(tabOffset++, tab, StdWStringToQString(drawings[i][j]))
				};
			}
		}

		// restore selected tab index
		if (curIdx < mTabWidget->count()) {
			mTabWidget->setCurrentIndex(curIdx);
		}
	}
}

HRESULT IfaceCalling CSimulation_Window::On_Filter_Configured(scgms::IFilter *filter, const void* data) {
	CSimulation_Window * local_instance = static_cast<CSimulation_Window *>(const_cast<void*>(data));

	Setup_Filter_DB_Access(filter, nullptr);
	local_instance->mGUI_Filter_Subchain.On_Filter_Configured(filter);
	local_instance->mErrorsWidget->On_Filter_Configured(filter);

	if (scgms::SCalculate_Filter_Inspection insp = scgms::SCalculate_Filter_Inspection{ scgms::SFilter{filter} }) {
		local_instance->mSolver_Filters.push_back(insp);
	}

	return S_OK;
}

void CSimulation_Window::On_Stop() {
	if (!mSimulationInProgress) {
		return;
	}

	mSimulationInProgress = false;
	mGUI_Filter_Subchain.Stop(true);

	mErrorsWidget->Clear_Filters(false);
	
	for (const auto& solvers : mSolver_Filters) {
		solvers->Cancel_Solver();
	}

	mSolver_Filters.clear();

	Inject_Event(scgms::NDevice_Event_Code::Shut_Down, Invalid_GUID, nullptr);

	if (Succeeded(mFilter_Executor->Terminate(TRUE))) {
		mStartButton->setEnabled(true);
		mStopButton->setEnabled(false);
	}

	mTerminal_Filter.reset();
}

void CSimulation_Window::On_Reset_And_Solve_Params() {
	Inject_Event(scgms::NDevice_Event_Code::Warm_Reset, Invalid_GUID, nullptr);
}

CSimulation_Window* CSimulation_Window::Get_Instance() {
	return mInstance;
}

void CSimulation_Window::Drawing_Callback(const scgms::TDrawing_Image_Type type, const scgms::TDiagnosis diagnosis, const std::string &image_data) {
	for (CDrawing_Tab_Widget* wg : mDrawingWidgets) {
		wg->Drawing_Callback(type, diagnosis, image_data);
	}
}

void CSimulation_Window::Drawing_v2_Callback(size_t filterIdx, size_t drawingIdx, const std::string& svg) {
	if (filterIdx >= mDrawing_v2_Widgets.size() || drawingIdx >= mDrawing_v2_Widgets[filterIdx].size()) {
		return;
	}

	mDrawing_v2_Widgets[filterIdx][drawingIdx].first->Drawing_Callback(svg);
}

void CSimulation_Window::Update_Preferred_Drawing_Dimensions(size_t filterIdx, size_t drawingIdx, int& width, int& height) {
	//does not work on non-visible elements:
	//mDrawing_v2_Widgets[filterIdx][drawingIdx].first->Get_Canvas_Dimensions(width, height);

	// TODO: proper way of obtaining width and height of viewport; for now, just tse 95 % of widget space
	width = mTabWidget->currentWidget()->width() * 0.95;
	height = mTabWidget->currentWidget()->height() * 0.95;
}

void CSimulation_Window::Log_Callback(std::shared_ptr<refcnt::wstr_list> messages) {
	refcnt::wstr_container **begin, **end;
	if (messages) {
		if (messages->get(&begin, &end) == S_OK) {
			for (auto iter = begin; iter != end; iter++) {
				mLogWidget->Log_Message(refcnt::WChar_Container_To_WString(*iter));
			}
		}
	}

}

void CSimulation_Window::Update_Solver_Progress(const GUID& solver, size_t progress, double bestMetric, scgms::TSolver_Status status) {
	// do not display disabled solver
	if (status == scgms::TSolver_Status::Disabled) {
		return;
	}

	mSolverProgress[solver] = { progress, bestMetric, status };
	emit On_Update_Solver_Progress(GUID_To_QUuid(solver));
}

void CSimulation_Window::Slot_Update_Solver_Progress(QUuid solver) {
	const GUID solver_id = QUuid_To_GUID(solver);

	auto itr = mProgressBars.find(solver_id);

	const auto& solverProgress = mSolverProgress[solver_id];

	const size_t progress = solverProgress.progress;
	const double bestMetric = solverProgress.bestMetric;
	const scgms::TSolver_Status status = solverProgress.status;

	QString metricString = tr(dsBest_Metric_Label);
	if (solverProgress.progress != Invalid_Value) {
		metricString = metricString.arg(bestMetric);
	}
	else {
		metricString = metricString.arg(dsBest_Metric_NotAvailable);
	}

	std::string statusStr;
	switch (status) {
		case scgms::TSolver_Status::Disabled:
			statusStr = dsSolver_Status_Disabled;
			break;
		case scgms::TSolver_Status::Idle:
			statusStr = dsSolver_Status_Idle;
			break;
		case scgms::TSolver_Status::In_Progress:
			statusStr = dsSolver_Status_In_Progress;
			break;
		case scgms::TSolver_Status::Completed_Improved:
			statusStr = dsSolver_Status_Completed_Improved;
			break;
		case scgms::TSolver_Status::Completed_Not_Improved:
			statusStr = dsSolver_Status_Completed_Not_Improved;
			break;
		case scgms::TSolver_Status::Failed:
			statusStr = dsSolver_Status_Failed;
			break;
	}

	if (itr == mProgressBars.end()) {
		QVBoxLayout* lay = dynamic_cast<QVBoxLayout*>(mProgressGroup->layout());
		if (lay) {
			QLabel* plabel = new QLabel{ StdWStringToQString(mSignal_Descriptors.Get_Name(solver_id)) };
			QLabel* metriclabel = new QLabel(metricString);
			mBestMetricLabels[solver_id] = metriclabel;
			QLabel* statusLabel = new QLabel(tr(statusStr.c_str()));
			mSolverStatusLabels[solver_id] = statusLabel;

			QProgressBar* pbar = new QProgressBar();
			if (pbar) {
				mProgressBars[solver_id] = pbar;

				pbar->setValue((int)progress);
				lay->insertWidget(0, metriclabel);
				lay->insertWidget(0, pbar);
				lay->insertWidget(0, statusLabel);
				lay->insertWidget(0, plabel);
			}
		}
	}
	else {
		itr->second->setValue((int)progress);

		mBestMetricLabels[solver_id]->setText(metricString);
		mSolverStatusLabels[solver_id]->setText(tr(statusStr.c_str()));
	}

	// In_Progress = progress bar visible, status hidden
	if (status != scgms::TSolver_Status::In_Progress) {
		mSolverStatusLabels[solver_id]->show();
		mProgressBars[solver_id]->hide();
	}
	else {
		mSolverStatusLabels[solver_id]->hide();
		mProgressBars[solver_id]->show();
	}
}

void CSimulation_Window::On_Segments_Draw_Request() {
	std::vector<uint64_t> segmentsToDraw;
	std::vector<GUID> signalsToDraw;
	std::vector<GUID> signalsReferenceIdsToDraw;

	for (const auto& ctrl : mSegmentWidgets) {
		if (ctrl.second->Is_Checked()) {
			segmentsToDraw.push_back(ctrl.second->Get_Segment_Id());
		}
	}

	for (const auto &ctrl : mSignalWidgets) {
		if (ctrl.second->Is_Checked()) {
			signalsToDraw.push_back(ctrl.second->Get_Signal_Id());
			signalsReferenceIdsToDraw.push_back(ctrl.second->Get_Reference_Signal_Id());
		}
	}
	
	mGUI_Filter_Subchain.Request_Redraw(segmentsToDraw, signalsToDraw, signalsReferenceIdsToDraw);
}

void CSimulation_Window::On_Select_Segments_All() {
	for (auto ctrl : mSegmentWidgets) {
		ctrl.second->Set_Checked(true);
	}
}

void CSimulation_Window::On_Select_Segments_None() {
	for (auto ctrl : mSegmentWidgets) {
		ctrl.second->Set_Checked(false);
	}
}

void CSimulation_Window::Start_Time_Segment(uint64_t segmentId) {
	emit On_Start_Time_Segment(segmentId);
}

void CSimulation_Window::Slot_Start_Time_Segment(quint64 id) {
	auto itr = mSegmentWidgets.find(id);

	if (itr == mSegmentWidgets.end()) {
		CTime_Segment_Group_Widget* grp = new CTime_Segment_Group_Widget(id);

		mSegmentWidgets[id] = grp;

		QVBoxLayout* lay = dynamic_cast<QVBoxLayout*>(mSegmentsGroup->layout());
		if (lay) {
			// append new segment after existing segments and before stretch
			lay->insertWidget(static_cast<int>(mSegmentWidgets.size()) - 1, grp);
		}
	}
}

void CSimulation_Window::Add_Signal(const GUID& signalId) {
	// possible narrowing conversion (fine, as QUuid internally matches GUID)
	emit On_Add_Signal(GUID_To_QUuid(signalId));
}

void CSimulation_Window::Slot_Add_Signal(QUuid id) {
	const GUID signal_id = QUuid_To_GUID(id);

	// do not add special signal markers
	if (signal_id == scgms::signal_All || signal_id == scgms::signal_Null) {
		return;
	}

	auto itr = mSignalWidgets.find(signal_id);

	if (itr == mSignalWidgets.end()) {
		CSignal_Group_Widget* grp = new CSignal_Group_Widget(signal_id);

		mSignalWidgets[signal_id] = grp;
		// show "solve" action
		if (mSignalSolveActions.find(signal_id) != mSignalSolveActions.end()) {
			mSignalSolveActions[signal_id]->setVisible(true);
		}

		QVBoxLayout* lay = dynamic_cast<QVBoxLayout*>(mSignalsGroup->layout());
		if (lay) {
			// append new signal after existing signals and before stretch
			lay->insertWidget(static_cast<int>(mSignalWidgets.size()) - 1, grp);
		}
	}
}

void CSimulation_Window::On_Solve_Signal(QString str) {
	bool ok;
	GUID signalId = WString_To_GUID(str.toStdWString(), ok);
	if (!ok) {
		return;
	}

	Inject_Event(scgms::NDevice_Event_Code::Solve_Parameters, signalId, nullptr, scgms::All_Segments_Id);
}

void CSimulation_Window::Update_Solver_Progress() {

	for (const auto& solvers : mSolver_Filters) {
		GUID guid;
		scgms::TSolver_Status status;
		if (solvers->Get_Solver_Information(&guid, &status) != S_OK) {
			continue;
		}

		solver::TSolver_Progress progress;
		if (solvers->Get_Solver_Progress(&progress) != S_OK) {
			continue;
		}

		size_t pct = Invalid_Value;
		if (progress.max_progress != 0) {
			pct = (progress.current_progress * 100) / progress.max_progress;
		}
		else if (progress.current_progress != 0) {
			pct = progress.current_progress;
		}

		Update_Solver_Progress(guid, pct, progress.best_metric[0], status);
	}
}

void CSimulation_Window::Inject_Event(const scgms::NDevice_Event_Code &code, const GUID &signal_id, const wchar_t *info, const uint64_t segment_id) {
	if (mFilter_Executor) {
		scgms::UDevice_Event evt{ code };
		evt.signal_id() = signal_id;
		evt.segment_id() = segment_id;
		evt.info.set(info);
		mFilter_Executor.Execute(std::move(evt));
	}
}

void CSimulation_Window::Update_Errors() {
	if (mErrorsWidget) {
		mErrorsWidget->Refresh();
	}
}

void CSimulation_Window::Stop_Simulation() {
	emit On_Shut_Down_Received();
}

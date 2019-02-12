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
 * distributed under this license terms is distributed on an "AS IS" BASIS, WITHOUT
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

#include "simulation_window.h"

#include "../../../common/lang/dstrings.h"
#include "../../../common/rtl/FilterLib.h"
#include "../../../common/rtl/UILib.h"
#include "../../../common/rtl/referencedImpl.h"
#include "../../../common/utils/QtUtils.h"
#include "../../../common/rtl/rattime.h"

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
#include "../filters/descriptor.h"
#include "../filters/gui_subchain.h"

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

CSimulation_Window::CSimulation_Window(CFilter_Chain &filter_chain, QWidget *owner) : QMdiSubWindow{ owner }, mTabWidget(nullptr)
{
	mFilter_Chain_Manager = std::make_unique<CFilter_Chain_Manager>(filter_chain);

	Setup_UI();

	mStopButton->setEnabled(false);
	mStartButton->setEnabled(true);
	mSimulationInProgress = false;
}

CSimulation_Window::~CSimulation_Window()
{
	for (const auto& solvers : mSolver_Filters)
		solvers->Cancel_Solver();
	mFilter_Chain_Manager->Terminate_Filters();

	mInstance = nullptr;
}

bool CSimulation_Window::Is_Simulation_In_Progress() const
{
	return mSimulationInProgress;
}

void CSimulation_Window::Update_Filter_Chain(CFilter_Chain& filter_chain)
{
	mFilter_Chain_Manager = std::make_unique<CFilter_Chain_Manager>(filter_chain);
}

void CSimulation_Window::Setup_Solve_Button_Menu()
{
	mSolveSignalMapper = new QSignalMapper(this);
	connect(mSolveSignalMapper, SIGNAL(mapped(QString)), this, SLOT(On_Solve_Signal(QString)));

	QMenu* menu = new QMenu(this);

	QAction* anyaction = menu->addAction(StdWStringToQString(mSignal_Names.Get_Name(glucose::signal_All).c_str()));
	connect(anyaction, SIGNAL(triggered()), mSolveSignalMapper, SLOT(map()));
	mSolveSignalMapper->setMapping(anyaction, StdWStringToQString(GUID_To_WString(glucose::signal_All)));

	menu->addSeparator();

	auto models = glucose::get_model_descriptors();
	for (const auto& model : models)
	{
		for (size_t i = 0; i < model.number_of_calculated_signals; i++)
		{
			auto& action = mSignalSolveActions[model.calculated_signal_ids[i]];

			action = menu->addAction(StdWStringToQString(mSignal_Names.Get_Name(model.calculated_signal_ids[i]).c_str()));
			action->setVisible(false);
			connect(action, SIGNAL(triggered()), mSolveSignalMapper, SLOT(map()));
			mSolveSignalMapper->setMapping(action, StdWStringToQString(GUID_To_WString(model.calculated_signal_ids[i])));
		}
	}

	mSolveParamsButton->setMenu(menu);
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

	Setup_Solve_Button_Menu();

	mProgressGroup = new QGroupBox();
	QVBoxLayout *progLayout = new QVBoxLayout();
	mProgressGroup->setLayout(progLayout);
	mProgressGroup->setTitle(StdWStringToQString(dsSolver_Progress_Box_Title));

	layout->addWidget(mProgressGroup, 1, 0);

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
	layout->addWidget(mTabWidget, 1, 1, 1, 8);

	// TODO: verify if GUI filter is actually present in filter chain
	{
		// drawing tab

		CDrawing_Tab_Widget* tab;

		tab = new CDrawing_Tab_Widget(glucose::TDrawing_Image_Type::Graph);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_Graph));

		tab->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

		tab = new CDrawing_Tab_Widget(glucose::TDrawing_Image_Type::Day);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_Day));

		tab = new CDrawing_Tab_Widget(glucose::TDrawing_Image_Type::Clark);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_Clark));

		tab = new CDrawing_Tab_Widget(glucose::TDrawing_Image_Type::Parkes);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_Parkes));

		tab = new CDrawing_Tab_Widget(glucose::TDrawing_Image_Type::AGP);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_AGP));

		tab = new CDrawing_Tab_Widget(glucose::TDrawing_Image_Type::ECDF);
		mDrawingWidgets.push_back(tab);
		mTabWidget->addTab(tab, tr(dsDrawing_Tab_ECDF));

		// log tab

		mLogWidget = new CLog_Tab_Widget(this);
		mTabWidget->addTab(mLogWidget, tr(dsLog_Tab));

		// errors tab

		mErrorsWidget = new CErrors_Tab_Widget(this);
		mTabWidget->addTab(mErrorsWidget, tr(dsErrors_Tab));

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

	mTabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(mTabWidget->tabBar(), SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(Show_Tab_Context_Menu(const QPoint &)));

	// GUI asynchronnous updaters
	connect(this, SIGNAL(On_Start_Time_Segment(quint64)), this, SLOT(Slot_Start_Time_Segment(quint64)), Qt::QueuedConnection);
	connect(this, SIGNAL(On_Add_Signal(QUuid)), this, SLOT(Slot_Add_Signal(QUuid)), Qt::QueuedConnection);
	connect(this, SIGNAL(On_Update_Solver_Progress(QUuid)), this, SLOT(Slot_Update_Solver_Progress(QUuid)), Qt::QueuedConnection);
	connect(this, SIGNAL(On_Simulation_Terminate()), this, SLOT(Slot_Simulation_Terminate()), Qt::QueuedConnection);
}

void CSimulation_Window::Show_Tab_Context_Menu(const QPoint &point)
{
	if (point.isNull())
		return;

	int tabIndex = mTabWidget->tabBar()->tabAt(point);
	QMenu menu(this);
	if (tabIndex >= mBase_Tab_Count)
		menu.addAction(tr(dsClose_Tab), std::bind(&CSimulation_Window::Close_Tab, this, tabIndex));
	else
		menu.addAction(tr(dsSave_Tab_State), std::bind(&CSimulation_Window::Save_Tab_State, this, tabIndex));

	menu.exec(mTabWidget->tabBar()->mapToGlobal(point));
}

void CSimulation_Window::Close_Tab(int index)
{
	CAbstract_Simulation_Tab_Widget* widget = dynamic_cast<CAbstract_Simulation_Tab_Widget*>(mTabWidget->widget(index));
	if (!widget)
		return;

	mTabWidget->removeTab(index);
}

void CSimulation_Window::Save_Tab_State(int index)
{
	CAbstract_Simulation_Tab_Widget* widget = dynamic_cast<CAbstract_Simulation_Tab_Widget*>(mTabWidget->widget(index));
	if (!widget)
		return;

	CAbstract_Simulation_Tab_Widget* cloned = widget->Clone();
	if (!cloned)
		return;

	mTabWidget->addTab(cloned, mTabWidget->tabBar()->tabText(index) + dsSaved_State_Tab_Suffix);
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
	mBestMetricLabels.clear();

	QVBoxLayout* lay = dynamic_cast<QVBoxLayout*>(mProgressGroup->layout());
	if (lay)
		lay->addStretch();

	// clean segments
	while ((wItem = mSegmentsGroup->layout()->takeAt(0)) != nullptr)
	{
		delete wItem->widget();
		delete wItem;
	}
	mSegmentWidgets.clear();

	lay = dynamic_cast<QVBoxLayout*>(mSegmentsGroup->layout());
	if (lay)
		lay->addStretch();

	// clean signals
	while ((wItem = mSignalsGroup->layout()->takeAt(0)) != nullptr)
	{
		delete wItem->widget();
		delete wItem;
	}
	mSignalWidgets.clear();

	lay = dynamic_cast<QVBoxLayout*>(mSignalsGroup->layout());
	if (lay)
		lay->addStretch();

	// initialize and start filter holder, this will start filters
	if (mFilter_Chain_Manager->Init_And_Start_Filters() != S_OK)
	{
		// TODO: error message
		mFilter_Chain_Manager->Terminate_Filters();
		return;
	}

	// retrieve GUI subchain shared ptr instance and solver filter instances
	m_guiSubchain.reset();
	mFilter_Chain_Manager->Traverse_Filters([this](glucose::SFilter filter) {
		if (!m_guiSubchain)
			m_guiSubchain = SGUI_Filter_Subchain{ filter };
		if (glucose::SCalculate_Filter_Inspection insp = glucose::SCalculate_Filter_Inspection{ filter })
			mSolver_Filters.push_back(insp);
		return true;
	});

	mSimulationInProgress = true;
	mStopButton->setEnabled(true);
	mStartButton->setEnabled(false);

	if (mErrorsWidget)
		mErrorsWidget->Reset();

	// hide all signal solve actions
	for (auto& action : mSignalSolveActions)
		action.second->setVisible(false);
}

void CSimulation_Window::On_Stop() {
	mStopButton->setEnabled(false);
	Inject_Event(glucose::NDevice_Event_Code::Shut_Down, glucose::signal_All, nullptr);

	for (const auto& solvers : mSolver_Filters)
		solvers->Cancel_Solver();
}

void CSimulation_Window::Stop_Simulation() {
	emit On_Simulation_Terminate();
}

void CSimulation_Window::Slot_Simulation_Terminate()
{
	mFilter_Chain_Manager->Terminate_Filters();
	mSimulationInProgress = false;
	mStopButton->setEnabled(false);
	mStartButton->setEnabled(true);

	m_guiSubchain.reset();
	mSolver_Filters.clear();
}

void CSimulation_Window::On_Reset_And_Solve_Params() {
	Inject_Event(glucose::NDevice_Event_Code::Warm_Reset, Invalid_GUID, nullptr);
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

void CSimulation_Window::Update_Solver_Progress(const GUID& solver, size_t progress, double bestMetric, glucose::TSolver_Status status)
{
	// do not display disabled solver
	if (status == glucose::TSolver_Status::Disabled)
		return;

	mSolverProgress[solver] = { progress, bestMetric, status };
	emit On_Update_Solver_Progress(GUID_To_QUuid(solver));
}

void CSimulation_Window::Slot_Update_Solver_Progress(QUuid solver)
{
	const GUID solver_id = QUuid_To_GUID(solver);

	auto itr = mProgressBars.find(solver_id);

	const size_t progress = mSolverProgress[solver_id].progress;
	const double bestMetric = mSolverProgress[solver_id].bestMetric;
	const glucose::TSolver_Status status = mSolverProgress[solver_id].status;

	std::string statusStr;
	switch (status)
	{
		case glucose::TSolver_Status::Disabled:					statusStr = dsSolver_Status_Disabled; break;
		case glucose::TSolver_Status::Idle:						statusStr = dsSolver_Status_Idle; break;
		case glucose::TSolver_Status::In_Progress:				statusStr = dsSolver_Status_In_Progress; break;
		case glucose::TSolver_Status::Completed_Improved:		statusStr = dsSolver_Status_Completed_Improved; break;
		case glucose::TSolver_Status::Completed_Not_Improved:   statusStr = dsSolver_Status_Completed_Not_Improved; break;
		case glucose::TSolver_Status::Failed:					statusStr = dsSolver_Status_Failed; break;
	}

	if (itr == mProgressBars.end())
	{
		QProgressBar* pbar = new QProgressBar();
		mProgressBars[solver_id] = pbar;

		QLabel* plabel = new QLabel(StdWStringToQString(mSignal_Names.Get_Name(solver_id)));
		QLabel* metriclabel = new QLabel(tr(dsBest_Metric_Label).arg(bestMetric));
		mBestMetricLabels[solver_id] = metriclabel;
		QLabel* statusLabel = new QLabel(tr(statusStr.c_str()));
		mSolverStatusLabels[solver_id] = statusLabel;

		pbar->setValue((int)progress);

		QVBoxLayout* lay = dynamic_cast<QVBoxLayout*>(mProgressGroup->layout());
		if (lay)
		{
			lay->insertWidget(0, metriclabel);
			lay->insertWidget(0, pbar);
			lay->insertWidget(0, statusLabel);
			lay->insertWidget(0, plabel);
		}
	}
	else
	{
		itr->second->setValue((int)progress);

		mBestMetricLabels[solver_id]->setText(tr(dsBest_Metric_Label).arg(bestMetric));
		mSolverStatusLabels[solver_id]->setText(tr(statusStr.c_str()));
	}

	// In_Progress = progress bar visible, status hidden
	if (status != glucose::TSolver_Status::In_Progress)
	{
		mSolverStatusLabels[solver_id]->show();
		mProgressBars[solver_id]->hide();
	}
	else
	{
		mSolverStatusLabels[solver_id]->hide();
		mProgressBars[solver_id]->show();
	}
}

void CSimulation_Window::On_Segments_Draw_Request()
{
	std::vector<uint64_t> segmentsToDraw;
	std::vector<GUID> signalsToDraw;

	for (auto ctrl : mSegmentWidgets)
	{
		if (ctrl.second->Is_Checked())
			segmentsToDraw.push_back(ctrl.second->Get_Segment_Id());
	}

	for (auto ctrl : mSignalWidgets)
	{
		if (ctrl.second->Is_Checked())
			signalsToDraw.push_back(ctrl.second->Get_Signal_Id());
	}

	if (m_guiSubchain)
		m_guiSubchain->Request_Redraw(segmentsToDraw, signalsToDraw);
}

void CSimulation_Window::On_Select_Segments_All()
{
	for (auto ctrl : mSegmentWidgets)
		ctrl.second->Set_Checked(true);
}

void CSimulation_Window::On_Select_Segments_None()
{
	for (auto ctrl : mSegmentWidgets)
		ctrl.second->Set_Checked(false);
}

void CSimulation_Window::Start_Time_Segment(uint64_t segmentId)
{
	emit On_Start_Time_Segment(segmentId);
}

void CSimulation_Window::Slot_Start_Time_Segment(quint64 id)
{
	auto itr = mSegmentWidgets.find(id);

	if (itr == mSegmentWidgets.end())
	{
		CTime_Segment_Group_Widget* grp = new CTime_Segment_Group_Widget(id);

		mSegmentWidgets[id] = grp;

		QVBoxLayout* lay = dynamic_cast<QVBoxLayout*>(mSegmentsGroup->layout());
		if (lay)
		{
			// append new segment after existing segments and before stretch
			lay->insertWidget(static_cast<int>(mSegmentWidgets.size()) - 1, grp);
		}
	}
}

void CSimulation_Window::Add_Signal(const GUID& signalId)
{
	// possible narrowing conversion (fine, as QUuid internally matches GUID)
	emit On_Add_Signal(GUID_To_QUuid(signalId));
}

void CSimulation_Window::Slot_Add_Signal(QUuid id)
{
	const GUID signal_id = QUuid_To_GUID(id);

	auto itr = mSignalWidgets.find(signal_id);

	if (itr == mSignalWidgets.end())
	{
		CSignal_Group_Widget* grp = new CSignal_Group_Widget(signal_id);

		mSignalWidgets[signal_id] = grp;
		// show "solve" action
		if (mSignalSolveActions.find(signal_id) != mSignalSolveActions.end())
			mSignalSolveActions[signal_id]->setVisible(true);

		QVBoxLayout* lay = dynamic_cast<QVBoxLayout*>(mSignalsGroup->layout());
		if (lay)
		{
			// append new signal after existing signals and before stretch
			lay->insertWidget(static_cast<int>(mSignalWidgets.size()) - 1, grp);
		}
	}
}

void CSimulation_Window::On_Solve_Signal(QString str)
{
	GUID signalId = WString_To_GUID(str.toStdWString());
	if (signalId == Invalid_GUID)
		return;

	Inject_Event(glucose::NDevice_Event_Code::Solve_Parameters, signalId, nullptr, glucose::All_Segments_Id);
}

void CSimulation_Window::Update_Error_Metrics(const GUID& signal_id, glucose::TError_Markers& container, glucose::NError_Type type)
{
	mErrorsWidget->Update_Error_Metrics(signal_id, container, type);
}

void CSimulation_Window::Update_Solver_Progress()
{
	for (const auto& solvers : mSolver_Filters)
	{
		GUID guid;
		glucose::TSolver_Status status;
		if (solvers->Get_Solver_Information(&guid, &status) != S_OK)
			continue;

		solver::TSolver_Progress progress;
		if (solvers->Get_Solver_Progress(&progress) != S_OK)
			continue;

		size_t pct = progress.max_progress != 0 ? (progress.current_progress * 100) / progress.max_progress : progress.current_progress;
		Update_Solver_Progress(guid, pct, progress.best_metric, status);
	}
}

void CSimulation_Window::Inject_Event(const glucose::NDevice_Event_Code &code, const GUID &signal_id, const wchar_t *info, const uint64_t segment_id) {
	glucose::UDevice_Event evt{ code };
	evt.signal_id = signal_id;
	evt.segment_id = segment_id;
	evt.info.set(info);
	mFilter_Chain_Manager->Send(evt);
}

QUuid CSimulation_Window::GUID_To_QUuid(const GUID& guid)
{
	return QUuid(guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2],
		guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}

GUID CSimulation_Window::QUuid_To_GUID(const QUuid& uuid)
{
	return { uuid.data1, uuid.data2, uuid.data3,
		uuid.data4[0], uuid.data4[1], uuid.data4[2], uuid.data4[3], uuid.data4[4], uuid.data4[5],
		uuid.data4[6], uuid.data4[7]
	};
}
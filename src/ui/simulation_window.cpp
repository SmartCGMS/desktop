/**
 * SmartCGMS - continuous glucose monitoring and controlling framework
 * https://diabetes.zcu.cz/
 *
 * Contact:
 * diabetes@mail.kiv.zcu.cz
 * Medical Informatics, Department of Computer Science and Engineering
 * Faculty of Applied Sciences, University of West Bohemia
 * Technicka 8
 * 314 06, Pilsen
 *
 * Licensing terms:
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * a) For non-profit, academic research, this software is available under the
 *    GPLv3 license. When publishing any related work, user of this software
 *    must:
 *    1) let us know about the publication,
 *    2) acknowledge this software and respective literature - see the
 *       https://diabetes.zcu.cz/about#publications,
 *    3) At least, the user of this software must cite the following paper:
 *       Parallel software architecture for the next generation of glucose
 *       monitoring, Proceedings of the 8th International Conference on Current
 *       and Future Trends of Information and Communication Technologies
 *       in Healthcare (ICTH 2018) November 5-8, 2018, Leuven, Belgium
 * b) For any other use, especially commercial use, you must contact us and
 *    obtain specific terms and conditions for the use of the software.
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
	On_Stop();
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

			//

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

	// retrieve GUI subchain shared ptr instance
	m_guiSubchain.reset();
	mFilter_Chain_Manager->Traverse_Filters([this](glucose::SFilter filter) {
		m_guiSubchain = SGUI_Filter_Subchain{ filter };
		if (m_guiSubchain)
			return false;
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
	mFilter_Chain_Manager->Terminate_Filters();
	mSimulationInProgress = false;
	mStopButton->setEnabled(false);
	mStartButton->setEnabled(true);

	m_guiSubchain.reset();
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

			QLabel* plabel = new QLabel(StdWStringToQString(mSignal_Names.Get_Name(solver)));

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
	QEventLoop loop;
	Q_UNUSED(loop);
	QTimer::singleShot(0, this, [this, segmentId]()
	{
		auto itr = mSegmentWidgets.find(segmentId);

		if (itr == mSegmentWidgets.end())
		{
			CTime_Segment_Group_Widget* grp = new CTime_Segment_Group_Widget(segmentId);

			mSegmentWidgets[segmentId] = grp;

			QVBoxLayout* lay = dynamic_cast<QVBoxLayout*>(mSegmentsGroup->layout());
			if (lay)
			{
				// append new segment after existing segments and before stretch
				lay->insertWidget(static_cast<int>(mSegmentWidgets.size()) - 1, grp);
			}
		}
	});
}

void CSimulation_Window::Add_Signal(const GUID& signalId)
{
	QEventLoop loop;
	Q_UNUSED(loop);
	QTimer::singleShot(0, this, [this, signalId]()
	{
		auto itr = mSignalWidgets.find(signalId);

		if (itr == mSignalWidgets.end())
		{
			CSignal_Group_Widget* grp = new CSignal_Group_Widget(signalId);

			mSignalWidgets[signalId] = grp;
			// show "solve" action
			if (mSignalSolveActions.find(signalId) != mSignalSolveActions.end())
				mSignalSolveActions[signalId]->setVisible(true);

			QVBoxLayout* lay = dynamic_cast<QVBoxLayout*>(mSignalsGroup->layout());
			if (lay)
			{
				// append new signal after existing signals and before stretch
				lay->insertWidget(static_cast<int>(mSignalWidgets.size()) - 1, grp);
			}
		}
	});
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

void CSimulation_Window::Inject_Event(const glucose::NDevice_Event_Code &code, const GUID &signal_id, const wchar_t *info, const uint64_t segment_id) {
	glucose::UDevice_Event evt{ code };
	evt.signal_id = signal_id;
	evt.segment_id = segment_id;
	evt.info.set(info);
	mFilter_Chain_Manager->Send(evt);
}

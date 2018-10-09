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

#include "gui_subchain.h"
#include "descriptor.h"

#include "../../../common/lang/dstrings.h"
#include "../../../common/rtl/FilesystemLib.h"
#include "../../../common/rtl/rattime.h"
#include "../ui/simulation_window.h"

SGUI_Filter_Subchain::SGUI_Filter_Subchain(glucose::SFilter &gui_subchain_filter) {
	if (gui_subchain_filter) refcnt::Query_Interface<glucose::IFilter, CGUI_Filter_Subchain>(gui_subchain_filter.get(), gui::gui_filter_guid, *this);
}

CGUI_Filter_Subchain::CGUI_Filter_Subchain(glucose::SFilter_Pipe in_pipe, glucose::SFilter_Pipe out_pipe)
	: mInput(in_pipe), mOutput(out_pipe), mChange_Available(false) {
	//
}

HRESULT IfaceCalling CGUI_Filter_Subchain::QueryInterface(const GUID*  riid, void ** ppvObj) {
	if (Internal_Query_Interface<glucose::IFilter>(gui::gui_filter_guid, *riid, ppvObj)) return S_OK;

	return E_NOINTERFACE;
}

void CGUI_Filter_Subchain::Run_Input() {

	mChange_Available = false;

	for (; glucose::UDevice_Event evt = mInput.Receive(); ) {
		// here we may perform some input filtering, but that's not typical for filter input
		// most of actions will be done in output handler (Run_Output)

		// synchronnously update GUI on Shut_Down event; the filter chain is destroyed after that message, so to not miss
		// any updates, send marker, wait for it to come out of subchain output pipe, update GUI and then propagate Shut_Down
		if (evt.event_code == glucose::NDevice_Event_Code::Shut_Down) {

			mMarker_Received = false;
			Emit_Marker();

			// wait for marker to propagate through subchain; Run_Output will notify when it arrives
			{
				std::unique_lock<std::mutex> lck(mShut_Down_Mtx);
				mShut_Down_Cv.wait(lck, [this]() { return mMarker_Received; });
			}

			std::unique_lock<std::mutex> lck(mUpdater_Mtx);
			Update_GUI();
		}

		if (mSubchainMgr->Send(evt) != S_OK)
			break;

		mChange_Available = true;
	}

	// all pipes are aborted automatically, since the outer code propagates Shut_Down message through them
	// so, when this message is propagated, all filters will return from blocking Receive call with an error code,
	// and therefore end their threads

	mSubchainMgr->Join_Filters();
}

void CGUI_Filter_Subchain::Run_Updater()
{
	while (mRunning) {

		// since user may asynchronnously request update of any component, we need to lock the mutex before updating
		std::unique_lock<std::mutex> lck(mUpdater_Mtx);

		// update if there was a change
		if (mChange_Available.exchange(false)) {
			Update_GUI();
		}

		// TODO: configurable delay, maybe even during simulation?
		mUpdater_Cv.wait_for(lck, std::chrono::milliseconds(GUI_Subchain_Default_Drawing_Update));
	}
}

void CGUI_Filter_Subchain::Run_Output() {

	// simulation window lifetime is always longer than GUI filter subchain's and overlaps it entirely, so we could store simwin pointer here
	CSimulation_Window* const simwin = CSimulation_Window::Get_Instance();

	for (; glucose::UDevice_Event evt = mSubchainMgr->Receive();) {
		// here we handle all messages that comes from last GUI-wrapped filter pipe
		// this of course involves all messages that hasn't been dropped prior sending through last filter pipe

		if (evt.event_code == glucose::NDevice_Event_Code::Information)
		{
			// shutdown marker received - notify input thread so it could update GUI and propagate Shut_Down event
			if (refcnt::WChar_Container_Equals_WString(evt.info.get(), rsInfo_Shut_Down_Marker))
			{
				std::unique_lock<std::mutex> lck(mShut_Down_Mtx);
				mMarker_Received = true;
				mShut_Down_Cv.notify_all();
			}
		}
		else if (evt.event_code == glucose::NDevice_Event_Code::Parameters)
		{
			if (mCalculatedSignalGUIDs.find(evt.signal_id) == mCalculatedSignalGUIDs.end())
				mCalculatedSignalGUIDs.insert(evt.signal_id);
		}
		else if (evt.event_code == glucose::NDevice_Event_Code::Time_Segment_Start)
		{
			simwin->Start_Time_Segment(evt.segment_id);

			// enable drawing of this segment by default
			if (mDraw_Segment_Ids)
				mDraw_Segment_Ids->add(&evt.segment_id, &evt.segment_id + 1);
		}
		else if (evt.event_code == glucose::NDevice_Event_Code::Shut_Down)
		{
			simwin->Stop_Simulation();
		}

		// if the event is containing valid level, propagate it to GUI, if not already there
		if (evt.is_level_event() /*|| evt.is_parameters_event()*/)
		{
			if (m_presentSignals.find(evt.signal_id) == m_presentSignals.end())
			{
				m_presentSignals.insert(evt.signal_id);
				// enable drawing of this signal by default
				if (mDraw_Signal_Ids)
					mDraw_Signal_Ids->add(&evt.signal_id, &evt.signal_id + 1);

				simwin->Add_Signal(evt.signal_id);
			}
		}

		if (!mOutput.Send(evt))
			break;
	}
}

HRESULT CGUI_Filter_Subchain::Run(refcnt::IVector_Container<glucose::TFilter_Parameter>* const configuration) {
	
	// extract parameters - the parameter vector is a union of all parameters of all subchain filters, so we need to extract it
	glucose::TFilter_Parameter *param_begin, *param_end;
	if (configuration->get(&param_begin, &param_end) != S_OK)
		param_begin = param_end = nullptr;

	CFilter_Chain subchain;

	glucose::TFilter_Descriptor desc = glucose::Null_Filter_Descriptor;
	CFilter_Configuration filter_config;

	for (size_t i = 0; i < gui::gui_filters.size(); i++)
	{
		const GUID &id = gui::gui_filters[i];

		if (glucose::get_filter_descriptor_by_id(id, desc))
		{
			if (param_begin != nullptr)
			{
				// skip null parameters (config headers)
				while (param_begin != param_end && param_begin->type == glucose::NParameter_Type::ptNull)
					param_begin += 1;

				// iterate all the way to the last filter parameter, and copy it to chain link configuration
				const auto param_end = param_begin + desc.parameters_count;
				for (; param_begin != param_end; param_begin++)
					filter_config.push_back(*param_begin);
			}

			subchain.push_back({ desc, filter_config });
		}
	}

	// initialize and start subchain
	mSubchainMgr = std::make_unique<CFilter_Chain_Manager>(subchain);
	HRESULT rc = mSubchainMgr->Init_And_Start_Filters(false);
	if (rc != S_OK)
		return rc;

	// now we need to retrieve references to inspectionable filters, so we could provide drawings, log, etc.
	mSubchainMgr->Traverse_Filters([this](glucose::SFilter filter) -> bool {
		if (glucose::SDrawing_Filter_Inspection insp = glucose::SDrawing_Filter_Inspection{ filter })
			mDrawing_Filter_Inspection = insp;
		else if (glucose::SError_Filter_Inspection insp = glucose::SError_Filter_Inspection{ filter })
			mError_Filter_Inspection = insp;
		else if (glucose::SLog_Filter_Inspection insp = glucose::SLog_Filter_Inspection{ filter })
			mLog_Filter_Inspection = insp;

		return true;
	});

	// now we are ready; however, this filter still needs to manage input and output functionality, since it  is a filter itself
	// therefore we need to start output thread to listen to subchain outputs and pass them to output pipe
	// and also run input loop for receiving messages through input pipe and passing them to subchain as inputs

	mRunning = true;

	mUpdater_Thread = std::make_unique<std::thread>(&CGUI_Filter_Subchain::Run_Updater, this);
	mOutput_Thread = std::make_unique<std::thread>(&CGUI_Filter_Subchain::Run_Output, this);

	// this thread serves as input thread
	Run_Input();
	// join output thread; should terminate shortly after input part
	if (mOutput_Thread->joinable())
		mOutput_Thread->join();

	// terminate updater thread
	{
		std::unique_lock<std::mutex> lck(mUpdater_Mtx);

		mRunning = false;
		mUpdater_Cv.notify_all();
	}

	if (mUpdater_Thread->joinable())
		mUpdater_Thread->join();
	
	return S_OK;
}

void CGUI_Filter_Subchain::Request_Redraw(std::vector<uint64_t>& segmentIds, std::vector<GUID>& signalIds)
{
	std::unique_lock<std::mutex> lck(mUpdater_Mtx);

	// store requested containers and request redraw
	mDraw_Segment_Ids = refcnt::Create_Container_shared<uint64_t>(segmentIds.data(), segmentIds.data() + segmentIds.size());
	mDraw_Signal_Ids = refcnt::Create_Container_shared<GUID>(signalIds.data(), signalIds.data() + signalIds.size());

	Update_Drawing();
}

void CGUI_Filter_Subchain::Update_GUI()
{
	Update_Drawing();
	Update_Log();
	Update_Error_Metrics();
	Hint_Update_Solver_Progress();
}

void CGUI_Filter_Subchain::Update_Drawing()
{
	CSimulation_Window* const simwin = CSimulation_Window::Get_Instance();
	if (!simwin || !mDrawing_Filter_Inspection)
		return;

	auto svg = refcnt::Create_Container_shared<char>(nullptr, nullptr);

	for (size_t type = 0; type < (size_t)glucose::TDrawing_Image_Type::count; type++) {
		if (mDrawing_Filter_Inspection->Draw((glucose::TDrawing_Image_Type)type, glucose::TDiagnosis::NotSpecified, svg.get(), mDraw_Segment_Ids.get(), mDraw_Signal_Ids.get()) == S_OK) {
			simwin->Drawing_Callback((glucose::TDrawing_Image_Type)type, glucose::TDiagnosis::NotSpecified, refcnt::Char_Container_To_String(svg.get()));
		}
	}

	if (mDrawing_Filter_Inspection->Draw(glucose::TDrawing_Image_Type::Parkes, glucose::TDiagnosis::Type2, svg.get(), mDraw_Segment_Ids.get(), mDraw_Signal_Ids.get()) == S_OK) {
		simwin->Drawing_Callback(glucose::TDrawing_Image_Type::Parkes, glucose::TDiagnosis::Type2, refcnt::Char_Container_To_String(svg.get()));
	}
}

void CGUI_Filter_Subchain::Update_Log()
{
	CSimulation_Window* const simwin = CSimulation_Window::Get_Instance();

	if (!simwin || !mLog_Filter_Inspection)
		return;

	std::shared_ptr<refcnt::wstr_list> lines;
	while (mLog_Filter_Inspection.pop(lines)) {
		simwin->Log_Callback(lines);
	}
}

void CGUI_Filter_Subchain::Update_Error_Metrics()
{
	CSimulation_Window* const simwin = CSimulation_Window::Get_Instance();
	if (!simwin || !mError_Filter_Inspection)
		return;

	glucose::TError_Markers err;
	for (auto& signal_id : mCalculatedSignalGUIDs) {
		for (size_t i = 0; i < static_cast<size_t>(glucose::NError_Type::count); i++) {
			if (mError_Filter_Inspection->Get_Errors(&signal_id, static_cast<glucose::NError_Type>(i), &err) == S_OK)
				simwin->Update_Error_Metrics(signal_id, err, static_cast<glucose::NError_Type>(i));
		}
	}

	for (auto& signal_id : glucose::signal_Virtual) {
		for (size_t i = 0; i < static_cast<size_t>(glucose::NError_Type::count); i++) {
			if (mError_Filter_Inspection->Get_Errors(&signal_id, static_cast<glucose::NError_Type>(i), &err) == S_OK)
				simwin->Update_Error_Metrics(signal_id, err, static_cast<glucose::NError_Type>(i));
		}
	}
}

void CGUI_Filter_Subchain::Hint_Update_Solver_Progress()
{
	CSimulation_Window* const simwin = CSimulation_Window::Get_Instance();
	if (!simwin)
		return;

	simwin->Update_Solver_Progress();
}

void CGUI_Filter_Subchain::Emit_Marker()
{
	glucose::UDevice_Event evt{ glucose::NDevice_Event_Code::Information };

	evt.device_time = Unix_Time_To_Rat_Time(time(nullptr));
	evt.info.set(rsInfo_Shut_Down_Marker);

	mSubchainMgr->Send(evt);
}

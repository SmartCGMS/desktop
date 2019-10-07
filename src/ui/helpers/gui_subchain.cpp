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
 *       monitoring", Procedia Computer Science, Volume 141C, pp. 279-286, 2018
 */

#include "gui_subchain.h"

#include "../../../../common/lang/dstrings.h"
#include "../../../../common/rtl/FilesystemLib.h"
#include "../../../../common/rtl/rattime.h"
#include "../../ui/simulation_window.h"

CGUI_Filter_Subchain::CGUI_Filter_Subchain() : mChange_Available(false) {

	// take all model-calculated signals and put them into calculated signal guids set
	const auto models = glucose::get_model_descriptors();
	for (const auto& model : models) {
		for (size_t i = 0; i < model.number_of_calculated_signals; i++)
			mCalculatedSignalGUIDs.insert(model.calculated_signal_ids[i]);
	}

	mRunning = true;

	mUpdater_Thread = std::make_unique<std::thread>(&CGUI_Filter_Subchain::Run_Updater, this);
}


CGUI_Filter_Subchain::~CGUI_Filter_Subchain() {
	// terminate updater thread
	{
		std::unique_lock<std::mutex> lck(mUpdater_Mtx);

		mRunning = false;
		mUpdater_Cv.notify_all();
	}

	if (mUpdater_Thread->joinable())
		mUpdater_Thread->join();

}

void CGUI_Filter_Subchain::Run_Updater()
{
	while (mRunning) {

		// since user may asynchronously request update of any component, we need to lock the mutex before updating
		std::unique_lock<std::mutex> lck(mUpdater_Mtx);

		// update if there was a change
		if (mChange_Available.exchange(false)) {
			Update_GUI();
		}

		// TODO: configurable delay, maybe even during simulation?
		mUpdater_Cv.wait_for(lck, std::chrono::milliseconds(GUI_Subchain_Default_Drawing_Update));
	}
}


void CGUI_Filter_Subchain::On_Filter_Configured(glucose::IFilter *filter) {
	if (glucose::SDrawing_Filter_Inspection insp = glucose::SDrawing_Filter_Inspection{ glucose::SFilter{filter} })
		mDrawing_Filter_Inspection = insp;
	
	if (glucose::SError_Filter_Inspection insp = glucose::SError_Filter_Inspection{ glucose::SFilter{filter} })
		mError_Filter_Inspection = insp;
	
	if (glucose::SLog_Filter_Inspection insp = glucose::SLog_Filter_Inspection{ glucose::SFilter{filter} })
		mLog_Filter_Inspection = insp;
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
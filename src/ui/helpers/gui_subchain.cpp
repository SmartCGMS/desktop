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

#include "gui_subchain.h"

#include <scgms/lang/dstrings.h>
#include <scgms/rtl/FilesystemLib.h>
#include <scgms/rtl/rattime.h>
#include <scgms/utils/DebugHelper.h>

#include "../../ui/simulation_window.h"

CGUI_Filter_Subchain::CGUI_Filter_Subchain() : mChange_Available(false), mRunning(false) {

	// take all model-calculated signals and put them into calculated signal guids set
	const auto models = scgms::get_model_descriptor_list();
	for (const auto& model : models) {
		for (size_t i = 0; i < model.number_of_calculated_signals; i++)
			mCalculatedSignalGUIDs.insert(model.calculated_signal_ids[i]);
	}
}

CGUI_Filter_Subchain::~CGUI_Filter_Subchain() {
	Stop();
}

void CGUI_Filter_Subchain::Start() {
	if (mRunning) {
		Stop();
	}

	mRunning = true;

	mUpdater_Thread = std::make_unique<std::thread>(&CGUI_Filter_Subchain::Run_Updater, this);
}

void CGUI_Filter_Subchain::Stop(bool update_gui) {
	if (!mRunning) {
		return;
	}

	// terminate updater thread
	{
		std::unique_lock<std::mutex> lck(mUpdater_Mtx);

		mRunning = false;
		mUpdateOnStop = update_gui;
		mUpdater_Cv.notify_all();
	}

	if (mUpdater_Thread && mUpdater_Thread->joinable()) {
		mUpdater_Thread->join();
		mUpdater_Thread.reset();
	}

	Relase_Filter_Bindings();
}

void CGUI_Filter_Subchain::Relase_Filter_Bindings() {
	mDrawing_Filter_Inspection = scgms::SDrawing_Filter_Inspection{ };
	mDrawing_Filter_Inspection_v2.clear();
	mAvailable_Plot_Views.clear();
	mDrawing_Clock = 0;
	mLog_Filter_Inspection = scgms::SLog_Filter_Inspection{};
}

void CGUI_Filter_Subchain::Run_Updater() {
	while (mRunning) {

		// since user may asynchronously request update of any component, we need to lock the mutex before updating
		std::unique_lock<std::mutex> lck(mUpdater_Mtx);

		// update if there was a change
		if (mRedraw_Mode == NRedraw_Mode::Periodic) {
			Update_GUI();
		}

		// TODO: configurable delay, maybe even during simulation?
		mUpdater_Cv.wait_for(lck, std::chrono::milliseconds(GUI_Subchain_Default_Drawing_Update));

		if (!mRunning && mUpdateOnStop) {
			Update_GUI();
		}
	}
}

void CGUI_Filter_Subchain::On_Filter_Configured(scgms::IFilter *filter) {
	if (scgms::SDrawing_Filter_Inspection insp = scgms::SDrawing_Filter_Inspection{ scgms::SFilter{filter} }) {
		mDrawing_Filter_Inspection = insp;
	}

	if (scgms::SDrawing_Filter_Inspection_v2 insp = scgms::SDrawing_Filter_Inspection_v2{ scgms::SFilter{filter} }) {
		auto caps = refcnt::Create_Container_shared<scgms::TPlot_Descriptor>(nullptr, nullptr);
		if (insp->Get_Capabilities(caps.get()) == S_OK && caps->empty() != S_OK) {
			mAvailable_Plot_Views.emplace_back(caps.begin(), caps.end());
			mDrawing_Filter_Inspection_v2.push_back(insp);
		}
	}

	if (scgms::SLog_Filter_Inspection insp = scgms::SLog_Filter_Inspection{ scgms::SFilter{filter} }) {
		mLog_Filter_Inspection = insp;
	}
}

void CGUI_Filter_Subchain::Request_Redraw(std::vector<uint64_t>& segmentIds, std::vector<GUID>& signalIds, std::vector<GUID>& referenceSignalIds) {
	std::unique_lock<std::mutex> lck(mUpdater_Mtx);

	// store requested containers and request redraw
	mDraw_Segment_Ids = refcnt::Create_Container_shared<uint64_t>(segmentIds.data(), segmentIds.data() + segmentIds.size());
	mDraw_Signal_Ids = refcnt::Create_Container_shared<GUID>(signalIds.data(), signalIds.data() + signalIds.size());
	mDraw_Reference_Signal_Ids = refcnt::Create_Container_shared<GUID>(referenceSignalIds.data(), referenceSignalIds.data() + referenceSignalIds.size());

	mForceUpdate = true;

	Update_Drawing();
}

void CGUI_Filter_Subchain::Update_GUI() {
	Update_Drawing();
	Update_Log();
	Update_Error_Metrics();
	Hint_Update_Solver_Progress();
}

void CGUI_Filter_Subchain::Update_Drawing() {

	CSimulation_Window* const simwin = CSimulation_Window::Get_Instance();
	if (!simwin) {
		return;
	}

	if (mDrawing_Filter_Inspection && (mForceUpdate || mDrawing_Filter_Inspection->New_Data_Available() == S_OK)) {

		auto svg = refcnt::Create_Container_shared<char>(nullptr, nullptr);

		for (size_t type = 0; type < (size_t)scgms::TDrawing_Image_Type::count; type++) {
			if (mDrawing_Filter_Inspection->Draw((scgms::TDrawing_Image_Type)type, scgms::TDiagnosis::NotSpecified, svg.get(), mDraw_Segment_Ids.get(), mDraw_Signal_Ids.get()) == S_OK) {
				simwin->Drawing_Callback((scgms::TDrawing_Image_Type)type, scgms::TDiagnosis::NotSpecified, refcnt::Char_Container_To_String(svg.get()));
			}
		}

		if (mDrawing_Filter_Inspection->Draw(scgms::TDrawing_Image_Type::Parkes, scgms::TDiagnosis::Type2, svg.get(), mDraw_Segment_Ids.get(), mDraw_Signal_Ids.get()) == S_OK) {
			simwin->Drawing_Callback(scgms::TDrawing_Image_Type::Parkes, scgms::TDiagnosis::Type2, refcnt::Char_Container_To_String(svg.get()));
		}
	}

	if (!mDrawing_Filter_Inspection_v2.empty()) {

		uint64_t *seg_begin, *seg_end;
		if (!mDraw_Segment_Ids || mDraw_Segment_Ids->get(&seg_begin, &seg_end) != S_OK) {
			seg_begin = seg_end = nullptr;
		}

		GUID *sig_begin, *sig_end;
		if (!mDraw_Signal_Ids || mDraw_Signal_Ids->get(&sig_begin, &sig_end) != S_OK) {
			sig_begin = sig_end = nullptr;
		}

		GUID *ref_begin, *ref_end;
		if (!mDraw_Reference_Signal_Ids || mDraw_Reference_Signal_Ids->get(&ref_begin, &ref_end) != S_OK) {
			// either both valid, or both nullptr
			ref_begin = sig_begin;
			ref_end = sig_end;
		}

		assert(std::distance(ref_begin, ref_end) == std::distance(sig_begin, sig_end) && "Reference signal count must be equal to signal count!");

		scgms::TDraw_Options opts;
		opts.width = mDrawing_v2_Width;
		opts.height = mDrawing_v2_Height;
		opts.in_signals = sig_begin;
		opts.reference_signals = ref_begin;
		opts.signal_count = std::distance(sig_begin, sig_end);
		opts.segments = seg_begin;
		opts.segment_count = std::distance(seg_begin, seg_end);

		for (size_t i = 0; i < mDrawing_Filter_Inspection_v2.size(); i++) {
			auto& insp = mDrawing_Filter_Inspection_v2[i];

			if (!mForceUpdate && insp->Logical_Clock(&mDrawing_Clock) != S_OK) {
				continue;
			}

			for (size_t j = 0; j < mAvailable_Plot_Views[i].size(); j++) {
				simwin->Update_Preferred_Drawing_Dimensions(i, j, opts.width, opts.height);
				auto svg = refcnt::Create_Container_shared<char>(nullptr, nullptr);

				if (insp->Draw(&mAvailable_Plot_Views[i][j].id, svg.get(), &opts) == S_OK) {
					auto str = refcnt::Char_Container_To_String(svg.get());
					simwin->Drawing_v2_Callback(i, j, str);
				}
			}
		}
	}
}

void CGUI_Filter_Subchain::Update_Log() {
	CSimulation_Window* const simwin = CSimulation_Window::Get_Instance();

	if (!simwin || !mLog_Filter_Inspection) {
		return;
	}

	std::shared_ptr<refcnt::wstr_list> lines;
	while (mLog_Filter_Inspection.pop(lines)) {
		simwin->Log_Callback(lines);
	}
}

void CGUI_Filter_Subchain::Update_Error_Metrics() {
	CSimulation_Window* const simwin = CSimulation_Window::Get_Instance();
	if (!simwin) {
		return;
	}

	simwin->Update_Errors();
}

void CGUI_Filter_Subchain::Hint_Update_Solver_Progress() {
	CSimulation_Window* const simwin = CSimulation_Window::Get_Instance();
	if (!simwin) {
		return;
	}

	simwin->Update_Solver_Progress();
}

void CGUI_Filter_Subchain::Set_Preferred_Drawing_Dimensions(const int width, const int height) {
	mDrawing_v2_Width = width;
	mDrawing_v2_Height = height;
}

void CGUI_Filter_Subchain::Set_Redraw_Mode(NRedraw_Mode mode) {
	mRedraw_Mode = mode;
}

std::vector<std::vector<std::wstring>> CGUI_Filter_Subchain::Get_Drawing_v2_Drawings() const {
	std::vector<std::vector<std::wstring>> ret;

	ret.resize(mAvailable_Plot_Views.size());

	for (size_t i = 0; i < mAvailable_Plot_Views.size(); i++) {
		ret[i].resize(mAvailable_Plot_Views[i].size());
		
		for (size_t j = 0; j < mAvailable_Plot_Views[i].size(); j++) {
			ret[i][j] = mAvailable_Plot_Views[i][j].name;
		}
	}

	return ret;
}

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

#pragma once

#include <scgms/iface/UIIface.h>
#include <scgms/rtl/referencedImpl.h>
#include <scgms/rtl/FilterLib.h>
#include <scgms/rtl/SolverLib.h>

#include <memory>
#include <thread>
#include <set>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <set>

// default time in [ms] to update drawing
constexpr size_t GUI_Subchain_Default_Drawing_Update = 500;

enum class NRedraw_Mode {
	Periodic,		// default - refresh once every GUI_Subchain_Default_Drawing_Update ms
	Shut_Down_Only,	// only redraw on shut_down

	count
};

// this is exception from filter decomposition model: this filter is special, in this context it means, it "knows" about several filters
// typically used by GUI - drawing filter, error metrics filter, log filter

#pragma warning( push )
#pragma warning( disable : 4250 ) // C4250 - 'class1' : inherits 'class2::member' via dominance

/*
 * Filter class for managing user interface needs
 */
class CGUI_Filter_Subchain {
	protected:
		scgms::SDrawing_Filter_Inspection mDrawing_Filter_Inspection;
		std::vector<scgms::SDrawing_Filter_Inspection_v2> mDrawing_Filter_Inspection_v2;
		scgms::SLog_Filter_Inspection mLog_Filter_Inspection;

		ULONG mDrawing_Clock = 0;
		std::vector<std::vector<scgms::TPlot_Descriptor>> mAvailable_Plot_Views;

		int mDrawing_v2_Width = 800;
		int mDrawing_v2_Height = 600;

		// set of all GUIDs of calculated signals that came through pipe
		std::set<GUID> mCalculatedSignalGUIDs;

		// thread for managing output pipe
		std::unique_ptr<std::thread> mOutput_Thread;
		// thread of periodic updater
		std::unique_ptr<std::thread> mUpdater_Thread;
		// updater mutex
		std::mutex mUpdater_Mtx;		
		// condition variable of periodic updater
		std::condition_variable mUpdater_Cv;		
		// flag to know whether to resume the updating thread
		std::atomic<bool> mChange_Available;

		// set of present signals in chain
		std::set<GUID> m_presentSignals;

		// is the subchain still running?
		bool mRunning = false;
		// should the GUI be updated one last time after simulation end?
		bool mUpdateOnStop = false;
		// should we update GUI even though filters do not signalize anything new?
		bool mForceUpdate = false;
		// was marker received?
		bool mMarker_Received = false;

		//  thread function for managing periodic updates (drawing)
		void Run_Updater();

		std::shared_ptr<refcnt::IVector_Container<uint64_t>> mDraw_Segment_Ids;
		std::shared_ptr<refcnt::IVector_Container<GUID>> mDraw_Signal_Ids;
		std::shared_ptr<refcnt::IVector_Container<GUID>> mDraw_Reference_Signal_Ids;

		void Update_GUI();

		void Update_Drawing();
		void Update_Log();
		void Update_Error_Metrics();
		void Hint_Update_Solver_Progress();

		NRedraw_Mode mRedraw_Mode = NRedraw_Mode::Periodic;

	public:
		CGUI_Filter_Subchain();
		virtual ~CGUI_Filter_Subchain();

		void On_Filter_Configured(scgms::IFilter *filter);

		void Request_Redraw(std::vector<uint64_t>& segmentIds, std::vector<GUID>& signalIds, std::vector<GUID>& referenceSignalIds);

		void Start();
		void Stop(bool update_gui = false);
		void Relase_Filter_Bindings();

		void Set_Preferred_Drawing_Dimensions(const int width, const int height);
		void Set_Redraw_Mode(NRedraw_Mode mode);

		std::vector<std::vector<std::wstring>> Get_Drawing_v2_Drawings() const;
};

#pragma warning( pop )

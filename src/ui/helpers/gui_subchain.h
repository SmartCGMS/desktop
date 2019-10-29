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

#pragma once

#include "../../../../common/iface/UIIface.h"
#include "../../../../common/rtl/referencedImpl.h"
#include "../../../../common/rtl/FilterLib.h"
#include "../../../../common/rtl/SolverLib.h"

#include <memory>
#include <thread>
#include <set>
#include <mutex>
#include <condition_variable>
#include <set>

// default time in [ms] to update drawing
constexpr size_t GUI_Subchain_Default_Drawing_Update = 500;

// this is exception from filter decomposition model: this filter is special, in this context it means, it "knows" about several filters
// typically used by GUI - drawing filter, error metrics filter, log filter

#pragma warning( push )
#pragma warning( disable : 4250 ) // C4250 - 'class1' : inherits 'class2::member' via dominance

/*
 * Filter class for managing user interface needs
 */
class CGUI_Filter_Subchain {
	protected:
		glucose::SDrawing_Filter_Inspection mDrawing_Filter_Inspection;		
		glucose::SLog_Filter_Inspection mLog_Filter_Inspection;		

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
		// was marker received?
		bool mMarker_Received = false;

		//  thread function for managing periodic updates (drawing)
		void Run_Updater();

		std::shared_ptr<refcnt::IVector_Container<uint64_t>> mDraw_Segment_Ids;
		std::shared_ptr<refcnt::IVector_Container<GUID>> mDraw_Signal_Ids;

		void Update_GUI();

		void Update_Drawing();
		void Update_Log();
		void Update_Error_Metrics();
		void Hint_Update_Solver_Progress();

	public:
		CGUI_Filter_Subchain();
		virtual ~CGUI_Filter_Subchain();

		void On_Filter_Configured(glucose::IFilter *filter);
			

		void Request_Redraw(std::vector<uint64_t>& segmentIds, std::vector<GUID>& signalIds);		

		void Start();
		void Stop();
};


#pragma warning( pop )

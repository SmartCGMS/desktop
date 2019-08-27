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

#include "../../../common/iface/UIIface.h"
#include "../../../common/rtl/referencedImpl.h"
#include "../../../common/rtl/FilterLib.h"
#include "../../../common/rtl/SolverLib.h"
#include "../../../common/desktop-console/filter_chain_manager.h"

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
class CGUI_Filter_Subchain : public glucose::IAsynchronnous_Filter, public virtual refcnt::CReferenced
{
	protected:
		glucose::SFilter_Asynchronnous_Pipe mInput;
		glucose::SFilter_Asynchronnous_Pipe mOutput;

		glucose::SDrawing_Filter_Inspection mDrawing_Filter_Inspection;
		glucose::SError_Filter_Inspection mError_Filter_Inspection;
		glucose::SLog_Filter_Inspection mLog_Filter_Inspection;

		// set of all GUIDs of calculated signals that came through pipe
		std::set<GUID> mCalculatedSignalGUIDs;

		// thread for managing output pipe
		std::unique_ptr<std::thread> mOutput_Thread;
		// thread of periodic updater
		std::unique_ptr<std::thread> mUpdater_Thread;
		// updater mutex
		std::mutex mUpdater_Mtx;
		// shut down synchronization mutex
		std::mutex mShut_Down_Mtx;
		// condition variable of periodic updater
		std::condition_variable mUpdater_Cv;
		// condition variable of shutdown marker
		std::condition_variable mShut_Down_Cv;
		// flag to know whether to resume the updating thread
		std::atomic<bool> mChange_Available;

		// set of present signals in chain
		std::set<GUID> m_presentSignals;

		// is the subchain still running?
		bool mRunning = false;
		// was marker received?
		bool mMarker_Received = false;

		// main function for managing input pipe
		void Run_Input();
		// second thread function for managing output pipe
		void Run_Output();
		// third thread function for managing periodic updates (drawing)
		void Run_Updater();

		std::shared_ptr<refcnt::IVector_Container<uint64_t>> mDraw_Segment_Ids;
		std::shared_ptr<refcnt::IVector_Container<GUID>> mDraw_Signal_Ids;

		std::unique_ptr<CFilter_Chain_Manager> mSubchainMgr;

		void Update_GUI();

		void Update_Drawing();
		void Update_Log();
		void Update_Error_Metrics();
		void Hint_Update_Solver_Progress();

		void Emit_Marker();

	public:
		CGUI_Filter_Subchain(glucose::SFilter_Asynchronnous_Pipe in_pipe, glucose::SFilter_Asynchronnous_Pipe out_pipe);
		virtual ~CGUI_Filter_Subchain() = default;

		virtual HRESULT Run(refcnt::IVector_Container<glucose::TFilter_Parameter>* const configuration) override;
		virtual HRESULT IfaceCalling QueryInterface(const GUID*  riid, void ** ppvObj) override;

		void Request_Redraw(std::vector<uint64_t>& segmentIds, std::vector<GUID>& signalIds);
};

class SGUI_Filter_Subchain : public std::shared_ptr<CGUI_Filter_Subchain> {
	public:
		SGUI_Filter_Subchain() noexcept {};
		SGUI_Filter_Subchain(glucose::SFilter &gui_subchain_filter);
};

#pragma warning( pop )

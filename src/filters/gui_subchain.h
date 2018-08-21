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
constexpr size_t GUI_Subchain_Default_Drawing_Update = 1000;

// this is exception from filter decomposition model: this filter is special, in this context it means, it "knows" about several filters
// typically used by GUI - drawing filter, error metrics filter, log filter

#pragma warning( push )
#pragma warning( disable : 4250 ) // C4250 - 'class1' : inherits 'class2::member' via dominance

/*
 * Filter class for managing user interface needs
 */
class CGUI_Filter_Subchain : public glucose::IFilter, public virtual refcnt::CReferenced
{
	protected:
		glucose::SFilter_Pipe mInput;
		glucose::SFilter_Pipe mOutput;

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
		// condition variable of periodic updater
		std::condition_variable mUpdater_Cv;
		// flag to know whether to resume the updating thread
		std::atomic<bool> mChange_Available;

		// set of present signals in chain
		std::set<GUID> m_presentSignals;

		// is the subchain still running?
		bool mRunning = false;

		// main function for managing input pipe
		void Run_Input();
		// second thread function for managing output pipe
		void Run_Output();
		// third thread function for managing periodic updates (drawing)
		void Run_Updater();

		std::shared_ptr<refcnt::IVector_Container<uint64_t>> mDraw_Segment_Ids;
		std::shared_ptr<refcnt::IVector_Container<GUID>> mDraw_Signal_Ids;

		std::unique_ptr<CFilter_Chain_Manager> mSubchainMgr;

		void Update_Drawing();
		void Update_Log();
		void Update_Error_Metrics();

	public:
		CGUI_Filter_Subchain(glucose::SFilter_Pipe in_pipe, glucose::SFilter_Pipe out_pipe);
		virtual ~CGUI_Filter_Subchain() {};

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

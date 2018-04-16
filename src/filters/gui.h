#pragma once

#include "../../../common/iface/FilterIface.h"
#include "../../../common/iface/UIIface.h"
#include "../../../common/rtl/referencedImpl.h"
#include "../../../common/rtl/FilterLib.h"
#include "../../../common/rtl/guid.h"

#include <memory>
#include <thread>
#include <set>

// this is exception from filter decomposition model: this filter is special, in this context it means, it "knows" about several filters
// typically used by GUI - drawing filter, error metrics filter, log filter

#include "../../../core/drawing/src/drawing.h"
#include "../../../core/log/src/log.h"
#include "../../../core/errors/src/errors.h"

#pragma warning( push )
#pragma warning( disable : 4250 ) // C4250 - 'class1' : inherits 'class2::member' via dominance

/*
 * Filter class for managing user interface needs
 */
class CGUI_Filter : public glucose::IFilter, public virtual refcnt::CReferenced
{
	protected:
		// input pipe
		glucose::IFilter_Pipe* mInput;
		// output pipe
		glucose::IFilter_Pipe* mOutput;

		// vector of additional pipes initialized for internally managed filters
		std::vector<glucose::SFilter_Pipe> mFilter_Pipes;
		// vector of internally managed filters
		std::vector<glucose::SFilter> mFilters;
		// vector of filter threads
		std::vector<std::unique_ptr<std::thread>> mFilter_Threads;

		// instance of drawing filter
		std::shared_ptr<CDrawing_Filter> mDrawing_Filter;
		// instance of log filter
		std::shared_ptr<CLog_Filter> mLog_Filter;
		// instance of error metrics filter
		std::shared_ptr<CErrors_Filter> mErrors_Filter;

		// AGP image data getter
		TGet_SVG Get_SVG_AGP;
		// clarks error grid image data getter
		TGet_SVG Get_SVG_Clark;
		// day image data getter
		TGet_SVG Get_SVG_Day;
		// summary graph image data getter
		TGet_SVG Get_SVG_Graph;
		// parkes error grid image data getter
		TGet_SVG Get_SVG_Parkes;

		// set of all GUIDs of calculated signals that came through pipe
		std::set<GUID> mCalculatedSignalGUIDs;
		// get error metrics
		TGet_Errors Get_Errors;

		// thread for managing output pipe
		std::unique_ptr<std::thread> mOutput_Thread;

		// main function for managing input pipe
		void Run_Input();
		// second thread function for managing output pipe
		void Run_Output();

	public:
		CGUI_Filter(glucose::IFilter_Pipe* inpipe, glucose::IFilter_Pipe* outpipe);

		virtual HRESULT Run(const refcnt::IVector_Container<glucose::TFilter_Parameter> *configuration) override final;
};

#pragma warning( pop )

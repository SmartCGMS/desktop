#pragma once

#include "../../../common/iface/FilterIface.h"
#include "../../../common/iface/UIIface.h"
#include "../../../common/rtl/referencedImpl.h"
#include "../../../common/rtl/FilterLib.h"
#include "../../../common/rtl/guid.h"

#pragma warning( push )
#pragma warning( disable : 4250 ) // C4250 - 'class1' : inherits 'class2::member' via dominance

//TODO: remove this entire filter and let CFilter chain manager to inject events

class CInput_Filter : public glucose::IFilter, public virtual refcnt::CReferenced
{
	protected:
		// output pipe
		glucose::SFilter_Pipe mInput, mOutput;

		// sends event with given parameters through pipe
		void Send_Event(glucose::NDevice_Event_Code code, const GUID &signal_id, const wchar_t *info = nullptr);

	public:
		CInput_Filter(glucose::IFilter_Pipe* inpipe, glucose::IFilter_Pipe* outpipe);

		virtual HRESULT Run(const refcnt::IVector_Container<glucose::TFilter_Parameter> *configuration) override final;

		// input filter specific methods

		// forces parameters solve of given signal ID (may be null to solve all signals)
		void Send_Force_Solve_Parameters(const GUID &signal_id, bool reset = false);
		// suspends any further parameter solving of given signal ID (may be null to suspend all); does not interrupt currently running solver
		void Send_Suspend_Solve_Parameters(const GUID &signal_id);
		// resumes starting routine of solvers of given signal ID (may be null to resume all)
		void Send_Resume_Solve_Parameters(const GUID &signal_id);

		// sends simulation step through pipe
		void Send_Simulation_Step(size_t amount = 1);
};

#pragma warning( pop )

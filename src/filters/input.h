#pragma once

#include "../../../common/iface/FilterIface.h"
#include "../../../common/iface/UIIface.h"
#include "../../../common/rtl/referencedImpl.h"
#include "../../../common/rtl/FilterLib.h"
#include "../../../common/rtl/guid.h"

#pragma warning( push )
#pragma warning( disable : 4250 ) // C4250 - 'class1' : inherits 'class2::member' via dominance

class CInput_Filter : public glucose::IFilter, public virtual refcnt::CReferenced
{
	protected:
		// output pipe
		glucose::IFilter_Pipe* mOutput;

		// sends event with given parameters through pipe
		void Send_Event(glucose::NDevice_Event_Code code, GUID* signal_id, refcnt::wstr_container* info = nullptr);

	public:
		CInput_Filter(glucose::IFilter_Pipe* inpipe, glucose::IFilter_Pipe* outpipe);

		virtual HRESULT Run(const refcnt::IVector_Container<glucose::TFilter_Parameter> *configuration) override final;

		// input filter specific methods

		// forces parameters solve of given signal ID (may be null to solve all signals)
		void Send_Force_Solve_Parameters(GUID* signal_id, bool reset = false);
		// suspends any further parameter solving of given signal ID (may be null to suspend all); does not interrupt currently running solver
		void Send_Suspend_Solve_Parameters(GUID* signal_id);
		// resumes starting routine of solvers of given signal ID (may be null to resume all)
		void Send_Resume_Solve_Parameters(GUID* signal_id);

		// sens simulation step through pipe
		void Send_Simulation_Step(size_t amount = 1);
};

#pragma warning( pop )

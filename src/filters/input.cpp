#include "input.h"
#include "../../../common/lang/dstrings.h"
#include "../../../common/rtl/rattime.h"

CInput_Filter::CInput_Filter(glucose::IFilter_Pipe* inpipe, glucose::IFilter_Pipe* outpipe)
	: mInput(inpipe), mOutput(outpipe)
{
	//
}

void CInput_Filter::Send_Event(glucose::NDevice_Event_Code code, const GUID &signal_id, const wchar_t *info)
{
	glucose::TDevice_Event evt;

	evt.device_id = { 0 };
	evt.device_time = Unix_Time_To_Rat_Time(time(nullptr));
	//evt.logical_time = 0; // this is probably not right in the bigger scale, but since we don't use logical time at this time,
						  // let's solve this later and mark it as TODO
	evt.signal_id = signal_id;
	evt.event_code = code;
	evt.segment_id = 0; // TODO: support more segments
	if (info)
		evt.info = refcnt::WString_To_WChar_Container(info);

	mOutput->send(&evt);
}

void CInput_Filter::Send_Force_Solve_Parameters(const GUID &signal, bool reset)
{
	// TODO: support more segments; for now, just reset all

	if (reset)
		Send_Event(glucose::NDevice_Event_Code::Information, signal, rsParameters_Reset_Request);

	Send_Event(glucose::NDevice_Event_Code::Solve_Parameters, signal);
}

void CInput_Filter::Send_Suspend_Solve_Parameters(const GUID &signal_id)
{
	Send_Event(glucose::NDevice_Event_Code::Suspend_Parameter_Solving, signal_id);
}

void CInput_Filter::Send_Resume_Solve_Parameters(const GUID &signal_id)
{
	Send_Event(glucose::NDevice_Event_Code::Resume_Parameter_Solving, signal_id);
}

void CInput_Filter::Send_Simulation_Step(size_t amount)
{
	// TODO: find a better solution than sending amount in GUID
	GUID amt{0};
	amt.Data1 = (unsigned long)amount; 

	Send_Event(glucose::NDevice_Event_Code::Simulation_Step, amt);
}

HRESULT CInput_Filter::Run(const refcnt::IVector_Container<glucose::TFilter_Parameter> *configuration)
{
	/* this filter does not even have its own thread; the thread started from outside code is immediatelly destroyed
	 * and this filter just passes input signals through output pipe to the rest of the chain */

	glucose::TDevice_Event evt;
	while (mInput->receive(&evt) == S_OK)
		mOutput->send(&evt);

	return S_OK;
}

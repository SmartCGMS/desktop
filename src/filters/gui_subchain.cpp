#pragma once

#define GUI_IMPORT

#include "gui_subchain.h"
#include "descriptor.h"

#include "../../../common/lang/dstrings.h"
#include "../../../common/rtl/FilesystemLib.h"
#include "../ui/simulation_window.h"

#include <iostream>

CGUI_Filter_Subchain::CGUI_Filter_Subchain(glucose::SFilter_Pipe in_pipe, glucose::SFilter_Pipe out_pipe)
	: mInput(in_pipe), mOutput(out_pipe) {
	//
}

void CGUI_Filter_Subchain::Run_Input() {

	for (; glucose::UDevice_Event evt = mInput.Receive(); evt) {
		// here we may perform some input filtering, but that's not typical for filter input
		// most of actions will be done in output handler (Run_Output)

		if (!mFilter_Pipes[0].Send(evt))
			break;
	}

	// now we need to abort all pipes of filters inside chain
	// this will effectively abort output thread

	//for (auto& pipe : mFilter_Pipes)
		//pipe->abort();
	glucose::UDevice_Event shut_down_event{ glucose::NDevice_Event_Code::Shut_Down };
	mFilter_Pipes[0].Send(shut_down_event);



	for (auto& thread : mFilter_Threads)
	{
		if (thread->joinable())
			thread->join();
	}

	if (mOutput_Thread->joinable())
		mOutput_Thread->join();
}

void CGUI_Filter_Subchain::Run_Output() {

	for (; glucose::UDevice_Event evt = mFilter_Pipes[mFilter_Pipes.size() - 1].Receive(); evt) {
		// here we handle all messages that comes from last GUI-wrapped filter pipe
		// this of course involves all messages that hasn't been dropped prior sending through last filter pipe

		if (evt.event_code == glucose::NDevice_Event_Code::Information)
		{
			// handle redraw messages (drawing filter)
			if (evt.info == rsInfo_Redraw_Complete)
			{
				CSimulation_Window* simwin = CSimulation_Window::Get_Instance();
				if (simwin && mDrawing_Filter_Inspection)
				{
					auto svg = refcnt::Create_Container_shared<char>(nullptr, nullptr);

					for (size_t type = 0; type < (size_t)glucose::TDrawing_Image_Type::count; type++)
					{
						if (mDrawing_Filter_Inspection->Draw((glucose::TDrawing_Image_Type)type, glucose::TDiagnosis::NotSpecified, svg.get()) == S_OK)
							simwin->Drawing_Callback((glucose::TDrawing_Image_Type)type, glucose::TDiagnosis::NotSpecified, refcnt::Char_Container_To_String(svg.get()));
					}

					// special drawing - e.g. Parkes' error grid for type 2 diabetes

					if (mDrawing_Filter_Inspection->Draw(glucose::TDrawing_Image_Type::Parkes, glucose::TDiagnosis::Type2, svg.get()) == S_OK)
						simwin->Drawing_Callback(glucose::TDrawing_Image_Type::Parkes, glucose::TDiagnosis::Type2, refcnt::Char_Container_To_String(svg.get()));
				}
			}
			// handle solver progress message
			else if (evt.info == rsInfo_Solver_Progress)
			{
				size_t progress;

				auto progStr = refcnt::WChar_Container_To_WString(evt.info.get());

				try
				{
					progress = std::stoull(progStr.substr(wcslen(rsInfo_Solver_Progress) + 1));
				}
				catch (...)
				{
					progress = 0;
				}

				CSimulation_Window* simwin = CSimulation_Window::Get_Instance();
				if (simwin)
					simwin->Update_Solver_Progress(evt.signal_id, progress);
			}
			// handle "error metrics ready" message
			else if (evt.info == rsInfo_Error_Metrics_Ready)
			{
				CSimulation_Window* simwin = CSimulation_Window::Get_Instance();
				if (simwin && mError_Filter_Inspection)
				{
					glucose::TError_Markers err;
					for (auto& signal_id : mCalculatedSignalGUIDs)
					{
						for (size_t i = 0; i < static_cast<size_t>(glucose::NError_Type::count); i++) {
							if (mError_Filter_Inspection->Get_Errors(&signal_id, static_cast<glucose::NError_Type>(i), &err) == S_OK)
								simwin->Update_Error_Metrics(signal_id, err, static_cast<glucose::NError_Type>(i));
						}
					}
				}
			}
		}
		else if (evt.event_code == glucose::NDevice_Event_Code::Parameters)
		{
			if (mCalculatedSignalGUIDs.find(evt.signal_id) == mCalculatedSignalGUIDs.end())
				mCalculatedSignalGUIDs.insert(evt.signal_id);
		}

		// TODO: log filter output to log tab

		if (!mOutput.Send(evt))
			break;
	}
}

HRESULT CGUI_Filter_Subchain::Run(const refcnt::IVector_Container<glucose::TFilter_Parameter> *configuration) {
	
	//TODO: re-engineer not to duplicate filter chain manager's functionality - it does the same!

	// initialize pipes
	mFilter_Pipes.clear();
	for (size_t i = 0; i < gui::gui_filters.size() + 1; i++) {
		glucose::SFilter_Pipe pipe{};
		if (!pipe) return E_FAIL;
		mFilter_Pipes.push_back(std::move(pipe));
	}		

	glucose::TFilter_Descriptor desc{0};

	// fetch configuration; the configuration is merged to one vector
	glucose::TFilter_Parameter *param_begin, *param_end;
	if (configuration->get(&param_begin, &param_end) != S_OK)
		param_begin = param_end = nullptr;

	mFilters.clear();
	for (size_t i = 0; i < gui::gui_filters.size(); i++)	
	{
		const GUID &filter_id = gui::gui_filters[i];
		glucose::get_filter_descriptor_by_id(filter_id, desc);

		// try to create filter
		auto filter = glucose::create_filter(filter_id, mFilter_Pipes[i], mFilter_Pipes[i + 1]);
		if (!filter)
		{
	//		std::wcerr << "ERROR: could not create filter " << desc.description << std::endl;
			return ENODEV;
		}

		//we've got the filter, is it an inspectionable one?
		if (filter_id == glucose::Drawing_Filter) 
			mDrawing_Filter_Inspection = glucose::SDrawing_Filter_Inspection(filter);		
		else if (filter_id == glucose::Error_Filter)
			mError_Filter_Inspection = glucose::SError_Filter_Inspection(filter);

	
		std::shared_ptr<refcnt::IVector_Container<glucose::TFilter_Parameter>> params;
		if (param_begin != nullptr) {
			// skip null parameters (config headers)
			while (param_begin != param_end && param_begin->type == glucose::NParameter_Type::ptNull)
				param_begin += 1;

			// create parameter container by copying just part of parameters given
			params = refcnt::Create_Container_shared<glucose::TFilter_Parameter>(param_begin, param_begin + desc.parameters_count);
			// move beginning by param count, so the next filter gets its own parameters
			param_begin += desc.parameters_count;
		}

		// configure filter using loaded configuration and start the filter thread
		mFilter_Threads.push_back(std::make_unique<std::thread>([params, filter, desc, &filter_id]() {
			HRESULT hres = filter->Run(params.get());
			//if (hres != S_OK)
				//std::wcerr << "ERROR: could not configure filter " << desc.description << ", error: " << hres << std::endl;
		}));

		// add filter to vector
		mFilters.push_back(filter);
		//std::wcerr << "GUI #" << i << ": " << desc.description << std::endl;
	}

	// start filter sub-chain	

	mOutput_Thread = std::make_unique<std::thread>(&CGUI_Filter_Subchain::Run_Output, this);
	Run_Input();
	
	return S_OK;
}

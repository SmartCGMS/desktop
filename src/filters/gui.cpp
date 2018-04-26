#pragma once

#define GUI_IMPORT

#include "gui.h"
#include "descriptor.h"

#include "../../../common/lang/dstrings.h"
#include "../../../common/rtl/Dynamic_Library.h"
#include "../../../common/rtl/FilesystemLib.h"
#include "../ui/simulation_window.h"

#include <iostream>

template <typename T>
auto Resolve_Symbol(CDynamic_Library &library, const char* name)
{
	return reinterpret_cast<typename std::remove_reference<T>::type>(library.Resolve(name));
}


CGUI_Filter::CGUI_Filter(glucose::IFilter_Pipe* inpipe, glucose::IFilter_Pipe* outpipe)
	: mInput(inpipe), mOutput(outpipe)
{
	//
}

void CGUI_Filter::Run_Input()
{
	glucose::TDevice_Event evt;

	while (mInput->receive(&evt) == S_OK)
	{
		// here we may perform some input filtering, but that's not typical for filter input
		// most of actions will be done in output handler (Run_Output)

		if (mFilter_Pipes[0]->send(&evt) != S_OK)
			break;
	}

	// now we need to abort all pipes of filters inside chain
	// this will effectively abort output thread

	for (auto& pipe : mFilter_Pipes)
		pipe->abort();

	for (auto& thread : mFilter_Threads)
	{
		if (thread->joinable())
			thread->join();
	}

	if (mOutput_Thread->joinable())
		mOutput_Thread->join();
}

void CGUI_Filter::Run_Output()
{
	glucose::TDevice_Event evt;

	while (mFilter_Pipes[mFilter_Pipes.size() - 1]->receive(&evt) == S_OK)
	{
		// here we handle all messages that comes from last GUI-wrapped filter pipe
		// this of course involves all messages that hasn't been dropped prior sending through last filter pipe

		if (evt.event_code == glucose::NDevice_Event_Code::Information)
		{
			// handle redraw messages (drawing filter)
			if (refcnt::WChar_Container_Equals_WString(evt.info, rsInfo_Redraw_Complete))
			{
				CSimulation_Window* simwin = CSimulation_Window::Get_Instance();
				if (simwin)
				{
					auto starget = refcnt::Create_Container_shared<wchar_t>(nullptr, nullptr);

					Get_SVG_Graph(starget.get());
					simwin->Drawing_Callback(rsCallback_Drawing_Graph, refcnt::WChar_Container_To_WString(starget.get()).c_str());

					Get_SVG_Day(starget.get());
					simwin->Drawing_Callback(rsCallback_Drawing_Day, refcnt::WChar_Container_To_WString(starget.get()).c_str());

					Get_SVG_Clark(starget.get());
					simwin->Drawing_Callback(rsCallback_Drawing_Clark, refcnt::WChar_Container_To_WString(starget.get()).c_str());

					Get_SVG_Parkes(starget.get());
					simwin->Drawing_Callback(rsCallback_Drawing_Parkes, refcnt::WChar_Container_To_WString(starget.get()).c_str());

					Get_SVG_Parkes_Type2(starget.get());
					simwin->Drawing_Callback(rsCallback_Drawing_Parkes_Type2, refcnt::WChar_Container_To_WString(starget.get()).c_str());

					Get_SVG_AGP(starget.get());
					simwin->Drawing_Callback(rsCallback_Drawing_AGP, refcnt::WChar_Container_To_WString(starget.get()).c_str());
				}
			}
			// handle solver progress message
			else if (refcnt::WChar_Container_Equals_WString(evt.info, rsInfo_Solver_Progress, 0, wcslen(rsInfo_Solver_Progress)))
			{
				size_t progress;

				auto progStr = refcnt::WChar_Container_To_WString(evt.info);

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
			else if (refcnt::WChar_Container_Equals_WString(evt.info, rsInfo_Error_Metrics_Ready))
			{
				CSimulation_Window* simwin = CSimulation_Window::Get_Instance();
				if (simwin && Get_Errors)
				{
					glucose::TError_Container err;
					for (auto& signal_id : mCalculatedSignalGUIDs)
					{
						for (size_t i = 0; i < glucose::Error_Type_Count; i++)
						{
							Get_Errors(&signal_id, &err, (glucose::NError_Type)i);
							simwin->Update_Error_Metrics(signal_id, err, (glucose::NError_Type)i);
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

		if (mOutput->send(&evt) != S_OK)
			break;
	}
}

HRESULT CGUI_Filter::Run(const refcnt::IVector_Container<glucose::TFilter_Parameter> *configuration) {
	
	// initialize pipes
	mFilter_Pipes.clear();
	for (size_t i = 0; i < gui::gui_filters.size() + 1; i++)
		mFilter_Pipes.push_back(glucose::create_filter_pipe());

	glucose::TFilter_Descriptor desc{0};

	// fetch configuration; the configuration is merged to one vector
	glucose::TFilter_Parameter *param_begin, *param_end;
	configuration->get(&param_begin, &param_end);

	mFilters.clear();
	for (size_t i = 0; i < gui::gui_filters.size(); i++)	
	{
		auto filt = gui::gui_filters[i];
		glucose::get_filter_descriptors_by_id(filt, desc);

		// try to create filter
		auto filter = glucose::create_filter(filt, mFilter_Pipes[i], mFilter_Pipes[i + 1]);
		if (!filter)
		{
	//		std::wcerr << "ERROR: could not create filter " << desc.description << std::endl;
			return ENODEV;
		}

	
		// skip null parameters (config headers)
		while (param_begin != param_end && param_begin->type == glucose::NParameter_Type::ptNull)
			param_begin += 1;

		// create parameter container by copying just part of parameters given
		auto params = refcnt::Create_Container_shared<glucose::TFilter_Parameter>(param_begin, param_begin + desc.parameters_count);
		// move beginning by param count, so the next filter gets its own parameters
		param_begin += desc.parameters_count;

		// configure filter using loaded configuration and start the filter thread
		mFilter_Threads.push_back(std::make_unique<std::thread>([params, filter, desc, &filt]() {
			HRESULT hres = filter->Run(params.get());
			//if (hres != S_OK)
				//std::wcerr << "ERROR: could not configure filter " << desc.description << ", error: " << hres << std::endl;
		}));

		// add filter to vector
		mFilters.push_back(filter);
		//std::wcerr << "GUI #" << i << ": " << desc.description << std::endl;
	}

	// resolve functions from dynamic libraries

	const std::vector<std::pair<TGet_SVG&, const char*>> drawingResolvMap = {
		{ Get_SVG_AGP, rsFilter_Get_SVG_AGP },
		{ Get_SVG_Clark, rsFilter_Get_SVG_Clark },
		{ Get_SVG_Day, rsFilter_Get_SVG_Day },
		{ Get_SVG_Graph, rsFilter_Get_SVG_Graph },
		{ Get_SVG_Parkes, rsFilter_Get_SVG_Parkes },
		{ Get_SVG_Parkes_Type2, rsFilter_Get_SVG_Parkes_Type2 }
	};

	const std::vector<std::pair<TGet_Errors&, const char*>> errorsResolvMap = {
		{ Get_Errors, rsFilter_Get_Errors }
	};

	auto appdir = Get_Application_Dir();
	const auto allFiles = List_Directory(Path_Append(appdir, rsSolversDir));
	CDynamic_Library dlib;

	for (const auto& filepath : allFiles) {
		if (CDynamic_Library::Is_Library(filepath)) {

			dlib.Set_Filename(filepath);

			if (dlib.Load())
			{
				for (auto& pr : drawingResolvMap)
				{
					if (auto fnc = Resolve_Symbol<decltype(pr.first)>(dlib, pr.second))
						pr.first = fnc;
				}

				for (auto& pr : errorsResolvMap)
				{
					if (auto fnc = Resolve_Symbol<decltype(pr.first)>(dlib, pr.second))
						pr.first = fnc;
				}

				dlib.Unload();
			}
		}
	}

	// start filter sub-chain

	mOutput_Thread = std::make_unique<std::thread>(&CGUI_Filter::Run_Output, this);
	Run_Input();

	return S_OK;
}

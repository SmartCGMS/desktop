#include "filters_window.h"


#ifndef MOC_DIR
	#include "moc_filters_window.cpp"
#endif

std::atomic<CFilters_Window*> CFilters_Window::mInstance = nullptr;

CFilters_Window* CFilters_Window::Show_Instance(QWidget *owner) {

	CFilters_Window* tmp = nullptr;
	bool created = mInstance.compare_exchange_strong(tmp, new CFilters_Window(owner));

	if (created) {		
		mInstance.load()->showMaximized();		
	}

	return mInstance;
}

CFilters_Window::CFilters_Window(QWidget *owner)  {
}

CFilters_Window::~CFilters_Window() {
	mInstance = nullptr;
}
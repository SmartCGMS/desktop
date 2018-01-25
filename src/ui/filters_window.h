#pragma once

#include <atomic>

#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QListWidget>

#include "../executive/filter_chain.h" 


class CFilters_Window : public QMdiSubWindow {
	Q_OBJECT
protected:
	CFilter_Chain & mFilter_Chain;
private:
	static std::atomic<CFilters_Window*> mInstance;
protected:
	QListWidget *lbxApplied_Filters, *lbxAvailable_Filters;
	void Setup_UI();
protected slots:	
	void On_Add_Filter();
	void On_Configure_Filter();
	void On_Commit_Filters();
public:
	static CFilters_Window* Show_Instance(CFilter_Chain &filter_chain, QWidget *owner);
	CFilters_Window(CFilter_Chain &filter_chain, QWidget *owner);
	~CFilters_Window();
};
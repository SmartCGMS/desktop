#pragma once

#include <atomic>

#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QListWidget>

#include "../../../common/desktop-console/filter_chain.h"


class CFilters_Window : public QMdiSubWindow {
	Q_OBJECT
protected:
	CFilter_Chain & mFilter_Chain;
private:
	static std::atomic<CFilters_Window*> mInstance;
protected:
	QListWidget *lbxApplied_Filters, *lbxAvailable_Filters;
	void Setup_UI();
	void Configure_Filter(QListWidgetItem *item);
protected slots:
	void On_Add_Filter();
	void On_Move_Filter_Up();
	void On_Move_Filter_Down();
	void On_Remove_Filter();
	void On_Configure_Filter();
	void On_Commit_Filters();
	void On_Applied_Filter_Dbl_Click(QListWidgetItem* item);
	void On_Filter_Configure_Complete();
public:
	static CFilters_Window* Show_Instance(CFilter_Chain &filter_chain, QWidget *owner);
	CFilters_Window(CFilter_Chain &filter_chain, QWidget *owner);
	virtual ~CFilters_Window();
};
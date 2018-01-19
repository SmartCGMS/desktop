#pragma once

#include <atomic>

#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QListWidget>


class CFilters_Window : public QMdiSubWindow {
	Q_OBJECT
private:
	static std::atomic<CFilters_Window*> mInstance;
protected:
	void Setup_UI();
protected slots:
	QListWidget *lbxApplied_Filters;
	void On_Configure_Filter();
public:
	static CFilters_Window* Show_Instance(QWidget *owner);
	CFilters_Window(QWidget *owner);
	~CFilters_Window();
};
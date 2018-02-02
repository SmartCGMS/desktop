#pragma once

#include "general_container_edit.h"

#include <map>

#include <QtWidgets/QWidget>
#include <QtSql/QSqlDatabase>
#include <QtWidgets/QTableView>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQueryModel>

class CSelect_Time_Segment_Id_Panel : public QTableView, public virtual filter_config_window::CContainer_Edit {
		Q_OBJECT
protected:
	const QString mDb_Connection_Name = "CSelect_Time_Segment_Id_Panel_Connection";
	std::unique_ptr<QSqlDatabase> mDb;
	std::unique_ptr<QSqlQueryModel> mSegmentsModel;
	std::unique_ptr<QSqlQuery> mSegmentsQuery;	 
	const std::vector<glucose::TFilter_Parameter> &mConfiguration;	
public:
	CSelect_Time_Segment_Id_Panel(const std::vector<glucose::TFilter_Parameter> &configuration, QWidget *parent);
	virtual glucose::TFilter_Parameter get_parameter();
	virtual void set_parameter(const glucose::TFilter_Parameter &param) ;
	virtual void apply();	//e.g., on click the Apply button - non-mandatory function
};
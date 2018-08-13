#pragma once

#include "general_container_edit.h"

#include <map>

#include <QtWidgets/QWidget>
#include <QtSql/QSqlDatabase>
#include <QtWidgets/QTableView>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQueryModel>

#include <QtWidgets/QButtonGroup>

class CSelect_Subject_Panel : public QWidget, public virtual filter_config_window::CContainer_Edit
{
		Q_OBJECT

	protected slots:
		void On_Radio_Button_Selected();

	protected:
		const QString mDb_Connection_Name = "CSelect_Subject_Panel_Connection";
		std::unique_ptr<QSqlDatabase> mDb;
		std::unique_ptr<QSqlQueryModel> mSubjectsModel;
		std::unique_ptr<QSqlQuery> mSubjectsQuery;
		const std::vector<glucose::TFilter_Parameter> &mConfiguration;

		QButtonGroup* mButtonGroup;
		QTableView* mDbSubjects;

	public:
		CSelect_Subject_Panel(const std::vector<glucose::TFilter_Parameter> &configuration, QWidget *parent);

		virtual glucose::TFilter_Parameter get_parameter();
		virtual void set_parameter(const glucose::TFilter_Parameter &param) ;
		virtual void apply();
};

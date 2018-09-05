/**
 * SmartCGMS - continuous glucose monitoring and controlling framework
 * https://diabetes.zcu.cz/
 *
 * Contact:
 * diabetes@mail.kiv.zcu.cz
 * Medical Informatics, Department of Computer Science and Engineering
 * Faculty of Applied Sciences, University of West Bohemia
 * Technicka 8
 * 314 06, Pilsen
 *
 * Licensing terms:
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * a) For non-profit, academic research, this software is available under the
 *    GPLv3 license. When publishing any related work, user of this software
 *    must:
 *    1) let us know about the publication,
 *    2) acknowledge this software and respective literature - see the
 *       https://diabetes.zcu.cz/about#publications,
 *    3) At least, the user of this software must cite the following paper:
 *       Parallel software architecture for the next generation of glucose
 *       monitoring, Proceedings of the 8th International Conference on Current
 *       and Future Trends of Information and Communication Technologies
 *       in Healthcare (ICTH 2018) November 5-8, 2018, Leuven, Belgium
 * b) For any other use, especially commercial use, you must contact us and
 *    obtain specific terms and conditions for the use of the software.
 */

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

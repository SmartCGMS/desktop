/**
 * SmartCGMS - continuous glucose monitoring and controlling framework
 * https://diabetes.zcu.cz/
 *
 * Copyright (c) since 2018 University of West Bohemia.
 *
 * Contact:
 * diabetes@mail.kiv.zcu.cz
 * Medical Informatics, Department of Computer Science and Engineering
 * Faculty of Applied Sciences, University of West Bohemia
 * Univerzitni 8
 * 301 00, Pilsen
 * 
 * 
 * Purpose of this software:
 * This software is intended to demonstrate work of the diabetes.zcu.cz research
 * group to other scientists, to complement our published papers. It is strictly
 * prohibited to use this software for diagnosis or treatment of any medical condition,
 * without obtaining all required approvals from respective regulatory bodies.
 *
 * Especially, a diabetic patient is warned that unauthorized use of this software
 * may result into severe injure, including death.
 *
 *
 * Licensing terms:
 * Unless required by applicable law or agreed to in writing, software
 * distributed under these license terms is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * a) For non-profit, academic research, this software is available under the
 *      GPLv3 license.
 * b) For any other use, especially commercial use, you must contact us and
 *       obtain specific terms and conditions for the use of the software.
 * c) When publishing work with results obtained using this software, you agree to cite the following paper:
 *       Tomas Koutny and Martin Ubl, "Parallel software architecture for the next generation of glucose
 *       monitoring", Procedia Computer Science, Volume 141C, pp. 279-286, 2018
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
		glucose::SFilter_Configuration mConfiguration;

		QButtonGroup* mButtonGroup;
		QTableView* mDbSubjects;

	public:
		CSelect_Subject_Panel(glucose::SFilter_Configuration configuration, glucose::SFilter_Parameter &parameter, QWidget *parent);
		
		virtual void fetch_parameter() override;
		virtual void store_parameter() override;
};

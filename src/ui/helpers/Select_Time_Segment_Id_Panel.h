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
 *       monitoring", Proceedings of the 8th International Conference on Current
 *       and Future Trends of Information and Communication Technologies
 *       in Healthcare (ICTH 2018) November 5-8, 2018, Leuven, Belgium
 */

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
	virtual void set_parameter(const glucose::TFilter_Parameter &param);
	virtual void apply();	//e.g., on click the Apply button - non-mandatory function
};

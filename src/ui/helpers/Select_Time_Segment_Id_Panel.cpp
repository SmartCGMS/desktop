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

#include "Select_Time_Segment_Id_Panel.h"

#include "../../../../common/lang/dstrings.h"
#include "../../../../common/rtl/FilterLib.h"

#include "moc_Select_Time_Segment_Id_Panel.cpp"

CSelect_Time_Segment_Id_Panel::CSelect_Time_Segment_Id_Panel(scgms::SFilter_Configuration configuration, scgms::SFilter_Parameter parameter, QWidget * parent) : 
	mConfiguration(configuration), QTableView(parent), CContainer_Edit(parameter) {
	//
}


void CSelect_Time_Segment_Id_Panel::store_parameter() {
	std::vector<int64_t> segment_ids;

	if (mDb && mDb->isOpen()) {
		//prepare Qt iterators
		auto selindexes = selectionModel()->selectedIndexes();

		if (selindexes.size() != 0) {
		
			auto iterend = selindexes.end();
			auto iterbegin = selindexes.begin();
			auto selcol = iterbegin->column();

			//and get all selected time segment ids
			for (auto iter = iterbegin; iter < iterend; iter++) {
				if (iter->column() == selcol) {
					segment_ids.push_back(mSegmentsModel->data(mSegmentsModel->index(iter->row(), 0)).toInt());
				}
			}
		}
	}

	//then, convert these ids into the array list
	check_rc(mParameter.set_int_array(segment_ids));
}

void CSelect_Time_Segment_Id_Panel::fetch_parameter() {
	if (!mDb) Connect_To_Db(); //try to connect first

	HRESULT rc;
	std::vector<int64_t> segment_ids = mParameter.as_int_array(rc);

	if (check_rc(rc)) {
		auto is_in_selection = [&segment_ids](const int id)->bool {
			for (const auto segment_id : segment_ids)
				if (segment_id == id) return true;
			return false;
		};

		if (mSegmentsModel)
			for (int data_row = 0;  data_row<mSegmentsModel->rowCount(); data_row++) {
				if (is_in_selection(mSegmentsModel->data(mSegmentsModel->index(data_row, 0)).toInt()))
					selectRow(data_row);
			}	
	}
}

void CSelect_Time_Segment_Id_Panel::Connect_To_Db() {
//	auto current_selection = get_parameter();

	mSegmentsModel.reset(nullptr);
	mSegmentsQuery.reset(nullptr);
	if (mDb) {
		QString connection;
		connection = mDb->connectionName();
		mDb.reset(nullptr);
		QSqlDatabase::removeDatabase(connection);
	}

	mDb = std::make_unique<QSqlDatabase>(QSqlDatabase::addDatabase(QString::fromStdWString(mConfiguration.Read_String(rsDb_Provider)), mDb_Connection_Name));
	mDb->setHostName(QString::fromStdWString(mConfiguration.Read_String(rsDb_Host)));
	mDb->setDatabaseName(QString::fromStdWString(mConfiguration.Read_String(rsDb_Name)));
	mDb->setUserName(QString::fromStdWString(mConfiguration.Read_String(rsDb_User_Name)));
	mDb->setPassword(QString::fromStdWString(mConfiguration.Read_String(rsDb_Password)));
	
	if (mDb->open()) {

		mSegmentsQuery = std::make_unique<QSqlQuery>(*mDb.get());
		mSegmentsQuery->prepare(QString::fromWCharArray(rsSelect_Subjects_And_Segments_For_Db_Reader_Filter));
		mSegmentsQuery->exec();

		mSegmentsModel = std::make_unique<QSqlQueryModel>();
		mSegmentsModel->setQuery(*mSegmentsQuery.get());

		//and fix the UI
		setModel(mSegmentsModel.get());
		setSelectionMode(QAbstractItemView::MultiSelection);
		setSelectionBehavior(QAbstractItemView::SelectRows);

		hideColumn(0); //segment it id
		mSegmentsModel->setHeaderData(1, Qt::Horizontal, tr(dsSubject));
		mSegmentsModel->setHeaderData(2, Qt::Horizontal, tr(dsSegment));
		mSegmentsModel->setHeaderData(3, Qt::Horizontal, tr(dsValue_Count));

		//set_parameter(current_selection);
		fetch_parameter();
	}
}

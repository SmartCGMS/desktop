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
 * Univerzitni 8, 301 00 Pilsen
 * Czech Republic
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
 * a) This file is available under the Apache License, Version 2.0.
 * b) When publishing any derivative work or results obtained using this software, you agree to cite the following paper:
 *    Tomas Koutny and Martin Ubl, "SmartCGMS as a Testbed for a Blood-Glucose Level Prediction and/or 
 *    Control Challenge with (an FDA-Accepted) Diabetic Patient Simulation", Procedia Computer Science,  
 *    Volume 177, pp. 354-362, 2020
 */

#include "Select_Subject_Panel.h"

#include <scgms/lang/dstrings.h>
#include <scgms/rtl/FilterLib.h>
#include <scgms/iface/DbIface.h>

#include "moc_Select_Subject_Panel.cpp"

#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QHeaderView>

constexpr int Subject_Selection_Anonymous = 0;
constexpr int Subject_Selection_Create_New = 1;
constexpr int Subject_Selection_Existing = 2;

CSelect_Subject_Panel::CSelect_Subject_Panel(scgms::SFilter_Configuration_Link configuration, scgms::SFilter_Parameter &parameter, QWidget *parent)
	: CContainer_Edit(parameter), QWidget(parent), mConfiguration(configuration) {

	QVBoxLayout* layout = new QVBoxLayout();

	QRadioButton* radioBtn;

	mButtonGroup = new QButtonGroup();

	radioBtn = new QRadioButton(dsAnonymous_Subject);
	radioBtn->setChecked(true);
	mButtonGroup->addButton(radioBtn, Subject_Selection_Anonymous);
	layout->addWidget(radioBtn);
	connect(radioBtn, SIGNAL(clicked()), this, SLOT(On_Radio_Button_Selected()));

	radioBtn = new QRadioButton(dsCreate_New_Subject);
	mButtonGroup->addButton(radioBtn, Subject_Selection_Create_New);
	layout->addWidget(radioBtn);
	connect(radioBtn, SIGNAL(clicked()), this, SLOT(On_Radio_Button_Selected()));

	radioBtn = new QRadioButton(dsExisting_Subject);
	mButtonGroup->addButton(radioBtn, Subject_Selection_Existing);
	layout->addWidget(radioBtn);
	connect(radioBtn, SIGNAL(clicked()), this, SLOT(On_Radio_Button_Selected()));

	mDbSubjects = new QTableView();
	layout->addWidget(mDbSubjects, 1);

	setLayout(layout);
}

void CSelect_Subject_Panel::On_Radio_Button_Selected() {
	const int selection = mButtonGroup->checkedId();
	mDbSubjects->setEnabled((selection == Subject_Selection_Existing));
}

void CSelect_Subject_Panel::store_parameter() {
	int64_t subject_id = db::Anonymous_Subject_Identifier;
	const int selection = mButtonGroup->checkedId();
	switch (selection) {
		case Subject_Selection_Anonymous:
			subject_id = db::Anonymous_Subject_Identifier;
			break;
		case Subject_Selection_Create_New:
			subject_id = db::New_Subject_Identifier;
			break;
		case Subject_Selection_Existing:
			if (mDb && mDb->isOpen()) {
				auto selindexes = mDbSubjects->selectionModel()->selectedIndexes();

				if (selindexes.size() != 0) {

					auto iterend = selindexes.end();
					auto iterbegin = selindexes.begin();
					auto selcol = iterbegin->column();

					//and get all selected time segment ids
					for (auto iter = iterbegin; iter < iterend; iter++) {
						if (iter->column() == selcol) {
							subject_id = mSubjectsModel->data(mSubjectsModel->index(iter->row(), 0)).toInt();
							break;
						}
					}
				}
			}
			break;
	}

	check_rc(mParameter->Set_Int64(subject_id));
}

void CSelect_Subject_Panel::fetch_parameter() {
	if (!mDb) {
		Connect_To_Db();
	}

	HRESULT rc;
	const auto db_id = mParameter.as_int(rc);
	if (check_rc(rc)) {
		switch (db_id) {
			case db::Anonymous_Subject_Identifier:
				mButtonGroup->button(Subject_Selection_Anonymous)->click();
				mDbSubjects->setEnabled(false);
				break;
			case db::New_Subject_Identifier:
				mButtonGroup->button(Subject_Selection_Create_New)->click();
				mDbSubjects->setEnabled(false);
				break;
			default:
				mButtonGroup->button(Subject_Selection_Existing)->click();
				mDbSubjects->setEnabled(true);
				if (mSubjectsModel) {
					for (int data_row = 0; data_row < mSubjectsModel->rowCount(); data_row++) {
						if (mSubjectsModel->data(mSubjectsModel->index(data_row, 0)).toInt() == db_id) {
							mDbSubjects->selectRow(data_row);
							break;
						}
					}
				}
				break;
		}
	}
}

void CSelect_Subject_Panel::Connect_To_Db() {

	mSubjectsModel.reset(nullptr);
	
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
		QSqlQuery subjects_query{ *mDb.get() };

		subjects_query.prepare(QString::fromWCharArray(rsSelect_Subjects));
		subjects_query.exec();

		mSubjectsModel = std::make_unique<QSqlQueryModel>();
		mSubjectsModel->setQuery( std::move(subjects_query));

		mDbSubjects->setModel(mSubjectsModel.get());
		mDbSubjects->setSelectionMode(QAbstractItemView::SingleSelection);
		mDbSubjects->setSelectionBehavior(QAbstractItemView::SelectRows);

		mDbSubjects->hideColumn(0);
		mSubjectsModel->setHeaderData(1, Qt::Horizontal, tr(dsSubject));
		mDbSubjects->horizontalHeader()->setStretchLastSection(true);

		//set_parameter(current_selection);
		fetch_parameter();
	}
}

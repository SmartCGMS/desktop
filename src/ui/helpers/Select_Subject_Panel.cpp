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

#include "Select_Subject_Panel.h"

#include "../../../../common/lang/dstrings.h"
#include "../../../../common/rtl/FilterLib.h"
#include "../../../../common/iface/DbIface.h"

#include "moc_Select_Subject_Panel.cpp"

#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QHeaderView>

constexpr int Subject_Selection_Anonymous = 0;
constexpr int Subject_Selection_Create_New = 1;
constexpr int Subject_Selection_Existing = 2;

CSelect_Subject_Panel::CSelect_Subject_Panel(glucose::SFilter_Configuration configuration, glucose::SFilter_Parameter &parameter, QWidget *parent)
	: QWidget(parent), mConfiguration(configuration), CContainer_Edit(parameter) {
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

void CSelect_Subject_Panel::On_Radio_Button_Selected()
{
	const int selection = mButtonGroup->checkedId();
	mDbSubjects->setEnabled((selection == Subject_Selection_Existing));
}

glucose::TFilter_Parameter CSelect_Subject_Panel::get_parameter()
{
	int64_t subject_id = db::Anonymous_Subject_Identifier;
	const int selection = mButtonGroup->checkedId();
	switch (selection)
	{
		case Subject_Selection_Anonymous:
			subject_id = db::Anonymous_Subject_Identifier;
			break;
		case Subject_Selection_Create_New:
			subject_id = db::New_Subject_Identifier;
			break;
		case Subject_Selection_Existing:
			if (mDb && mDb->isOpen())
			{
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

	glucose::TFilter_Parameter result;
	result.config_name = nullptr;
	result.type = glucose::NParameter_Type::ptSubject_Id;
	result.int64 = subject_id;
	return result;
}

void CSelect_Subject_Panel::set_parameter(const glucose::TFilter_Parameter &param)
{
	if (!mDb)
		apply();

	switch (param.int64)
	{
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
			if (mSubjectsModel)
			{
				for (int data_row = 0; data_row < mSubjectsModel->rowCount(); data_row++)
				{
					if (mSubjectsModel->data(mSubjectsModel->index(data_row, 0)).toInt() == param.int64)
					{
						mDbSubjects->selectRow(data_row);
						break;
					}
				}
			}
			break;
	}
}

void CSelect_Subject_Panel::apply()
{
	auto current_selection = get_parameter();

	auto get_attr = [this](const wchar_t* attr_name)->std::wstring {

		for (const auto &param : mConfiguration) {
			if (WChar_Container_Equals_WString(param.config_name, attr_name)) {
				return WChar_Container_To_WString(param.wstr);
			}
		}

		return std::wstring{};
	};

	mSubjectsModel.reset(nullptr);
	mSubjectsQuery.reset(nullptr);
	if (mDb)
	{
		QString connection;
		connection = mDb->connectionName();
		mDb.reset(nullptr);
		QSqlDatabase::removeDatabase(connection);
	}

	mDb = std::make_unique<QSqlDatabase>(QSqlDatabase::addDatabase(QString::fromStdWString(get_attr(rsDb_Provider)), mDb_Connection_Name));
	mDb->setHostName(QString::fromStdWString(get_attr(rsDb_Host)));
	mDb->setDatabaseName(QString::fromStdWString(get_attr(rsDb_Name)));
	mDb->setUserName(QString::fromStdWString(get_attr(rsDb_User_Name)));
	mDb->setPassword(QString::fromStdWString(get_attr(rsDb_Password)));
	
	if (mDb->open())
	{
		mSubjectsQuery = std::make_unique<QSqlQuery>(*mDb.get());
		mSubjectsQuery->prepare(QString::fromWCharArray(rsSelect_Subjects));
		mSubjectsQuery->exec();

		mSubjectsModel = std::make_unique<QSqlQueryModel>();
		mSubjectsModel->setQuery(*mSubjectsQuery.get());

		mDbSubjects->setModel(mSubjectsModel.get());
		mDbSubjects->setSelectionMode(QAbstractItemView::SingleSelection);
		mDbSubjects->setSelectionBehavior(QAbstractItemView::SelectRows);

		mDbSubjects->hideColumn(0);
		mSubjectsModel->setHeaderData(1, Qt::Horizontal, tr(dsSubject));
		mDbSubjects->horizontalHeader()->setStretchLastSection(true);

		set_parameter(current_selection);
	}

	Release_Filter_Parameter(current_selection);
}
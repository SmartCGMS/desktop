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
	QListWidget *lbxApplied_Filters, *lbxAvailable_Filters = nullptr;
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

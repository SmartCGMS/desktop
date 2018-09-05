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
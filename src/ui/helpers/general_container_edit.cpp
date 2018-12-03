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


#include "general_container_edit.h"

#include "../../../../common/rtl/referencedImpl.h"
#include "../../../../common/lang/dstrings.h"

#include <QtGui/QValidator>


#include "moc_general_container_edit.cpp"

namespace filter_config_window {
	
	glucose::TFilter_Parameter CWChar_Container_Edit ::get_parameter() {
		glucose::TFilter_Parameter result;
		result.type = glucose::NParameter_Type::ptWChar_Container;
		const std::wstring str = text().toStdWString();
		result.wstr = refcnt::WString_To_WChar_Container(str.c_str());
		return result;
}

	void CWChar_Container_Edit ::set_parameter(const glucose::TFilter_Parameter &param) {
		setText(QString::fromStdWString(WChar_Container_To_WString(param.wstr)));
	}
	

	CDouble_Container_Edit::CDouble_Container_Edit(QWidget *parent) : QLineEdit(parent) {
		auto validator = new QDoubleValidator(this);
		// force english locale rules (e.g. dot decimal separator)
		validator->setLocale(QLocale(QLocale::English));
		setValidator(validator);
	}

	glucose::TFilter_Parameter CDouble_Container_Edit::get_parameter() {
		glucose::TFilter_Parameter result;
		result.type = glucose::NParameter_Type::ptDouble;
		bool ok;
		result.dbl = text().toDouble(&ok);
		if (!ok)
			result.dbl = 0.0;
		return result;
	}

	void CDouble_Container_Edit::set_parameter(const glucose::TFilter_Parameter &param) {
		setText(QString::number(param.dbl));
	}
	

	double CRatTime_Container_Edit::QTime2RatTime(const QTime &qdt) {
		const size_t msecs = qdt.msecsSinceStartOfDay();
		return static_cast<double>(msecs)*InvMSecsPerDay;
	}

	QTime CRatTime_Container_Edit::rattime2QTime(const double rt) {
		QTime tmp(0, 0, 0, 0);
		return tmp.addMSecs((int)(rt*MSecsPerDay));
	}

	
	CRatTime_Container_Edit::CRatTime_Container_Edit(QWidget *parent) : QDateTimeEdit(parent) {
		setDisplayFormat(rsRattime_Edit_Mask);
	}

	glucose::TFilter_Parameter CRatTime_Container_Edit::get_parameter() {
		glucose::TFilter_Parameter result;
		result.type = glucose::NParameter_Type::ptRatTime;

		result.dbl = QTime2RatTime(time());
		return result;
	}

	void CRatTime_Container_Edit::set_parameter(const glucose::TFilter_Parameter &param) {
		setTime(rattime2QTime(param.dbl));
	}
	


	CInteger_Container_Edit::CInteger_Container_Edit(QWidget *parent) : QLineEdit(parent) {
		setValidator(new QIntValidator(this));
	}

	glucose::TFilter_Parameter CInteger_Container_Edit::get_parameter() {
		glucose::TFilter_Parameter result;
		result.type = glucose::NParameter_Type::ptInt64;
		bool ok;
		result.int64 = text().toLongLong(&ok);
		if (!ok)
			result.int64 = 0;
		return result;
	}

	void CInteger_Container_Edit::set_parameter(const glucose::TFilter_Parameter &param) {
		setText(QString::number(param.int64));
	}

	
	glucose::TFilter_Parameter CBoolean_Container_Edit::get_parameter() {
		glucose::TFilter_Parameter result;
		result.type = glucose::NParameter_Type::ptBool;
		result.boolean = (checkState() == Qt::Checked);
		return result;
	}

	void CBoolean_Container_Edit::set_parameter(const glucose::TFilter_Parameter &param) {
		setCheckState(param.boolean ? Qt::Checked : Qt::Unchecked);
	}
	

	glucose::TFilter_Parameter CNull_Container_Edit::get_parameter() {
		return glucose::Null_Filter_Parameter;
	}

	void CNull_Container_Edit::set_parameter(const glucose::TFilter_Parameter &param) {
	}
	
}
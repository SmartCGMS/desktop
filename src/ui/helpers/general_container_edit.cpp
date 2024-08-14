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

#include "general_container_edit.h"

#include <scgms/rtl/referencedImpl.h>
#include <scgms/rtl/UILib.h>
#include <scgms/utils/string_utils.h>
#include <scgms/rtl/rattime.h>
#include <scgms/lang/dstrings.h>

#include <QtGui/QValidator>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QApplication>
#include <cmath>

#include "moc_general_container_edit.cpp"

namespace filter_config_window {

	CContainer_Edit::CContainer_Edit(scgms::SFilter_Parameter parameter) : mParameter(parameter) {
		//fetch_parameter(); - avoid possibly virtual call from ctor
	}

	bool CContainer_Edit::check_rc(const HRESULT rc) {
		if (!Succeeded(rc)) {

			wchar_t *conf_name = nullptr;
			mParameter->Get_Config_Name(&conf_name);
			const auto narrowed_name = Narrow_WChar(conf_name);

			const QString qstr = QString::fromWCharArray(dsParameter_Configuration_Failed_RC).arg(conf_name ? narrowed_name.c_str() : "").arg(rc, 0, 16);
			QMessageBox::warning(QApplication::activeWindow(), dsInformation, qstr);
			return false;
		}
		else {
			return true;
		}
	}

	CInteger_Container_Edit::CInteger_Container_Edit(scgms::SFilter_Parameter parameter, QWidget *parent) : CContainer_Edit(parameter), QLineEdit(parent) {
		setValidator(new QIntValidator(this));
	}

	void CInteger_Container_Edit::fetch_parameter() {
		HRESULT rc;
		setText(QString::number(mParameter.as_int(rc)));
		check_rc(rc);
	}

	void CInteger_Container_Edit::store_parameter() {
		HRESULT rc;
		bool ok;
		int64_t int64 = text().toLongLong(&ok);
		if (ok) {
			rc = mParameter->Set_Int64(int64);
		}
		else {
			rc = E_FAIL;
		}

		check_rc(rc);
	}

	CWChar_Container_Edit::CWChar_Container_Edit(scgms::SFilter_Parameter parameter, QWidget *parent) : CContainer_Edit(parameter), QLineEdit(parent) {
		//
	}

	void CWChar_Container_Edit::fetch_parameter() {
		HRESULT rc;
		const std::wstring str = mParameter.as_wstring(rc, false);
		if (Succeeded(rc)) {
			setText(QString::fromStdWString(str));
		}
		else if (rc == E_NOT_SET) {
			clear(); //just not set
		}
		else {
			check_rc(rc); //truly en error
		}
	}

	void CWChar_Container_Edit::store_parameter() {
		check_rc(mParameter.set_wstring(text().toStdWString()));
	}

	CRatTime_Validator::CRatTime_Validator(QWidget* parent) : QValidator(parent) {
	}

	bool CRatTime_Validator::allowed_chars_only(const QString &input) {

		int first = 0;
		int len = input.size();

		if (len == 0) {
			return false;
		}

		//we allow minus only as the very first char
		if (input[0] == '-') {
			first++;
			//len--;
		}

		for (auto i = first; i < len; i++) {
			switch (input[i].toLatin1()) {
				case '$':
				case '(':
				case ')':
					//support for variables
				case ' ':
				case ':':
				case '.':
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					break;
				default:
					return false;
			}
		}

		return true;
	}

	bool CRatTime_Validator::string_to_rattime(const QString& input, double& converted) {
		bool result = false;
		converted = Default_Str_To_Rat_Time(input.toStdWString(), result);
		return result;
	}

	QString CRatTime_Validator::rattime_to_string(double rattime) {
		return QString::fromStdWString(Rat_Time_To_Default_WStr(rattime));
	}

	void CRatTime_Validator::fixup(QString& input) const {
		input = input.simplified();
	}

	QValidator::State CRatTime_Validator::validate(QString& input, int& pos) const {
		if (!allowed_chars_only(input)) {
			return QValidator::Invalid;
		}
			
		auto [is_var, var_name] = scgms::Is_Variable_Name(input.toStdWString());
		if (is_var) {
			return QValidator::Acceptable;
		}

		double tmp;
		return string_to_rattime(input.simplified(), tmp) ? QValidator::Acceptable : QValidator::Invalid; //do not allow Intermediate as the user may possible enter a non-sense
	}

	CRatTime_Container_Edit::CRatTime_Container_Edit(scgms::SFilter_Parameter parameter, QWidget* parent) : CContainer_Edit(parameter), QLineEdit(parent), mValidator(new CRatTime_Validator{parent}) {
		setValidator(mValidator);
	}

	void CRatTime_Container_Edit::fetch_parameter() {
		if (mParameter) {
			HRESULT rc;
			std::wstring raw_text = mParameter.as_wstring(rc, false);
			if (check_rc(rc)) {
				setText(QString::fromStdWString(raw_text));
			}
		}
	}
	
	void CRatTime_Container_Edit::store_parameter() {
		check_rc(mParameter.set_wstring(text().toStdWString()));
	}

	double CRatTime_Container_Edit::as_double() {
		double result;
		return mValidator->string_to_rattime(text(), result) ? result : std::numeric_limits<double>::quiet_NaN();
	}

	void CRatTime_Container_Edit::set_double(const double value) {
		setText(mValidator->rattime_to_string(value));
	}

	CDouble_Validator::CDouble_Validator(QWidget* parent) : QValidator(parent) {
		//
	}

	void CDouble_Validator::fixup(QString& input) const {
		input = input.simplified();
	}

	QValidator::State CDouble_Validator::validate(QString& input, int& pos) const {
		auto [is_var, var_name] = scgms::Is_Variable_Name(input.toStdWString());
		if (is_var) {
			return QValidator::Acceptable;
		}

		auto [ok, dbl] = text_2_dbl(input);
		
		return ok ? QValidator::Acceptable : QValidator::Intermediate; 
	}

	std::tuple<bool, double> CDouble_Validator::text_2_dbl(const QString& text) {
		bool ok = false;
		std::wstring str = text.simplified().toStdWString(); //wstr to allow infinity symbol
		double converted = str_2_dbl(str.c_str(), ok);
		if (!ok) {
			ok = CRatTime_Validator::string_to_rattime(text, converted);
		}

		if (!ok) {
			converted = std::numeric_limits<double>::quiet_NaN();
		}

		return std::tuple<bool, double>{ok, converted};
	}

	CDouble_Container_Edit::CDouble_Container_Edit(scgms::SFilter_Parameter parameter, QWidget *parent) : 
			CContainer_Edit(parameter), QLineEdit(parent), mValidator(new  CDouble_Validator{ parent }) {
		
		setValidator(mValidator);
	}

	void CDouble_Container_Edit::store_parameter() {
		check_rc(mParameter.set_wstring(text().toStdWString()));
	}

	void CDouble_Container_Edit::fetch_parameter() {
		if (mParameter) {
			HRESULT rc;
			std::wstring raw_text = mParameter.as_wstring(rc, false);
			if (check_rc(rc)) {
				setText(QString::fromStdWString(raw_text));
			}
		}
	}
	

	double CDouble_Container_Edit::as_double() {
		auto [ok, dbl] = mValidator->text_2_dbl(text());
		return dbl;	//nan if !ok
	}

	void CDouble_Container_Edit::set_double(const double value) {
		std::wstring converted = dbl_2_wstr(value);
		setText(QString::fromStdWString(converted));
	}

	CBoolean_Container_Edit::CBoolean_Container_Edit(scgms::SFilter_Parameter parameter, QWidget *parent) : CContainer_Edit(parameter), QCheckBox(parent) {
		//
	}

	void CBoolean_Container_Edit::fetch_parameter() {
		HRESULT rc;
		setCheckState(mParameter.as_bool(rc) ? Qt::Checked : Qt::Unchecked);
		check_rc(rc);
	}

	void CBoolean_Container_Edit::store_parameter() {
		check_rc(mParameter.set_bool(checkState() == Qt::Checked));
	}

	CGUID_Validator::CGUID_Validator(QWidget* parent) : QValidator(parent) {

	}

	void CGUID_Validator::fixup(QString& input) const {
		input = input.simplified();
	}

	CGUID_Validator::State CGUID_Validator::validate(QString& input, int& pos) const {
		bool ok;
		/*const GUID tmp =*/ WString_To_GUID(input.toStdWString(), ok);
		return ok ? CGUID_Validator::State::Acceptable : CGUID_Validator::State::Invalid;
	}

	CGUIDCombo_Container_Edit::CGUIDCombo_Container_Edit(scgms::SFilter_Parameter parameter, QWidget *parent) :
		CContainer_Edit(parameter), QComboBox(parent), mValidator(new CGUID_Validator{parent}) {
		mParameter = parameter; //fixing some strange behavior, may be Qt implied, that enforces default ctor of cont_edit, thus not setting mParameter

		setEditable(true);
		setValidator(mValidator);
	}

	void CGUIDCombo_Container_Edit::fetch_parameter() {
		HRESULT rc;
		const GUID id = mParameter.as_guid(rc);

		if (check_rc(rc)) {

			bool index_found = false;
			for (int i = 0; i < count(); i++) {
				if (id == *reinterpret_cast<const GUID*>(itemData(i).toByteArray().constData())) {
					setCurrentIndex(i);
					index_found = true;
					break;
				}
			}

			if (!index_found) {
				setCurrentText(QString::fromStdWString(GUID_To_WString(id)));
			}
		}
	}

	void CGUIDCombo_Container_Edit::store_parameter() {
		//if the current text can be interpreted as valid GUID, let's store the converted text
		//else, we store the currentData
	
		GUID id = Invalid_GUID;
		std::wstring current_str = currentText().toStdWString();
		current_str.erase(std::remove_if(current_str.begin(), current_str.end(), ::iswspace), current_str.end());
				
		const bool user_intends_invalid_guid = current_str == GUID_To_WString(Invalid_GUID);

		if (!user_intends_invalid_guid) {
			bool ok;
			id = WString_To_GUID(current_str, ok);
			if ((!ok) && (currentIndex() >= 0)) {
				id = *reinterpret_cast<const GUID*>(currentData().toByteArray().constData());
			}
		}

		//const GUID id = currentIndex() >= 0 ? *reinterpret_cast<const GUID*>(currentData().toByteArray().constData()) : WString_To_GUID(currentText().toStdWString());
		check_rc(mParameter->Set_GUID(&id));
	}

	CNull_Container_Edit::CNull_Container_Edit(QWidget *parent) : CContainer_Edit(scgms::SFilter_Parameter{}), QWidget(parent) {
		//
	}

	void CNull_Container_Edit::fetch_parameter() {
	}

	void CNull_Container_Edit::store_parameter() {
	}
}

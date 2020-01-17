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
#include "../../../../common/utils/string_utils.h"
#include "../../../../common/lang/dstrings.h"

#include <QtGui/QValidator>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QApplication>
#include <cmath>

#include "moc_general_container_edit.cpp"

namespace filter_config_window {
	

	CContainer_Edit::CContainer_Edit(scgms::SFilter_Parameter parameter) : mParameter(parameter) {
		fetch_parameter();
	}

	bool CContainer_Edit::check_rc(const HRESULT rc) {
		
		if (!SUCCEEDED(rc)) {

			wchar_t *conf_name = nullptr;
			mParameter->Get_Config_Name(&conf_name);

			const QString qstr = QString::fromWCharArray(dsParameter_Configuration_Failed_RC).arg(conf_name ? Narrow_WChar(conf_name).c_str() : "").arg(rc, 0, 16);
			QMessageBox::warning(QApplication::activeWindow(), dsInformation, qstr);
			return false;
		}
		else
			return true;
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
		} else
			rc = E_FAIL;

		check_rc(rc);
	}

	CWChar_Container_Edit::CWChar_Container_Edit(scgms::SFilter_Parameter parameter, QWidget *parent) : QLineEdit(parent), CContainer_Edit(parameter) {
		//
	}

	void CWChar_Container_Edit::fetch_parameter() {
		HRESULT rc;
		setText(QString::fromStdWString(mParameter.as_wstring(rc)));
		check_rc(rc);
	}

	void CWChar_Container_Edit::store_parameter() {		
		check_rc(mParameter.set_wstring(text().toStdWString().c_str()));
	}


	bool CRatTime_Validator::allowed_chars_only(const QString &input) const {

		int first = 0;
		int len = input.size();

		if (len == 0) return false;

		//we allow minus only as the very first char
		if (input[0] == '-') {
			first++;
			len--;
		}

		QStringRef vw(&input, first, len);

		for (auto ch : vw) {
			switch (ch.toLatin1()) {
				case ' ': case ':': case '.':
				case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': break;
				default: return false;
			}
		}


		return true;
	}

	bool CRatTime_Validator::string_to_rattime(const QString &input, double& converted) const {
		double days = 0.0, hours = 0.0, minutes = 0.0, seconds = 0.0;		

		double plus_minus_sign = 1.0;
		int plus_minus_pos = 0;	//must be signed int!
		if (input.size() == 0) return false;
		if (input[0] == '-') {
			plus_minus_sign = -1.0;
			plus_minus_pos = 1;
		}


		int pos, last_pos = input.size();
			
		auto fetch_number = [&](const char sep, const char decimal, double &result, const double result_max) {
			pos = last_pos-1;				

			while (pos >= plus_minus_pos) {
				const char ch = input[pos].toLatin1();
				if (ch == sep) break;					
				if (!isdigit(ch) && (ch != decimal)) return false;

				pos--;
			}

			pos++;
			QStringRef substring(&input, pos, last_pos - pos);
			bool ok;
			result = substring.toDouble(&ok);
			if ((!ok) || (result >= result_max)) return false;

			last_pos = pos-1;
			return true;
		};


		//search for seconds, minutes, hours and days

		if (!fetch_number(':', '.', seconds, 60.0)) return false;

		if (last_pos > plus_minus_pos) {
			if (!fetch_number(':', 0, minutes, 60.0)) return false;

			if (last_pos > plus_minus_pos) {
				if (!fetch_number(' ', 0, hours, 24.0)) return false;

				if (last_pos> plus_minus_pos)
					if (!fetch_number('-', 0, days, std::numeric_limits<double>::max())) return false;
			}
		}

		converted = plus_minus_sign*(days + scgms::One_Hour * hours + scgms::One_Minute * minutes + scgms::One_Second * seconds);	

		return true;
	}

	QString CRatTime_Validator::rattime_to_string(double rattime) const {
		if (std::isnan(rattime))
			return QString::fromStdWString(dsNaN);

		auto add_fraction = [&](const double factor) {
			double intpart;
			rattime *= factor;
			rattime = std::modf(rattime, &intpart);

			if (factor == 1.0) {	//days
				return intpart != 0.0 ? QString::number(static_cast<int>(intpart)) + ' ' : QString{ "" };
			} else
				return QString::number(static_cast<int>(intpart)).rightJustified(2, '0');
		};


		//days
		QString result{ rattime < 0.0 ? "-" : "" };
		rattime = std::fabs(rattime);
		result += add_fraction(1.0);
		result += add_fraction(24.0) + ':';
		result += add_fraction(60.0) + ':';
		result += add_fraction(60.0);

		return result;
	}

	void CRatTime_Validator::fixup(QString& input) const {
		input = input.simplified();
	}

	QValidator::State CRatTime_Validator::validate(QString& input, int& pos) const {
		if (!allowed_chars_only(input)) return QValidator::Invalid;
			
		double tmp;
		return  string_to_rattime(input.simplified(), tmp) ? QValidator::Acceptable : QValidator::Invalid; //do not allow Intermediate as the user may possible enter a non-sense
	}
	

	CRatTime_Container_Edit::CRatTime_Container_Edit(scgms::SFilter_Parameter parameter, QWidget *parent) : QLineEdit(parent), CContainer_Edit(parameter) {				
		setValidator(mValidator);
	}

	
	void CRatTime_Container_Edit::fetch_parameter() {
		if (mParameter) {
			HRESULT rc;
			setText(mValidator->rattime_to_string(mParameter.as_double(rc)));
			check_rc(rc);
		}
	}
	
	void CRatTime_Container_Edit::store_parameter() {
		check_rc(mParameter->Set_Double(as_double()));
	}


	double CRatTime_Container_Edit::as_double() {
		double result;
		return mValidator->string_to_rattime(text(), result) ? result : std::numeric_limits<double>::quiet_NaN();		
	}

	void CRatTime_Container_Edit::set_double(const double value) {
		setText(mValidator->rattime_to_string(value));		
	}


	CDouble_Container_Edit::CDouble_Container_Edit(scgms::SFilter_Parameter parameter, QWidget *parent) : QLineEdit(parent), CContainer_Edit(parameter) {
		auto validator = new QDoubleValidator(this);
		// force english locale rules (e.g. dot decimal separator)
		validator->setLocale(QLocale(QLocale::English));
		setValidator(validator);
	}

	void CDouble_Container_Edit::store_parameter() {
		HRESULT rc;

		bool ok;
		const double dbl = text().toDouble(&ok);
		if (ok) rc = mParameter->Set_Double(dbl);
			else rc = E_FAIL;
		
		check_rc(rc);
	}

	void CDouble_Container_Edit::fetch_parameter() {
		if (mParameter) {
			HRESULT rc;
			setText(QString::number(mParameter.as_double(rc)));
			check_rc(rc);
		}
	}
	

	double CDouble_Container_Edit::as_double() {
		bool ok;
		const double dbl = text().toDouble(&ok);
		if (ok) return dbl;
			else return std::numeric_limits<double>::quiet_NaN();
	}

	void CDouble_Container_Edit::set_double(const double value) {
		setText(QString::number(value));
	}

	CBoolean_Container_Edit::CBoolean_Container_Edit(scgms::SFilter_Parameter parameter, QWidget *parent) : QCheckBox(parent), CContainer_Edit(parameter) {
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


	CGUIDCombo_Container_Edit::CGUIDCombo_Container_Edit(scgms::SFilter_Parameter parameter, QWidget *parent) :
		QComboBox(parent), CContainer_Edit(parameter) {
		mParameter = parameter; //fixing some strange behavior, may be Qt implied, that enforces default ctor of cont_edit, thus not setting mParameter
	}

	void CGUIDCombo_Container_Edit::fetch_parameter() {
		HRESULT rc;
		const GUID id = mParameter.as_guid(rc);

		if (check_rc(rc)) {
			for (int i = 0; i < count(); i++) {
				if (id == *reinterpret_cast<const GUID*>(itemData(i).toByteArray().constData())) {
					setCurrentIndex(i);
					break;
				}
			}
		}		

	}

	void CGUIDCombo_Container_Edit::store_parameter() {
		const GUID id = *reinterpret_cast<const GUID*>(currentData().toByteArray().constData());		
		check_rc(mParameter->Set_GUID(&id));
	}

	CNull_Container_Edit::CNull_Container_Edit(QWidget *parent) : QWidget(parent), CContainer_Edit(scgms::SFilter_Parameter{}) {
		//
	}


	void CNull_Container_Edit::fetch_parameter() {
	}

	void CNull_Container_Edit::store_parameter() {
	}
}
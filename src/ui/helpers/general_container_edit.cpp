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
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QApplication>


#include "moc_general_container_edit.cpp"

namespace filter_config_window {
	

	CContainer_Edit::CContainer_Edit(glucose::SFilter_Parameter parameter) : mParameter(parameter) {
		fetch_parameter();
	}

	bool CContainer_Edit::check_rc(const HRESULT rc) {
		
		if (!SUCCEEDED(rc)) {

			wchar_t *conf_name = nullptr;
			mParameter->Get_Config_Name(&conf_name);

			const QString qstr = QString::fromWCharArray(dsParameter_Configuration_Failed_RC).arg(conf_name ? conf_name : L"").arg(rc, 0, 16);
			QMessageBox::warning(QApplication::activeWindow(), dsInformation, qstr);
			return false;
		}
		else
			return true;
	}


	CInteger_Container_Edit::CInteger_Container_Edit(glucose::SFilter_Parameter parameter, QWidget *parent) : CContainer_Edit(parameter), QLineEdit(parent) {
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
			mParameter.set_int(int64, rc);
		} else
			rc = E_FAIL;

		check_rc(rc);
	}

	CWChar_Container_Edit::CWChar_Container_Edit(glucose::SFilter_Parameter parameter, QWidget *parent) : QLineEdit(parent), CContainer_Edit(parameter) {
		//
	}

	void CWChar_Container_Edit::fetch_parameter() {
		HRESULT rc;
		setText(QString::fromStdWString(mParameter.as_string(rc)));
		check_rc(rc);
	}

	void CWChar_Container_Edit::store_parameter() {
		HRESULT rc;
		mParameter.set_string(text().toStdWString().c_str(), rc);
		check_rc(rc);
	}


	CRatTime_Container_Edit::CRatTime_Container_Edit(glucose::SFilter_Parameter parameter, QWidget *parent) : QDateTimeEdit(parent), CContainer_Edit(parameter) {
		setDisplayFormat(rsRattime_Edit_Mask);
	}

	double CRatTime_Container_Edit::QTime2RatTime(const QTime &qdt) {
		const size_t msecs = qdt.msecsSinceStartOfDay();
		return static_cast<double>(msecs)*InvMSecsPerDay;
	}

	QTime CRatTime_Container_Edit::rattime2QTime(const double rt) {
		QTime tmp(0, 0, 0, 0);
		return tmp.addMSecs((int)(rt*MSecsPerDay));
	}

	void CRatTime_Container_Edit::fetch_parameter() {
		HRESULT rc;
		setTime(rattime2QTime(mParameter.as_double(rc));
		check_rc(rc);
	}
	
	void CRatTime_Container_Edit::store_parameter() {
		HRESULT rc;
		mParameter.set_double(QTime2RatTime(this->time()), rc);
		check_rc(rc);
	}



	CDouble_Container_Edit::CDouble_Container_Edit(glucose::SFilter_Parameter parameter, QWidget *parent) : QLineEdit(parent), CContainer_Edit(parameter) {
		auto validator = new QDoubleValidator(this);
		// force english locale rules (e.g. dot decimal separator)
		validator->setLocale(QLocale(QLocale::English));
		setValidator(validator);
	}

	void CDouble_Container_Edit::store_parameter() {
		HRESULT rc;

		bool ok;
		const double dbl = text().toDouble(&ok);
		if (ok) mParameter.set_double(dbl, rc);
			else rc = E_FAIL;
		
		check_rc(rc);
	}

	void CDouble_Container_Edit::fetch_parameter() {
		HRESULT rc;
		setText(QString::number(mParameter.as_double(rc)));
		check_rc(rc);
	}
	

	CBoolean_Container_Edit::CBoolean_Container_Edit(glucose::SFilter_Parameter parameter, QWidget *parent) : QCheckBox(parent), CContainer_Edit(parameter) {
		//
	}
	

	void CBoolean_Container_Edit::fetch_parameter() {
		HRESULT rc;
		setCheckState(mParameter.as_bool(rc) ? Qt::Checked : Qt::Unchecked);
		check_rc(rc);
	}

	void CBoolean_Container_Edit::store_parameter() {
		HRESULT rc;

		mParameter.set_bool(checkState() == Qt::Checked);

		check_rc(rc);
	}


	CGUIDCombo_Container_Edit::CGUIDCombo_Container_Edit(glucose::SFilter_Parameter parameter, QWidget *parent) :
		QComboBox(parent), CContainer_Edit(parameter) {
		//
	}

	void CGUIDCombo_Container_Edit::fetch_parameter() {
		const GUID id = *reinterpret_cast<const GUID*>(currentData().toByteArray().constData());
		HRESULT rc;
		mParameter.set_guid(id, rc);
		check_rc(rc);

	}

	void CGUIDCombo_Container_Edit::store_parameter() {
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

}
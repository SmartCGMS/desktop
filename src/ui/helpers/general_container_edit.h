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

#pragma once

#include <scgms/rtl/FilterLib.h>

#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>

namespace filter_config_window {

	class CContainer_Edit {
		protected:
			scgms::SFilter_Parameter mParameter;
			bool check_rc(const HRESULT rc);	//returns true if rc Succeeded, else displays an error message

		public:
			CContainer_Edit() {};
			CContainer_Edit(scgms::SFilter_Parameter parameter);
			virtual ~CContainer_Edit() {};
			virtual void fetch_parameter() {};	//loads the parameter into the UI element
			virtual void store_parameter() = 0;	//stores the parameter from the UI element
	};

	class IAs_Double_Container  {
		public:
			virtual ~IAs_Double_Container() {};
			virtual double as_double() = 0;
			virtual void set_double(const double value) = 0;
	};

	class CInteger_Container_Edit : public QLineEdit, public virtual filter_config_window::CContainer_Edit {
		Q_OBJECT
		public:
			CInteger_Container_Edit(scgms::SFilter_Parameter parameter, QWidget *parent);
			virtual void fetch_parameter() override;
			virtual void store_parameter() override;
	};

	class CWChar_Container_Edit : public QLineEdit, public virtual filter_config_window::CContainer_Edit {
		public:
			CWChar_Container_Edit(scgms::SFilter_Parameter parameter, QWidget *parent);
			virtual void fetch_parameter() override;
			virtual void store_parameter() override;
	};

	//allows time: [-][days hours:minutes:]seconds.zzz with minutes, hours and days optional
	class CRatTime_Validator : public QValidator {
		protected:
			static bool allowed_chars_only(const QString& input);

		public:
			CRatTime_Validator(QWidget* parent);

			static bool string_to_rattime(const QString& input, double& converted);
			static QString rattime_to_string(double rattime);
			virtual void fixup(QString& input) const override;
			virtual State validate(QString& input, int& pos) const override;
	};

	//allows time: [-][days hours:minutes:]seconds.zzz with minutes, hours and days optional
	class CRatTime_Container_Edit : public QLineEdit, public virtual filter_config_window::CContainer_Edit, public virtual IAs_Double_Container {
		protected:
			CRatTime_Validator* mValidator = nullptr;

		public:
			CRatTime_Container_Edit(scgms::SFilter_Parameter parameter, QWidget *parent);
			virtual ~CRatTime_Container_Edit() {};
			virtual void fetch_parameter() override;
			virtual void store_parameter() override;

			virtual double as_double() override;
			virtual void set_double(const double value) override;
	};

	//we provide custom validator that accepts symbolic constants and rat time
	class CDouble_Validator : public QValidator {
		public:
			CDouble_Validator(QWidget* parent);

			virtual void fixup(QString& input) const override;
			virtual State validate(QString& input, int& pos) const override;

			//accepts both double and rattime
			static std::tuple<bool, double> text_2_dbl(const QString& str);
	};

	class CDouble_Container_Edit : public QLineEdit, public virtual filter_config_window::CContainer_Edit, public virtual IAs_Double_Container {
		protected:
			CDouble_Validator* mValidator = nullptr;

		public:
			CDouble_Container_Edit(scgms::SFilter_Parameter parameter, QWidget *parent);
			virtual ~CDouble_Container_Edit() {};
			virtual void fetch_parameter() override;
			virtual void store_parameter() override;

			virtual double as_double() override;
			virtual void set_double(const double value) override;
	};

	class CBoolean_Container_Edit : public QCheckBox, public virtual filter_config_window::CContainer_Edit {
		public:
			CBoolean_Container_Edit(scgms::SFilter_Parameter parameter, QWidget *parent);
			virtual void fetch_parameter() override;
			virtual void store_parameter() override;
	};

	class CGUID_Validator : public QValidator {
		public:
			CGUID_Validator(QWidget* parent);
			virtual void fixup(QString& input) const override;
			virtual State validate(QString& input, int& pos) const override;
	};

	class CGUIDCombo_Container_Edit : public QComboBox, public virtual filter_config_window::CContainer_Edit {
		protected:
			CGUID_Validator* mValidator = nullptr;

		public:
			CGUIDCombo_Container_Edit(scgms::SFilter_Parameter parameter, QWidget *parent);
			virtual void fetch_parameter() override;
			virtual void store_parameter() override;
	};

	class CNull_Container_Edit : public QWidget, public virtual filter_config_window::CContainer_Edit {
		public:
			CNull_Container_Edit(QWidget *parent);
			virtual void fetch_parameter() override;
			virtual void store_parameter() override;
	};
}

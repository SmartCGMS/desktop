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

#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

#include <scgms/rtl/scgmsLib.h>
#include <scgms/utils/winapi_mapping.h>
#include <scgms/utils/DebugHelper.h>
#include <scgms/lang/dstrings.h>
#include <scgms/utils/QtUtils.h>

#include "ui/main_window.h"

int MainCalling main(int argc, char *argv[]) {

	QApplication application(argc, argv);
	qGuiApp->setWindowIcon(QIcon(":/app/appicon.png"));
	qGuiApp->setApplicationName(StdWStringToQString(dsGPredict3_App_Name));
	qGuiApp->setOrganizationDomain(StdWStringToQString(dsGPredict3_App_Domain));

	if (!scgms::is_scgms_loaded()) {
		QMessageBox::information(nullptr, dsInformation, dsSCGMS_Not_Loaded);
		return 3;
	}

	// determine config file path
	const std::wstring config_filepath = argc > 1 ? std::wstring{ argv[1], argv[1] + strlen(argv[1]) } : std::wstring{};

	// create the GUI
	CMain_Window main_window{ config_filepath };
	main_window.show();
	
	return application.exec();
}

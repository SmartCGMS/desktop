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
 * a) For non-profit, academic research, this software is available under the
 *      GPLv3 license.
 * b) For any other use, especially commercial use, you must contact us and
 *       obtain specific terms and conditions for the use of the software.
 * c) When publishing work with results obtained using this software, you agree to cite the following paper:
 *       Tomas Koutny and Martin Ubl, "Parallel software architecture for the next generation of glucose
 *       monitoring", Procedia Computer Science, Volume 141C, pp. 279-286, 2018
 */

#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

#include "../../common/rtl/scgmsLib.h"
#include "../../common/utils/winapi_mapping.h"
#include "../../common/utils/DebugHelper.h"
#include "../../common/lang/dstrings.h"

#include "ui/main_window.h"

int MainCalling main(int argc, char *argv[]) {
    

	QApplication application(argc, argv);
    qGuiApp->setWindowIcon(QIcon(":/app/appicon.png"));

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
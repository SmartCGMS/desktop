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

#include "ui/main_window.h"

#include "../../common/desktop-console/config.h"
#include "../../common/rtl/FilterLib.h"
#include "filters/descriptor.h"

#include <QtWidgets/QApplication>



int __cdecl main(int argc, char *argv[]) {

	QApplication application(argc, argv);
	// inject GUI-specific loaders to generic code
	glucose::add_filters(gui::get_gui_filter_descriptors(), &gui::create_gui_filter);

	
	Configuration.Resolve_And_Load_Config_File(argc > 1 ? std::wstring{ argv[1], argv[1] + strlen(argv[1]) } : std::wstring{});	//config uses QApp to determine the file path (to be platorm indepenedent) and it has to be initialized first
																															//but it tries to load custom config as well

	//create the GUI
	CMain_Window main_window{};
	main_window.show();
	
	return application.exec();
}
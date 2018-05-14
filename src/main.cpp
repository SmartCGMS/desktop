#include "ui/main_window.h"

#include "../../common/desktop-console/config.h"
#include "../../common/rtl/FilterLib.h"
#include "filters/descriptor.h"

#include <QtWidgets/QApplication>



int __cdecl main(int argc, char *argv[]) {

	QApplication application(argc, argv);
	// inject GUI-specific loaders to generic code
	glucose::add_filters(gui::get_gui_filter_descriptors(), &gui::create_gui_filter);

	
	Configuration.Resolve_And_Load_Config_File();	//config uses QApp to determine the file path (to be platorm indepenedent) and it has to be initialized first

	//create the GUI
	CMain_Window main_window{};
	main_window.show();
	
	return application.exec();
}
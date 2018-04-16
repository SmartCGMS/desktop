#include "ui/main_window.h"

#include "../../common/desktop-console/config.h"
#include "../../common/rtl/FilterLib.h"
#include "filters/descriptor.h"

#include <QtWidgets/QApplication>



int __cdecl main(int argc, char *argv[]) {

	// inject GUI-specific loaders to generic code
	glucose::inject_filter_loader(&gui::do_get_filter_descriptors, &gui::do_create_filter);

	QApplication application(argc, argv);
	Configuration.Resolve_And_Load_Config_File();	//config uses QApp to determine the file path (to be platorm indepenedent) and it has to be initialized first

	//create the GUI
	CMain_Window main_window{};
	main_window.show();
	
	return application.exec();
}
#include "ui/main_window.h"

#include <QtWidgets/QApplication>


int __cdecl main(int argc, char *argv[]) {

	QApplication application(argc, argv);

	//create the GUI
	CMain_Window main_window{};
	main_window.show();
	
	return application.exec();
}
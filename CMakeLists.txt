CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

SET(PROJ "gpredict3-desktop")

MESSAGE(STATUS "Configuring desktop project")

CONFIGURE_COMMON(${PROJ})

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE SRC_GUI_FILTERS src/filters/*.cpp src/filters/*.hpp src/filters/*.c src/filters/*.h)
FILE(GLOB_RECURSE SRC_GUI_UI src/ui/*.cpp src/ui/*.hpp src/ui/*.c src/ui/*.h)
FILE(GLOB_RECURSE SRC_GUI_RES src/resource/*.cpp src/resource/*.hpp src/resource/*.c src/resource/*.h)
IF(WIN32)
	FILE(GLOB SRC_ADD_RES src/platform/win/*.rc)
	LIST(APPEND SRC_GUI_RES ${SRC_ADD_RES})
ENDIF()
LIST(FILTER SRC_GUI_UI EXCLUDE REGEX ".*moc_.*.cpp$")

SET(DESKTOP_RTL_SRC )
ADD_RTL_FILE(DESKTOP_RTL_SRC "qdb_connector.h")
ADD_RTL_FILE(DESKTOP_RTL_SRC "qdb_connector.cpp")

SET(DESKTOP_UTILS_SRC )
ADD_UTILS_FILE(DESKTOP_RTL_SRC "string_utils.h")
ADD_UTILS_FILE(DESKTOP_RTL_SRC "string_utils.cpp")

ADD_EXECUTABLE(${PROJ} src/main.cpp ${COMMON_FILES} ${COMMON_DESKTOP_CONSOLE_FILES} ${SRC_GUI_FILTERS} ${SRC_GUI_UI} ${SRC_GUI_RES} ${DESKTOP_RTL_SRC} ${DESKTOP_UTILS_SRC})

TARGET_LINK_LIBRARIES(${PROJ} Qt5::Core Qt5::Sql Qt5::Widgets Qt5::Svg Qt5::Gui scgms)

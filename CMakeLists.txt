CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

SET(PROJ "gpredict3-desktop")

MESSAGE(STATUS "Configuring desktop project")

CONFIGURE_COMMON(${PROJ})

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

FILE(GLOB_RECURSE SRC_GUI_FILTERS src/filters/*.cpp src/filters/*.hpp src/filters/*.c src/filters/*.h)
FILE(GLOB_RECURSE SRC_GUI_UI src/ui/*.cpp src/ui/*.hpp src/ui/*.c src/ui/*.h)
LIST(FILTER SRC_GUI_UI EXCLUDE REGEX ".*moc_.*.cpp$")

SET(DESKTOP_RTL_SRC )
ADD_RTL_FILE(DESKTOP_RTL_SRC "qdb_connector.h")
ADD_RTL_FILE(DESKTOP_RTL_SRC "qdb_connector.cpp")

ADD_EXECUTABLE(${PROJ} src/main.cpp ${COMMON_FILES} ${COMMON_DESKTOP_CONSOLE_FILES} ${SRC_GUI_FILTERS} ${SRC_GUI_UI} ${DESKTOP_RTL_SRC})

TARGET_LINK_LIBRARIES(${PROJ} Qt5::Core Qt5::Sql Qt5::Widgets Qt5::Svg Qt5::Gui factory)

cmake_minimum_required(VERSION 3.0.0)

find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file(GLOB HEADERS *.h)
file(GLOB FORMS *.ui)
qt5_wrap_ui(UI_HEADERS ${FORMS})
qt5_add_resources(QRC src.qrc)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC")

include_directories(${MAIN_SRC_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${Qt5Widgets_INCLUDE_DIRS})

add_library(core
	data_t.cpp
	mainwindow.cpp
	Node.cpp
	NodeBox.cpp
	Workspace.cpp
	${HEADERS} ${UI_HEADERS})

add_executable(${PROJECT_NAME} main.cpp ${QRC})

target_link_libraries(${PROJECT_NAME} core Qt5::Widgets)
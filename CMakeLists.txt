cmake_minimum_required(VERSION 2.6)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

PROJECT(CompareDescriptors)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

QT4_WRAP_UI(UISrcs CompareDescriptorsWidget.ui)
QT4_WRAP_CPP(MOCSrcs CompareDescriptorsWidget.h)

ADD_EXECUTABLE(CompareDescriptors
CompareDescriptors.cpp
CompareDescriptorsWidget.cpp
Helpers.cpp
PointSelectionStyle3D.cpp
${UISrcs} ${MOCSrcs} ${ResourceSrcs})
TARGET_LINK_LIBRARIES(CompareDescriptors QVTK ${VTK_LIBRARIES} ${ITK_LIBRARIES})

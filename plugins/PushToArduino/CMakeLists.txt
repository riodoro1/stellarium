# This is the cmake config file for the PushToArduino plugin
# Copied from the AngleMeasure plugin

SET(PUSHTOARDUINO_VERSION "0.0.1")
ADD_DEFINITIONS(-DPUSHTOARDUINO_VERSION="${PUSHTOARDUINO_VERSION}")

ADD_SUBDIRECTORY( src )

IF(APPLE)
     SET(CMAKE_INSTALL_PREFIX $ENV{HOME}/Library/Application\ Support/Stellarium)
ElSE(APPLE)
     SET(CMAKE_INSTALL_PREFIX $ENV{HOME}/.stellarium)
ENDIF(APPLE)
INSTALL(FILES DESTINATION "modules/PushToArduino")

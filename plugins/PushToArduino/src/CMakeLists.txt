INCLUDE_DIRECTORIES(. gui)
LINK_DIRECTORIES(${BUILD_DIR}/src)

SET(PushToArduino_SRCS
     PushToArduino.hpp
     PushToArduino.cpp
     PushToSerialComm.hpp
     PushToSerialComm.cpp
     gui/PushToArduinoDialog.hpp
     gui/PushToArduinoDialog.cpp
)

SET(PushToArduino_UIS
     gui/pushToArduinoDialog.ui
)

QT5_WRAP_UI(PushToArduino_UIS_H ${PushToArduino_UIS})

################# compiles resources files ############
SET(PushToArduino_RES ../PushToArduino.qrc)
QT5_ADD_RESOURCES(PushToArduino_RES_CXX ${PushToArduino_RES})

ADD_LIBRARY(PushToArduino-static STATIC ${PushToArduino_SRCS} ${PushToArduino_RES_CXX} ${PushToArduino_UIS_H})
TARGET_LINK_LIBRARIES(PushToArduino-static Qt5::Core Qt5::Network Qt5::Widgets Qt5::SerialPort)
# The library target "PushToArduino-static" has a default OUTPUT_NAME of "PushToArduino-static", so change it.
SET_TARGET_PROPERTIES(PushToArduino-static PROPERTIES OUTPUT_NAME "PushToArduino")
SET_TARGET_PROPERTIES(PushToArduino-static PROPERTIES COMPILE_FLAGS "-DQT_STATICPLUGIN")
ADD_DEPENDENCIES(AllStaticPlugins PushToArduino-static)

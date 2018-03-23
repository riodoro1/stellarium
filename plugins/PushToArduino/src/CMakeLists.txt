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

SET(extLinkerOption ${JPEG_LIBRARIES} ${PNG_LIBRARIES} ${OPENGL_LIBRARIES})

ADD_LIBRARY(PushToArduino-static STATIC ${PushToArduino_SRCS} ${PushToArduino_RES_CXX} ${PushToArduino_UIS_H})
QT5_USE_MODULES(PushToArduino-static Core Network Widgets SerialPort)
# The library target "PushToArduino-static" has a default OUTPUT_NAME of "PushToArduino-static", so change it.
SET_TARGET_PROPERTIES(PushToArduino-static PROPERTIES OUTPUT_NAME "PushToArduino")
TARGET_LINK_LIBRARIES(PushToArduino-static ${extLinkerOption})
SET_TARGET_PROPERTIES(PushToArduino-static PROPERTIES COMPILE_FLAGS "-DQT_STATICPLUGIN")
ADD_DEPENDENCIES(AllStaticPlugins PushToArduino-static)

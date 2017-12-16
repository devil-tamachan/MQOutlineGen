cmake_minimum_required(VERSION 3.8)
project (MQOutlineGen)

add_definitions(-D_UNICODE -DUNICODE)

set(MQSDK_DIR "" CACHE PATH "MQInit.cpp")

if(EXISTS ${MQSDK_DIR}/mqsdk/MQInit.cpp)
  set(MQSDK_INCLUDE_DIR ${MQSDK_DIR}/mqsdk/)
elseif(EXISTS ${MQSDK_DIR}/MQInit.cpp)
  set(MQSDK_INCLUDE_DIR ${MQSDK_DIR})
endif()

if(NOT DEFINED MQSDK_INCLUDE_DIR)
  message(FATAL_ERROR "MQSDK_DIR not found!")
endif()

INCLUDE_DIRECTORIES( "${MQSDK_INCLUDE_DIR}" )

IF(WIN32)
  add_library(MQOutlineGen SHARED
    MQOutlineGen.cpp
    ${MQSDK_INCLUDE_DIR}/MQInit.cpp
    ${MQSDK_INCLUDE_DIR}/MQPlugin.cpp
    ${MQSDK_INCLUDE_DIR}/MQWidget.cpp
    ${MQSDK_INCLUDE_DIR}/MQ3DLib.cpp
  )
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /OPT:REF")
ELSE()
  set(MACOSX_BUNDLE_INFO_STRING "tamachan.mq.MQOutlineGen")
  set(MACOSX_BUNDLE_BUNDLE_NAME "tamachan.mq.MQOutlineGen")
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "tamachan.mq.MQOutlineGen")

  add_library(MQOutlineGen MODULE
    MQOutlineGen.cpp
    ${MQSDK_INCLUDE_DIR}/MQInit.cpp
    ${MQSDK_INCLUDE_DIR}/MQPlugin.cpp
    ${MQSDK_INCLUDE_DIR}/MQWidget.cpp
    ${MQSDK_INCLUDE_DIR}/MQ3DLib.cpp
    ${MQSDK_INCLUDE_DIR}/osx/StringUtil.mm
  )
  target_link_libraries(MQOutlineGen "-framework CoreFoundation")
  target_link_libraries(MQOutlineGen "-framework Cocoa")
  set_target_properties(MQOutlineGen PROPERTIES
    BUNDLE TRUE
    BUNDLE_EXTENSION plugin
  )
ENDIF()

if(CMAKE_CL_64)
  set_target_properties(MQOutlineGen PROPERTIES OUTPUT_NAME "MQOutlineGen64")
endif()

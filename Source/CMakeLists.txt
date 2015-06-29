cmake_minimum_required(VERSION 2.8.3)
PROJECT(MediaKit)
INCLUDE(${CMAKE_SOURCE_DIR}/common.cmake)

#marco definiton
ADD_DEFINITIONS(-DUSE_FFMPEG)

macro(ADD_FRAMEWORK fwname appname)
find_library(FRAMEWORK_${fwname}
NAMES ${fwname}
PATHS ${CMAKE_OSX_SYSROOT}/System/Library/Frameworks ../../thirdparty/SDL
PATH_SUFFIXES
NO_DEFAULT_PATH)
if( ${FRAMEWORK_${fwname}} STREQUAL FRAMEWORK_${fwname}-NOTFOUND)
MESSAGE(ERROR ": Framework ${fwname} not found")
else()
TARGET_LINK_LIBRARIES(${appname} "${FRAMEWORK_${fwname}}/${fwname}")
MESSAGE(STATUS "Framework ${fwname} found at ${FRAMEWORK_${fwname}}")
endif()
endmacro(ADD_FRAMEWORK)


SET(CMAKE_BUILD_TYPE Release)
set(INC_PATH ./)
set(SRC_PATH ./)

#Head File
ADD_DEFINITIONS(-DREGISTER_MEDIA_HANDLERS -DHAVE_CONFIG_H)
INCLUDE_DIRECTORIES(${INC_PATH}/MediaCore/media/)
INCLUDE_DIRECTORIES(${INC_PATH}/MediaCore/media/FFmpeg/)
INCLUDE_DIRECTORIES(${INC_PATH}/MediaCore/sound/)
INCLUDE_DIRECTORIES(${INC_PATH}/MediaCore/sound/sdl)
INCLUDE_DIRECTORIES(${INC_PATH}/MediaCore/netWork/)
INCLUDE_DIRECTORIES(${INC_PATH}/MediaCore/netWork/curl)
INCLUDE_DIRECTORIES(${INC_PATH}/MediaCore/netWork/bt)
INCLUDE_DIRECTORIES(${INC_PATH}/MediaCore/base/)
INCLUDE_DIRECTORIES(${INC_PATH}/MediaCore/timer/)
INCLUDE_DIRECTORIES(${INC_PATH}/MediaCore/platform)
INCLUDE_DIRECTORIES(${INC_PATH}/MediaCore/thirdParty/glew/include)
INCLUDE_DIRECTORIES(${INC_PATH}/MediaCore/)
INCLUDE_DIRECTORIES(${INC_PATH}/MTF/mtf/)
INCLUDE_DIRECTORIES(/usr/include/)
INCLUDE_DIRECTORIES(/usr/local/include/)

#framework Path

#Source File 
AUX_SOURCE_DIRECTORY(${SRC_PATH}/MediaCore/media/ SRC_LIST)
AUX_SOURCE_DIRECTORY(${SRC_PATH}/MediaCore/media/FFmpeg/ SRC_LIST)
AUX_SOURCE_DIRECTORY(${SRC_PATH}/MediaCore/netWork/curl/ SRC_LIST)
AUX_SOURCE_DIRECTORY(${SRC_PATH}/MediaCore/netWork/bt/ SRC_LIST)
AUX_SOURCE_DIRECTORY(${SRC_PATH}/MediaCore/netWork/ SRC_LIST)
AUX_SOURCE_DIRECTORY(${SRC_PATH}/MediaCore/sound/ SRC_LIST)
AUX_SOURCE_DIRECTORY(${SRC_PATH}/MediaCore/sound/sdl SRC_LIST)
AUX_SOURCE_DIRECTORY(${SRC_PATH}/MediaCore/base/ SRC_LIST)
AUX_SOURCE_DIRECTORY(${SRC_PATH}/MediaCore/timer/ SRC_LIST)
AUX_SOURCE_DIRECTORY(${SRC_PATH}/MediaCore/platform/ SRC_LIST)
AUX_SOURCE_DIRECTORY(${SRC_PATH}/MediaCore/platform/linux SRC_LIST)
AUX_SOURCE_DIRECTORY(${SRC_PATH}/MediaCore/thirdParty/glew/src/ SRC_LIST)
AUX_SOURCE_DIRECTORY(${SRC_PATH}/MTF/mtf/ SRC_LIST)
SET(SRC_LIST ${SRC_LIST}  ${SRC_PATH}/MediaCore/AVPipeline.cpp)
SET(SRC_LIST ${SRC_LIST}  ${SRC_PATH}/MediaCore/main.cpp)

#Depend librarys
#find_package(SDL REQUIRED)
#if(SDL_FOUND)
#	message(STATUS "Required SDL version >= 1.2.0: yes")
#	INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR})
#else(SDL_FOUND)
#	message(SEND_ERROR "Required SDL is't founded")
#endif(SDL_FOUND)
#find_package(CURL REQUIRED)
#find_package(Boost REQUIRED)
#find_package(GLUT REQUIRED)
#find_package(OpenGL REQUIRED)
#find_package(ZLIB REQUIRED)
#look for ffmpeg librarys

INCLUDE_DIRECTORIES(./MediaCore/thirdParty/glew/include/)
INCLUDE_DIRECTORIES(../../thirdparty/include)
INCLUDE_DIRECTORIES(../../thirdparty/glew/)
INCLUDE_DIRECTORIES(../../thirdparty/boost_1_58_0)
INCLUDE_DIRECTORIES(/opt/X11/include)
#INCLUDE_DIRECTORIES(../../thirdparty/SDL)
#INCLUDE_DIRECTORIES(../../thirdparty/SDL/SDL.framework)

LINK_DIRECTORIES(/usr/lib)
LINK_DIRECTORIES(/usr/local/lib)
LINK_DIRECTORIES(../../thirdparty/lib)
LINK_DIRECTORIES(../../thirdparty/boost_1_58_0/stage/lib)
LINK_DIRECTORIES(/opt/X11/lib)

MESSAGE(STATUS "${SRC_LIST}")
ADD_EXECUTABLE(mediaplayer ${SRC_LIST})

add_framework(CoreFoundation mediaplayer)
add_framework(AVFoundation mediaplayer)
add_framework(AudioToolbox mediaplayer)
add_framework(CoreVideo mediaplayer)
add_framework(CoreMedia mediaplayer)
add_framework(VideoDecodeAcceleration mediaplayer)
add_framework(GLUT mediaplayer)
add_framework(OpenGL mediaplayer)

#add_framework(SDL mediaplayer)

TARGET_LINK_LIBRARIES(mediaplayer -lpthread -lboost_system -lz -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswscale -lswresample  -lz -lboost_thread -lbz2 -lcurl -liconv -Wl,-rpath,../../thirdparty/boost_1_58_0/stage/lib)

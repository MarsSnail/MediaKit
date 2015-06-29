cmake_minimum_required(VERSION 2.8.3)
#For debug
SET(CMAKE_VERBOSE_MAKEFILE TRUE)

SET(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -g -pipe -Wall")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

IF (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "mips64_1")
	# for mips shared library build
	SET(CMAKE_CXX_FLAGS_DEBUG "-g -fPIC -Wall -mlong-calls")
	SET(CMAKE_C_FLAGS_DEBUG "-g -fPIC -Wall -mlong-calls")
	SET(CMAKE_CXX_FLAGS_RELEASE "-O1 -g -fPIC -Wall -mlong-calls -fthread-jumps -falign-functions  -falign-jumps -falign-loops  -falign-labels -fcaller-saves -fcrossjumping -fcse-follow-jumps  -fcse-skip-blocks -fdelete-null-pointer-checks -fexpensive-optimizations -fgcse -fgcse-lm -foptimize-sibling-calls -fpeephole2 -fregmove -freorder-blocks  -freorder-functions -frerun-cse-after-loop -fsched-interblock  -fsched-spec -fschedule-insns -fschedule-insns2 -fstrict-aliasing -fstrict-overflow -ftree-pre -ftree-vrp -finline-functions -funswitch-loops -fpredictive-commoning -fgcse-after-reload -ftree-vectorize")
	SET(CMAKE_C_FLAGS_RELEASE "-O1 -g -fPIC -Wall -mlong-calls -fthread-jumps -falign-functions  -falign-jumps -falign-loops  -falign-labels -fcaller-saves -fcrossjumping -fcse-follow-jumps  -fcse-skip-blocks -fdelete-null-pointer-checks -fexpensive-optimizations -fgcse -fgcse-lm -foptimize-sibling-calls -fpeephole2 -fregmove -freorder-blocks  -freorder-functions -frerun-cse-after-loop -fsched-interblock  -fsched-spec -fschedule-insns -fschedule-insns2 -fstrict-aliasing -fstrict-overflow -ftree-pre -ftree-vrp -finline-functions -funswitch-loops -fpredictive-commoning -fgcse-after-reload -ftree-vectorize")
ENDIF(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "mips64_1")

#Global variables
#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
#SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

IF(NOT EXECUTABLE_OUTPUT_PATH)
	SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
ENDIF(NOT EXECUTABLE_OUTPUT_PATH)

IF(NOT LIBRARY_OUTPUT_PATH)
	SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
ENDIF(NOT LIBRARY_OUTPUT_PATH)

if (EXECUTABLE_OUTPUT_PATH)
else (EXECUTABLE_OUTPUT_PATH)
endif (EXECUTABLE_OUTPUT_PATH)

IF(NOT DEFINED ENV{static_lib})
	SET(ENV{static_lib})
ENDIF(NOT DEFINED ENV{static_lib})

IF(NOT DEFINED ENV{static_obj})
	SET(ENV{static_obj})
ENDIF(NOT DEFINED ENV{static_obj})

IF(NOT DEFINED ENV{shared_lib})
	SET(ENV{shared_lib})
ENDIF(NOT DEFINED ENV{shared_lib})

IF(NOT DEFINED ENV{lib_path})
	SET(ENV{lib_path})
ENDIF(NOT DEFINED ENV{lib_path})

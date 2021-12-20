PROJECT(bigtable)

SET(USE_VTUNE "0") 

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g -Wall -pthread -O2")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
SET(CMAKE_C_COMPILER "gcc")

CMAKE_MINIMUM_REQUIRED(VERSION 3.1.0)

FIND_PACKAGE(PkgConfig REQUIRED)

PKG_CHECK_MODULES(GLIB REQUIRED glib-2.0)

if (USE_VTUNE)
    include_directories(
        /opt/intel/oneapi/vtune_profiler/latest/Intel\ oneAPI\ VTune\ Profiler\ 2021.2.0.app/Contents/include   # for vtune instrumentation
    )
    
    ADD_LIBRARY(ittnotify STATIC IMPORTED)
    SET_TARGET_PROPERTIES(ittnotify PROPERTIES IMPORTED_LOCATION /opt/intel/oneapi/vtune_profiler/latest/Intel\ oneAPI\ VTune\ Profiler\ 2021.2.0.app/Contents/sdk/lib64/libittnotify.a)
    SET (COMMON_LIB ittnotify ${COMMON_LIB})
  
endif()

INCLUDE_DIRECTORIES(
    ./
	${GLIB_INCLUDE_DIRS}
)

INCLUDE_DIRECTORIES(src/include)

LINK_DIRECTORIES(${GLIB_LIBRARY_DIRS})

FILE(GLOB SRC src/bigtable.c)

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${SRC})

TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME} ${GLIB_LIBRARIES})
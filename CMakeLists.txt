cmake_minimum_required(VERSION 2.6)
project(ednio)
set(EDNIO_VER_MAJOR 0)
set(EDNIO_VER_MINOR 9)
set(EDNIO_VER_PATCH 1)
set(EDNIO_VER ${EDNIO_VER_MAJOR}.${EDNIO_VER_MINOR}.${EDNIO_VER_PATCH} )

# set default build option
set(USE_SSL 0)
option(SSL "Use SSL library" true)
if(SSL)
  set(USE_SSL 1)
endif(SSL)

configure_file (
  "${PROJECT_SOURCE_DIR}/src/ednio_config.h.cmakein"
	"${PROJECT_SOURCE_DIR}/src/ednio_config.h"
)

include_directories("${PROJECT_BINARY_DIR}")
set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++0x -O3 -Wall -fmessage-length=0 " )
set(CMAKE_CXX_FLAGS "-std=c++0x -pthread " )
file(GLOB ALLSRCS "src/*.cpp" )
#file(GLOB ALLSRCS "src/*.cpp" "src/http/*.cpp" )
if(SSL)
  file(GLOB ALLSRCS ${ALLSRCS} "src/edssl/*.cpp")
endif(SSL)

add_library(ednio SHARED ${ALLSRCS})
add_library(ednio_static STATIC ${ALLSRCS})

target_link_libraries(ednio "pthread")
if(SSL)
	target_link_libraries(ednio "ssl" )
endif(SSL)


#set_target_properties(ednio PROPERTIES SUFFIX .${EDNIO_VER_MAJOR}.0.${EDNIO_VER_MINOR}.so)
#get_target_property(suffix ednio SUFFIX)
#MESSAGE('suffix = ${suffix}')
set_target_properties(ednio PROPERTIES SOVERSION 0 VERSION ${EDNIO_VER})
install(TARGETS ednio DESTINATION lib ) 
#install(FILES  DESTINATION ednio )
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/ DESTINATION include/ednio
        FILES_MATCHING PATTERN "*.h")
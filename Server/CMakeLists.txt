include_directories("${PROJECT_SOURCE_DIR}/CommonLibrary")

set (HEADERS 
"Server.h"
)

set (SOURCES
"Server.cpp"
"main.cpp"
)

source_group("Header files" FILES ${HEADERS})

source_group("Source files" FILES ${SOURCES})

set (PROJECT_CONTENT ${SOURCES} ${HEADERS})

add_executable (Server ${PROJECT_CONTENT})

target_link_libraries(Server LIB)

#START BOOST
add_definitions(-DBOOST_ALL_NO_LIB)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.53 REQUIRED system)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	target_link_libraries(Server ${Boost_LIBRARIES})
endif()
#END BOOST
	

set_property(TARGET Server PROPERTY FOLDER "executables")

install (TARGETS Server RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin)
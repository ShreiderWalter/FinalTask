include_directories("${PROJECT_SOURCE_DIR}/CommonLibrary")

set (HEADERS 
"Client.h"
"resource.h"
)

set (SOURCES
"Client.cpp"
"main.cpp"
"Client.rc"
)

source_group("Header files" FILES ${HEADERS})

source_group("Source files" FILES ${SOURCES})

set (PROJECT_CONTENT ${SOURCES} ${HEADERS})

add_executable (Client ${PROJECT_CONTENT})

target_link_libraries(Client LIB)

set_property(TARGET Client PROPERTY FOLDER "executables")

set_target_properties(Client PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")

set_target_properties(Client PROPERTIES COMPILE_DEFINITIONS_DEBUG "_WINDOWS")

set_target_properties(Client PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")

set_target_properties(Client PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_WINDOWS")

set_target_properties(Client PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")

set_target_properties(Client PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")


#START BOOST
add_definitions(-DBOOST_ALL_NO_LIB)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.53 REQUIRED system)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	target_link_libraries(Client ${Boost_LIBRARIES})
endif()

#message(STATUS "Boost_INCLUDE_DIR	: ${Boost_INCLUDE_DIR}")
#message(STATUS "Boost_LIBRARY_DIRS	: ${Boost_LIBRARY_DIRS}")
#END BOOST

install (TARGETS Client RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin)
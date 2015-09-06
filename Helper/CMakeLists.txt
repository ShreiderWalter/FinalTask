include_directories("${PROJECT_SOURCE_DIR}/CommonLibrary")

set (HEADERS 
"Helper.h"
)

set (SOURCES
"Helper.cpp"
"main.cpp"
)

source_group("Header files" FILES ${HEADERS})

source_group("Source files" FILES ${SOURCES})

set (PROJECT_CONTENT ${SOURCES} ${HEADERS})

add_executable (Helper ${PROJECT_CONTENT})

target_link_libraries(Helper LIB)

set_property(TARGET Helper PROPERTY FOLDER "executables")

install (TARGETS Helper RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin)
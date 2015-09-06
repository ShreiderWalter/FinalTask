set (HEADERS 
"ScreenUtils.h"
"SharedMemoryManager.h"
)

set (SOURCES
"ScreenUtils.cpp"
"SharedMemoryManager.cpp"
)

source_group("Header files" FILES ${HEADERS})

source_group("Source files" FILES ${SOURCES})

set (PROJECT_CONTENT ${SOURCES} ${HEADERS})

include_directories(.)

add_library (LIB STATIC ${PROJECT_CONTENT})

set_property(TARGET LIB PROPERTY FOLDER "libraries")

install (TARGETS LIB ARCHIVE DESTINATION ${PROJECT_BINARY_DIR}/bin
		     RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin
		     LIBRARY DESTINATION ${PROJECT_BINARY_DIR}/bin)
cmake_minimum_required (VERSION 2.6)

project (FinalTask)

set_property(GLOBAL PROPERTY ON)

message ("cxx Flags: " ${CMAKE_CXX_FLAGS})

add_subdirectory(Client)
add_subdirectory(CommonLibrary)
add_subdirectory(Server)
add_subdirectory(Helper)
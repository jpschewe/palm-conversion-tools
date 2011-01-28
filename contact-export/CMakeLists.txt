CMAKE_MINIMUM_REQUIRED (VERSION 2.6) 
PROJECT (contact-export) 

set(CMAKE_CXX_FLAGS "-g -Wall -Werror")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})


ADD_EXECUTABLE (export-contacts export-contacts.cpp ) 

TARGET_LINK_LIBRARIES (export-contacts pisock ) 

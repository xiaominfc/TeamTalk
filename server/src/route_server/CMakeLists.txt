cmake_minimum_required(VERSION 2.8.12)
PROJECT(routeserver)

AUX_SOURCE_DIRECTORY(./ SRC_LIST)

#SET(LIBRARY_OUTPUT_PATH ./)
SET(EXECUTABLE_OUTPUT_PATH  ./)

INCLUDE(${PROJECT_SOURCE_DIR}/../base/base_dep.cmake)

INCLUDE_DIRECTORIES(./)

#ADD_XXXX必须在TARGET_LINK_LIBRARIES前面，否则会报错
#ADD_LIBRARY(${PROJECTNAME} SHARED/STATIC ${SRC_LIST})
ADD_EXECUTABLE(route_server ${SRC_LIST})

TARGET_LINK_LIBRARIES(route_server base slog protobuf-lite pthread ssl crypto)

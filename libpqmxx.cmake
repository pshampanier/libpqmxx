# libpqmxx
# --------
#
# Add the libpqmxx library to the project.
#
# This script defines
#
#   LIBPQMXX_LIBRARIES - the libpqmxx libraries needed for linking
#   LIBPQMXX_INCLUDE_DIRS - the directories of the libpmxx headers
#
# Usage
# ^^^^^
#
#  set(CMAKE_CXX_STANDARD 11)
#  set(CMAKE_CXX_STANDARD_REQUIRED on)
#
#  include_directories(${LIBPQMXX_INCLUDE_DIRS})
#  target_link_libraries(project ${LIBPQMXX_LIBRARIES})

set(LIBPQMXX_LIBRARIES    "libpqmxx")
set(LIBPQMXX_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include)

# Source & header files
file(GLOB LIBPQMXX_SOURCE_FILES ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB LIBPQMXX_INCLUDE_FILES ${LIBPQMXX_INCLUDE_DIRS}/*.h)

# IDE source group
source_group("Header Files" FILES ${LIBPQMXX_INCLUDE_FILES})

# Add the library to the project
add_library(${LIBPQMXX_LIBRARIES} STATIC ${LIBPQMXX_SOURCE_FILES} ${LIBPQMXX_INCLUDE_FILES})

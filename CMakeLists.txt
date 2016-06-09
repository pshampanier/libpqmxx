cmake_minimum_required (VERSION 3.1)
project (postgres-test)

# PostgreSQL_LIBRARIES    the PostgreSQL libraries needed for linking
# PostgreSQL_INCLUDE_DIRS the directories of the PostgreSQL headers
# OPENSSL_LIBRARIES       the OpenSSL libraries needed for linking

# C++ standard 14 for lampda functions and closure support
set (CMAKE_CXX_STANDARD 14)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(CMAKE_XCODE_ATTRIBUTE_ENABLE_TESTABILITY "YES")
endif()

set(ALL_LIBRARIES "")

macro(add_libraries target libraries)
  foreach(library ${libraries})
    get_filename_component(library_absolute "${CMAKE_BINARY_DIR}/${library}" ABSOLUTE)
    # set(ALL_LIBRARIES "${ALL_LIBRARIES} ${library_absolute}")
    target_link_libraries(${target} ${library_absolute})
  endforeach()
endmacro()

if (PostgreSQL_INCLUDE_DIRS)
  get_filename_component(PostgreSQL_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/${PostgreSQL_INCLUDE_DIRS}" ABSOLUTE)
endif()

macro(googletest target)
  set(GTEST_RELEASE 1.7.0)
  set(DIR_GTEST_SOURCES ${CMAKE_BINARY_DIR}/googletest-release-${GTEST_RELEASE})
  if(NOT IS_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-release-${GTEST_RELEASE})
    file(DOWNLOAD https://github.com/google/googletest/archive/release-${GTEST_RELEASE}.tar.gz ${DIR_GTEST_SOURCES}.tar.gz)
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar -zxf ${DIR_GTEST_SOURCES}.tar.gz WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    execute_process(
      COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" ${DIR_GTEST_SOURCES}
      WORKING_DIRECTORY ${DIR_GTEST_SOURCES}
    )
    execute_process(
      COMMAND ${CMAKE_MAKE_PROGRAM}
      WORKING_DIRECTORY ${DIR_GTEST_SOURCES}
    )
  endif()

  include_directories(${DIR_GTEST_SOURCES}/include)
  find_library(LIB_GTEST      gtest       PATH ${CMAKE_BINARY_DIR}/googletest-release-${GTEST_RELEASE}/Debug)
  find_library(LIB_GTEST_MAIN gtest_main  PATH ${CMAKE_BINARY_DIR}/googletest-release-${GTEST_RELEASE}/Debug)
  target_link_libraries(${target} ${LIB_GTEST} ${LIB_GTEST_MAIN})
  
endmacro(googletest)

#
# postgres-test (Unit Testing)
#
macro(postgres_test target)

  include_directories(${PostgreSQL_INCLUDE_DIRS})

  file(GLOB TEST_FILES ${CMAKE_CURRENT_LIST_DIR}/test/*.cpp)
  file(GLOB SOURCE_FILES ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
  file(GLOB INCLUDE_FILES ${CMAKE_CURRENT_LIST_DIR}/include/*.h)
  source_group("test" FILES ${TEST_FILES})
  source_group("include" FILES ${INCLUDE_FILES})
  include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
  add_executable(${target} ${TEST_FILES} ${INCLUDE_FILES} ${SOURCE_FILES})
  add_libraries(${target} "${OPENSSL_LIBRARIES}")
  add_libraries(${target} "${PostgreSQL_LIBRARIES}")
  googletest(${target})
endmacro(postgres_test)

postgres_test(postgres-test)
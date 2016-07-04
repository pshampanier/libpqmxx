cmake_minimum_required (VERSION 3.5)
project (postgres-test)

# C++ standard 14 for lampda functions and closure support
set (CMAKE_CXX_STANDARD 14)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(CMAKE_XCODE_ATTRIBUTE_ENABLE_TESTABILITY "YES")
endif()

find_package(PostgreSQL REQUIRED)

macro(googletest target)
  set(GTEST_RELEASE 1.7.0)
  set(DIR_GTEST_SOURCES ${CMAKE_BINARY_DIR}/googletest-release-${GTEST_RELEASE})
  if(NOT IS_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-release-${GTEST_RELEASE})
    file(DOWNLOAD https://github.com/google/googletest/archive/release-${GTEST_RELEASE}.tar.gz ${DIR_GTEST_SOURCES}.tar.gz)
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar -zxf ${DIR_GTEST_SOURCES}.tar.gz WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    execute_process(
      COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" -Dgtest_disable_pthreads=ON ${DIR_GTEST_SOURCES}
      WORKING_DIRECTORY ${DIR_GTEST_SOURCES}
    )
    execute_process(
      COMMAND ${CMAKE_MAKE_PROGRAM}
      WORKING_DIRECTORY ${DIR_GTEST_SOURCES}
    )
  endif()

  include_directories(${DIR_GTEST_SOURCES}/include)
  find_library(LIB_GTEST gtest PATHS 
    ${CMAKE_BINARY_DIR}/googletest-release-${GTEST_RELEASE}
    ${CMAKE_BINARY_DIR}/googletest-release-${GTEST_RELEASE}/Debug
    NO_DEFAULT_PATH
  )
  find_library(LIB_GTEST_MAIN gtest_main PATHS 
    ${CMAKE_BINARY_DIR}/googletest-release-${GTEST_RELEASE}
    ${CMAKE_BINARY_DIR}/googletest-release-${GTEST_RELEASE}/Debug
    NO_DEFAULT_PATH
  )
  
  target_link_libraries(${target} ${LIB_GTEST} ${LIB_GTEST_MAIN})
  
endmacro(googletest)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${PostgreSQL_INCLUDE_DIRS})
file(GLOB SOURCE_FILES ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB INCLUDE_FILES ${CMAKE_CURRENT_LIST_DIR}/include/*.h)
source_group("include" FILES ${INCLUDE_FILES})

#
# postgres-test (Unit Testing)
#
macro(postgres_test target)
  file(GLOB TEST_FILES ${CMAKE_CURRENT_LIST_DIR}/test/*.cpp)
  source_group("test" FILES ${TEST_FILES})
  add_executable(${target} ${TEST_FILES} ${INCLUDE_FILES} ${SOURCE_FILES})
  target_link_libraries(${target} "${PostgreSQL_LIBRARIES}")
  googletest(${target})
endmacro(postgres_test)

#
# postgres-example
#
macro(postgres_example target)
  file(GLOB EXAMPLES_FILES ${CMAKE_CURRENT_LIST_DIR}/example/*.cpp)
  source_group("example" FILES ${EXAMPLES_FILES})
  add_executable(${target} ${EXAMPLES_FILES} ${INCLUDE_FILES} ${SOURCE_FILES})
  target_link_libraries(${target} "${PostgreSQL_LIBRARIES}")
  googletest(${target})
endmacro(postgres_example)

postgres_test(postgres-test)
postgres_example(postgres-example)
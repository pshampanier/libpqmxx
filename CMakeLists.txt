cmake_minimum_required (VERSION 3.4.1)
project (postgres-test)

# C++ standard 11
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(CMAKE_XCODE_ATTRIBUTE_ENABLE_TESTABILITY "YES")
endif()

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /source-charset:utf-8 /execution-charset:utf-8")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:msvcrt.lib")
elseif (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

# Configure Boost
set(Boost_DEBUG OFF)
set(Boost_NO_SYSTEM_PATHS        ON)
set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     OFF)
find_package(Boost 1.61.0 EXACT REQUIRED COMPONENTS system date_time regex)

# Add boost to the include path.
include_directories(${Boost_INCLUDE_DIRS})

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # Specify the minimum version to be supported by the library (Windows 7)
  # see https://msdn.microsoft.com/en-us/library/6sehtctf.aspx
  add_definitions(-D_WIN32_WINNT=0x0601)
endif()

# Configure Googletest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Configure libpq
find_package(PostgreSQL REQUIRED)
include_directories(${PostgreSQL_INCLUDE_DIRS})
link_directories(${PostgreSQL_LIBRARY_DIRS})

# Configure libpqmxx
include("libpqmxx.cmake")
include_directories(${LIBPQMXX_INCLUDE_DIRS})

#
# postgres-test (Unit Testing)
#
macro(postgres_test target)
  file(GLOB TEST_FILES ${CMAKE_CURRENT_LIST_DIR}/test/*.cpp)
  add_executable(${target} ${TEST_FILES} ${INCLUDE_FILES} ${SOURCE_FILES})
  target_link_libraries(${target} ${GTEST_LIBRARIES} ${LIBPQMXX_LIBRARIES} ${PostgreSQL_LIBRARIES})
  target_link_libraries(${target} ${Boost_SYSTEM_LIBRARY} ${Boost_DATE_TIME_LIBRARY} ${Boost_REGEX_LIBRARY})
endmacro(postgres_test)

#
# postgres-example
#
macro(postgres_example target)
  file(GLOB EXAMPLES_FILES ${CMAKE_CURRENT_LIST_DIR}/example/*.cpp)
  add_executable(${target} ${EXAMPLES_FILES} ${INCLUDE_FILES} ${SOURCE_FILES})
  target_link_libraries(${target} ${LIBPQMXX_LIBRARIES} ${PostgreSQL_LIBRARIES})
endmacro(postgres_example)

postgres_test(postgres-test)
postgres_example(postgres-example)

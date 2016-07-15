cmake_minimum_required (VERSION 3.4.1)
project (postgres-test)

# C++ standard 11
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(CMAKE_XCODE_ATTRIBUTE_ENABLE_TESTABILITY "YES")
endif()

find_package(PostgreSQL REQUIRED)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(PostgreSQL_LIBRARIES "${PostgreSQL_ROOT}/lib/${PostgreSQL_LIBRARIES}" ws2_32 Secur32)
endif()
message("PostgreSQL_LIBRARIES: ${PostgreSQL_LIBRARIES}")

# Boost
set(Boost_DEBUG                 OFF)
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost COMPONENTS system)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

if(MSVC)
  set(CMAKE_CXX_FLAGS                   "${CMAKE_CXX_FLAGS} /GR")
  set(CMAKE_CXX_FLAGS_DEBUG             "${CMAKE_CXX_FLAGS_DEBUG} /D_DEBUG /Ob0 /Od /RTC1 /MTd")
  set(CMAKE_C_FLAGS_DEBUG               "${CMAKE_C_FLAGS_DEBUG} /D_DEBUG /Ob0 /Od /RTC1 /MTd")
  set(CMAKE_C_FLAGS_RELEASE             "${CMAKE_C_FLAGS_RELEASE} /Zi /O2 /Ob2 /MT /D NDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE           "${CMAKE_CXX_FLAGS_RELEASE} /Zi /O2 /Ob2 /MT /D NDEBUG")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE    "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO      "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MT")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO    "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
endif()

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
  target_link_libraries(${target} ${GTEST_BOTH_LIBRARIES})
endmacro(postgres_test)

#
# postgres-example
#
macro(postgres_example target)
  file(GLOB EXAMPLES_FILES ${CMAKE_CURRENT_LIST_DIR}/example/*.cpp)
  source_group("example" FILES ${EXAMPLES_FILES})
  add_executable(${target} ${EXAMPLES_FILES} ${INCLUDE_FILES} ${SOURCE_FILES})
  target_link_libraries(${target} "${PostgreSQL_LIBRARIES}")
  target_link_libraries(${target} ${GTEST_BOTH_LIBRARIES})
endmacro(postgres_example)

postgres_test(postgres-test)
postgres_example(postgres-example)

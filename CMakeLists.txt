cmake_minimum_required(VERSION 3.1.0)
#Add custom cmake scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CMAKE_CXX_STANDARD 11)

project(mad_udf)
find_package(MySQL REQUIRED)
include_directories(${MYSQL_INCLUDE_DIR})
link_directories(${MYSQL_LIB_DIR})

include_directories("include")
ADD_LIBRARY(mad_udf MODULE src/mad_udf.cpp src/median_udf.cpp)

if(${MYSQL_LIB_DIR} STREQUAL "/usr/lib")
  install(TARGETS mad_udf LIBRARY DESTINATION "${MYSQL_LIB_DIR}/mysql/plugin" COMPONENT libraries)
else()
  install(TARGETS mad_udf LIBRARY DESTINATION "${MYSQL_LIB_DIR}/plugin" COMPONENT libraries)
endif()

project(quantile_udf)
ADD_LIBRARY(quantile_udf MODULE src/quantile_udf.cpp)

if(${MYSQL_LIB_DIR} STREQUAL "/usr/lib")
  install(TARGETS quantile_udf LIBRARY DESTINATION "${MYSQL_LIB_DIR}/mysql/plugin" COMPONENT libraries)
else()
  install(TARGETS quantile_udf LIBRARY DESTINATION "${MYSQL_LIB_DIR}/plugin" COMPONENT libraries)
endif()

project(outliers_udf)
ADD_LIBRARY(outliers_udf MODULE src/outliers_udf.cpp)

if(${MYSQL_LIB_DIR} STREQUAL "/usr/lib")
  install(TARGETS outliers_udf LIBRARY DESTINATION "${MYSQL_LIB_DIR}/mysql/plugin" COMPONENT libraries)
else()
  install(TARGETS outliers_udf LIBRARY DESTINATION "${MYSQL_LIB_DIR}/plugin" COMPONENT libraries)
endif()

# Unit Tests
enable_testing()
macro (create_test target)
    add_test (NAME ${target} COMMAND $<TARGET_FILE:${target}>)
endmacro (create_test)

project(mad_tests)
ADD_EXECUTABLE(mad_tests tests/mad_test.cpp)
include_directories("include")
create_test(mad_tests)

project(outlier_tests)
ADD_EXECUTABLE(outlier_tests tests/outliers_test.cpp)
include_directories("include")
create_test(outlier_tests)

project(quantile_tests)
ADD_EXECUTABLE(quantile_tests tests/quantile_test.cpp)
include_directories("include")
create_test(quantile_tests)

if(COVERAGE)
    # Set CodeCoverage
    include(CodeCoverage)

    setup_target_for_coverage(mad_tests_coverage mad_tests "coverage")
    setup_target_for_coverage(quantile_tests_coverage quantile_tests "coverage")
    setup_target_for_coverage(outlier_tests_coverage outlier_tests "coverage")
endif()

##use c++11

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -fprofile-arcs -ftest-coverage -O0 -coverage")

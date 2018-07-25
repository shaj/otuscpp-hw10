cmake_minimum_required(VERSION 3.2)

set(PRJ_NAME bulkmt)

set(PATCH_VERSION 1)
if($ENV{TRAVIS_BUILD_NUMBER})
  set(PATCH_VERSION $ENV{TRAVIS_BUILD_NUMBER})
endif()

project(${PRJ_NAME} VERSION 0.0.${PATCH_VERSION})

set(Boost_USE_STATIC_LIBS   ON)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
find_package(Boost COMPONENTS program_options REQUIRED)
find_package(Threads)

add_executable(${PRJ_NAME} main.cpp bulk.cpp tp.cpp)

set_target_properties(${PRJ_NAME} PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  COMPILE_OPTIONS -Wpedantic -Wall -Wextra
  INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
)

target_link_libraries(${PRJ_NAME}
  ${Boost_LIBRARIES}          # program_options
  ${CMAKE_THREAD_LIBS_INIT}   # spdlog
)

if((NOT CMAKE_BUILD_TYPE) OR (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
  target_compile_definitions(${PRJ_NAME} PUBLIC SPDLOG_TRACE_ON)
  message("SET DEBUG SPDLOG_TRACE_ON")
endif()

message(${CMAKE_BUILD_TYPE})

install(TARGETS ${PRJ_NAME} RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT kolobrod2002@yandex.ru)

include (CPack)

configure_file("${PROJECT_SOURCE_DIR}/version.h.in" "${PROJECT_BINARY_DIR}/version.h")
target_include_directories(${PRJ_NAME} PRIVATE "${PROJECT_BINARY_DIR}")
target_include_directories(${PRJ_NAME} PRIVATE spdlog/include)

configure_file("${PROJECT_SOURCE_DIR}/bintray.json.in" "${PROJECT_BINARY_DIR}/bintray.json")

configure_file("${PROJECT_SOURCE_DIR}/tests/t1.txt" "${PROJECT_BINARY_DIR}/t1.txt" COPYONLY)
configure_file("${PROJECT_SOURCE_DIR}/tests/t2.txt" "${PROJECT_BINARY_DIR}/t2.txt" COPYONLY)
configure_file("${PROJECT_SOURCE_DIR}/tests/t3.txt" "${PROJECT_BINARY_DIR}/t3.txt" COPYONLY)
configure_file("${PROJECT_SOURCE_DIR}/tests/t4.txt" "${PROJECT_BINARY_DIR}/t4.txt" COPYONLY)

enable_testing()

add_subdirectory(tests)

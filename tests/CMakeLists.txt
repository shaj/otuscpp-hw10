
find_package(Boost COMPONENTS unit_test_framework REQUIRED)
find_package (Threads)

add_executable(test_func test_func.cpp ../bulk.cpp)

add_executable(test_logic test_logic.cpp ../bulk.cpp)


set_target_properties(test_func test_logic PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  COMPILE_DEFINITIONS BOOST_LOG_DYN_LINK
  COMPILE_OPTIONS -Wpedantic -Wall -Wextra
  INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
)

target_include_directories(test_func PRIVATE ..)
target_include_directories(test_logic PRIVATE ..)
target_include_directories(test_func PRIVATE ../spdlog/include)
target_include_directories(test_logic PRIVATE ../spdlog/include)

target_link_libraries(test_func
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
)

target_link_libraries(test_logic
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
)


add_test(test_func test_func)
add_test(test_logic test_logic)


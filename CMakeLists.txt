cmake_minimum_required(VERSION 3.20)
project(janus_noise VERSION 0.1.0 LANGUAGES CXX)

add_library(janus_core core/src/config.cpp)
target_include_directories(janus_core PUBLIC core/include)
target_compile_features(janus_core PUBLIC cxx_std_20)
set_target_properties(janus_core PROPERTIES CXX_EXTENSIONS OFF)

include(CTest)
if(BUILD_TESTING)
  add_executable(contract_checks tests/contract_checks.cpp)
  target_link_libraries(contract_checks PRIVATE janus_core)
  add_test(NAME core_contracts COMMAND contract_checks)
endif()

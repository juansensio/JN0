add_library(janus_godot SHARED src/janus_game.cpp src/register_types.cpp)
target_include_directories(janus_godot PRIVATE src)
target_link_libraries(janus_godot PRIVATE godot-cpp janus_core)
target_compile_features(janus_godot PRIVATE cxx_std_20)
set_target_properties(janus_godot PROPERTIES
  CXX_EXTENSIONS OFF
  POSITION_INDEPENDENT_CODE ON
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/client/bin"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/client/bin"
)

#include "janus_game.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_janus_module(ModuleInitializationLevel level) {
    if (level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        GDREGISTER_CLASS(JanusGame);
    }
}

void uninitialize_janus_module(ModuleInitializationLevel) {}

extern "C" {
GDExtensionBool GDE_EXPORT janus_library_init(
    GDExtensionInterfaceGetProcAddress get_proc_address,
    GDExtensionClassLibraryPtr library,
    GDExtensionInitialization *initialization
) {
    GDExtensionBinding::InitObject init_obj(get_proc_address, library, initialization);
    init_obj.register_initializer(initialize_janus_module);
    init_obj.register_terminator(uninitialize_janus_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);
    return init_obj.init();
}
}

.PHONY: configure build test play godot mac open

configure:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON

build:
	cmake --build build --parallel

test:
	ctest --test-dir build --output-on-failure

play:
	./build/janus_cli play --seed 42 --bot heuristic

godot:
# 	godot --headless --path client --quit
	godot --editor --path client

mac:
	cmake -S . -B build/godot-release \
		-DCMAKE_BUILD_TYPE=Release \
		-DJANUS_BUILD_GODOT=ON \
		-DGODOTCPP_TARGET=template_release \
		-DBUILD_TESTING=OFF
	cmake --build build/godot-release --parallel
	godot --headless --editor --path client --import
	godot --path client

open:
	open "client/dist/macos/Janus Noise.app"
	# Para distribuir públicamente fuera de la Mac App Store, eventualmente añadiremos Developer ID + notarización; Apple recomienda precisamente ese flujo para distribución macOS externa.
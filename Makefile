.PHONY: configure build test play godot

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
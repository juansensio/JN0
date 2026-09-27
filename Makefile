configure:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON

build:
	cmake --build build --parallel

test:
	ctest --test-dir build --output-on-failure
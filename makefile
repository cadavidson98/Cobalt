tests: build/gcc-debug
	cmake --build build/gcc-debug --target CobaltTests

debug: tests
	gdb -tui build/gcc-debug/cobalt/tests/CobaltTests
tests: build/gcc-debug
	cmake --build build/gcc-debug --target CobaltTests

debug: tests
	gdb -tui build/gcc-debug/cobalt/tests/CobaltTests

coverage:
	cmake --preset=gcc-unit-testing
	cmake --build build/gcc-unit-testing --target CobaltTests
	mkdir build/gcc-unit-testing/coverage-report || true

coverage-baseline: coverage
	lcov --initial --capture --directory build/gcc-unit-testing --output-file build/gcc-unit-testing/baseline.info --ignore-errors mismatch

coverage-report: coverage-baseline
	build/gcc-unit-testing/cobalt/tests/CobaltTests
	lcov --capture --directory build/gcc-unit-testing --output-file build/gcc-unit-testing/coverage.info --ignore-errors mismatch
	lcov --add-tracefile build/gcc-unit-testing/baseline.info --add-tracefile build/gcc-unit-testing/coverage.info --output-file build/gcc-unit-testing/all_coverage.info
	lcov --build-directory build/gcc-unit-testing --remove build/gcc-unit-testing/all_coverage.info "/usr/*" --output-file build/gcc-unit-testing/cobalt_coverage.info
	genhtml build/gcc-unit-testing/cobalt_coverage.info --output-directory build/gcc-unit-testing/coverage-report

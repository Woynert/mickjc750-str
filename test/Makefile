.PHONY: *
default:

clean:
	rm -rf build

test:
	premake5 ninja && ninja -C build && premake5 test

coverage:
	mkdir -p ./build/coverage/
	gcovr -r .. --html-details ./build/coverage/coverage.html

coverageClean:
	find build -name "*.profraw" -delete
	find build -name "*.gcda" -delete
	find build -name "*.gcno" -delete


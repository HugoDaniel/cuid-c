CC = clang
CFLAGS = -Weverything -Wno-poison-system-directories -Wno-padded -Werror -std=c17 -ferror-limit=1
UBSAN = -fsanitize=undefined,float-divide-by-zero,unsigned-integer-overflow,implicit-conversion

build: tests
	@echo "\nInclude the cuid.h file in your project, it has no dependencies."

.cbuild/munit.o:
	@mkdir -p .cbuild
	@$(CC) -O3 -fsanitize=undefined -x c -c tests/munit.c -o .cbuild/munit.o

.cbuild/tests: cuid.h tests/cuid_tests.h .cbuild/munit.o
	@rm -f .cbuild/tests~
	@rm -f .cbuild/cuid_tests.o~
	@$(CC) -DCUID_PURE -DCUID_IMPL -DCUID_TESTS -O $(CFLAGS) $(UBSAN) -Wno-unused-macros -Wno-unused-parameter -Wno-unused-variable -Wno-vla .cbuild/munit.o -x c cuid.h -o .cbuild/tests

tests: .cbuild/tests
	@./.cbuild/tests
	@rm -f .cbuild/tests~

clean_tests:
	@rm -f .cbuild/tests~
	@rm -f .cbuild/munit.o~

runtime_checks:
	@rm -f .cbuild/example
	@$(CC) -DCUID_PURE -DCUID_IMPL -DCUID_EXAMPLE_USAGE -O2 $(CFLAGS) $(UBSAN) -Wno-unused-parameter -Wno-unused-variable -x c cuid.h -o .cbuild/example
	@ASAN_OPTIONS=detect_leaks=1 ./.cbuild/example

debug: 
	@rm -f .cbuild/tests~
	@rm -f .cbuild/cuid_tests.o~
	@rm -f .cbuild/munit.o~
	$(CC) -O0 -g -xc -c tests/munit.c -o .cbuild/munit.o
	$(CC) -DCUID_IMPL -DCUID_TESTS -O0 -g $(CFLAGS) -Wno-unused-parameter -Wno-unused-variable -Wno-vla -xc -c cuid.h -o .cbuild/cuid_tests.o
	$(CC) -DCUID_TESTS -O0 -g .cbuild/munit.o .cbuild/cuid_tests.o -o .cbuild/tests
	lldb .cbuild/tests

typecheck:
	@opam exec frama-c -- -c11 -eva -eva-precision 1 cuid.h

clean:
	@rm -rf .cbuild/*

TEST_LIBS = -lgmock -lgtest -lgtest_main -lpthread -L/usr/lib
TEST_CXX_FLAGS = --coverage
TEST_TARGETS = result_test
TEST_DIR = test

EXAMPLE_TARGETS = example
EXAMPLE_DIR = examples

CXX_FLAGS = -std=c++17 -Iinclude
OUT_DIR = bin
COVERAGE_DIR = coverage

COVERAGE_CXX_FLAGS = -O0 --coverage

test-coverage: CXX_FLAGS += $(COVERAGE_CXX_FLAGS)

$(TEST_TARGETS): % : $(TEST_DIR)/%.cpp
	mkdir -p $(OUT_DIR)
	g++ $(CXX_FLAGS) $(TEST_CXX_FLAGS) -o $(OUT_DIR)/$@ $< $(TEST_LIBS)

$(EXAMPLE_TARGETS): % : $(EXAMPLE_DIR)/%.cpp
	mkdir -p $(OUT_DIR)
	g++ $(CXX_FLAGS) -o $(OUT_DIR)/$@ $<

test: $(TEST_TARGETS)
	./$(OUT_DIR)/$<

test-coverage: $(TEST_TARGETS)
	./$(OUT_DIR)/$<
	mkdir -p $(COVERAGE_DIR)
	mv *.gcda *.gcno $(COVERAGE_DIR)

clean:
	rm -f $(addprefix $(OUT_DIR)/, $(TEST_TARGETS))
	rmdir --ignore-fail-on-non-empty $(OUT_DIR)
	rm -f $(addprefix $(COVERAGE_DIR)/, *.gcda *.gcno)
	rmdir --ignore-fail-on-non-empty $(COVERAGE_DIR)
	
.PHONY: test test-coverage clean

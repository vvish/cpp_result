TEST_LIBS = -lgmock -lgtest -lgtest_main -lpthread -L/usr/lib
TEST_CXX_FLAGS = -std=c++17 -Iinclude
TEST_TARGETS = result_test
TEST_DIR = test
OUT_DIR = bin

$(TEST_TARGETS): % : $(TEST_DIR)/%.cpp
	mkdir -p $(OUT_DIR)
	g++ $(TEST_CXX_FLAGS) -o $(OUT_DIR)/$@ $< $(TEST_LIBS)

test: $(TEST_TARGETS)
	./$(OUT_DIR)/$<

clean:
	rm $(addprefix $(OUT_DIR)/, $(TEST_TARGETS))
	rmdir --ignore-fail-on-non-empty $(OUT_DIR)

.PHONY: test clean
# Convenience wrapper around CMake. Run these from the project root.
# NOTE: the indented lines below MUST be real TAB characters, not spaces.

# TODO: Configure below suitable with CMake
# NOTE: the VM looks images up under its own kImageDir ("images") and appends
# ".obj", so the run targets pass only the bare image name, not a path.
BUILD_DIR := out
TARGET    := executableBinary

# Default target: configure (if needed) + build.
.PHONY: all
all:
	cmake -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR)

# Delete the whole build folder.
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

# Build and then run the program.
.PHONY: run
run: # all
	./$(BUILD_DIR)/$(TARGET)

# Run the 2048 game. Build first with `make rebuild` if needed.
.PHONY: run-2048
run-2048:
	./$(BUILD_DIR)/$(TARGET) 2048

# Run the Rogue game. Build first with `make rebuild` if needed.
.PHONY: run-rogue
run-rogue:
	./$(BUILD_DIR)/$(TARGET) rogue

.PHONY: debug
debug:
	cmake -B $(BUILD_DIR) --fresh -DCMAKE_BUILD_TYPE=Debug
	cmake --build $(BUILD_DIR)

# Clean rebuild: throw away cached config, reconfigure, rebuild.
# Reconfigure the project into the out folder.
# Use this after changing the compiler.
.PHONY: rebuild
rebuild:
	cmake -B $(BUILD_DIR) --fresh
	cmake --build $(BUILD_DIR)

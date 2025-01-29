CC = g++

TARGET_EXEC ?= result

ANTLR4_GEN_DIR ?= ./generated
BUILD_DIR ?= ./build
DEBUG_DIR ?= ./debug
SRC_DIRS ?= /

SRCS := main.cpp Interpreter.cpp IRcompiler.cpp $(ANTLR4_GEN_DIR)/*.cpp
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CFLAGS ?= -I/usr/local/include/antlr4-runtime -I/home/danandla/BOTAY/yap_lazdin/noshift/generated -lantlr4-runtime

$(BUILD_DIR)/$(TARGET_EXEC): $(SRCS)
	$(MKDIR_P) $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRCS) -o $@

.PHONY: all clean run

all: $(BUILD_DIR)/$(TARGET_EXEC)

clean:
	$(RM) -r $(BUILD_DIR) $(DEBUG_DIR)

run: $(BUILD_DIR)/$(TARGET_EXEC)
	$(BUILD_DIR)/$(TARGET_EXEC) progs/sampleDbg.nshift
dbg: $(SRCS)
	$(MKDIR_P) $(DEBUG_DIR)
	cd $(DEBUG_DIR); $(CC) $(CFLAGS) -g $(addprefix ../,$(SRCS))
	gdb --args $(DEBUG_DIR)/a.out ./progs/sampleDbg.nshift
MKDIR_P ?= mkdir -p

ANTLR_COMMAND = java -Xmx500M -cp "/usr/local/lib/antlr4-4.13.3-SNAPSHOT-complete.jar:$$CLASSPATH" org.antlr.v4.Tool
GRUN_COMMAND = java -Xmx500M -cp "/usr/local/lib/antlr-4.13.3-SNAPSHOT-complete.jar:$$CLASSPATH" org.antlr.v4.gui.TestRig

gen:
	$(ANTLR_COMMAND) -o generated -Xexact-output-dir NoShift.g4 -Dlanguage=Cpp -visitor

genj:
	$(ANTLR_COMMAND) -o jgenerated -Xexact-output-dir NoShift.g4 -Dlanguage=Java -visitor
	javac jgenerated/*.java
tree:
	$(GRUN_COMMAND) NoShift prog ../progs/sample2.nshift

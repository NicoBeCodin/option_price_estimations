# Compiler
CXX = clang++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++11 -I imgui -I imgui/backends

IMGUI_SRCS = imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_widgets.cpp imgui/imgui_tables.cpp imgui/imgui_demo.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp

# Source files
SRCS = main.cpp functions.cpp $(IMGUI_SRCS)

# Object files
OBJS = $(SRCS:%.cpp=%.o) 
#main.o functions.o imgui.o imgui_draw.o imgui_widgets.o imgui_tables.o imgui_demo.o imgui_impl_glfw.o imgui_impl_opengl3.o
#$(patsubst %.cpp,%.o,$(notdir $(SRCS)))



VPATH = imgui backends

# Executable name
EXEC = option_pricing

LIBS = -lglfw -lGL -ldl -lpthread -lX11

# Default target
all: $(EXEC)

# Linking the object files to create the executable
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Compiling source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(EXEC)

# Phony targets
.PHONY: all clean

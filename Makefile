CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
PLUGIN_FLAGS = -std=c++17 -Wall -Wextra -pedantic -fPIC
TARGET = encryption_rgr
PLUGIN_DIR = plugins
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
	PLUGIN_EXT = dylib
	SHARED_FLAG = -dynamiclib
	DL_LIBS =
else
	PLUGIN_EXT = so
	SHARED_FLAG = -shared
	DL_LIBS = -ldl
endif

MAIN_OBJECTS = main.o PluginLoader.o FileUtils.o
PLUGINS = \
	$(PLUGIN_DIR)/librabin.$(PLUGIN_EXT) \
	$(PLUGIN_DIR)/libmassey_omura.$(PLUGIN_EXT) \
	$(PLUGIN_DIR)/libchacha20.$(PLUGIN_EXT) \
	$(PLUGIN_DIR)/librsa.$(PLUGIN_EXT) \
	$(PLUGIN_DIR)/libshamir.$(PLUGIN_EXT) \
	$(PLUGIN_DIR)/liba1ya33.$(PLUGIN_EXT)

all: $(PLUGIN_DIR) $(PLUGINS) $(TARGET)

$(PLUGIN_DIR):
	mkdir -p $(PLUGIN_DIR)

$(TARGET): $(MAIN_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(MAIN_OBJECTS) $(DL_LIBS)

$(PLUGIN_DIR)/librabin.$(PLUGIN_EXT): Rabin.cpp MathUtils.cpp
	$(CXX) $(PLUGIN_FLAGS) $(SHARED_FLAG) -o $@ Rabin.cpp MathUtils.cpp

$(PLUGIN_DIR)/libmassey_omura.$(PLUGIN_EXT): MasseyOmura.cpp MathUtils.cpp
	$(CXX) $(PLUGIN_FLAGS) $(SHARED_FLAG) -o $@ MasseyOmura.cpp MathUtils.cpp

$(PLUGIN_DIR)/libchacha20.$(PLUGIN_EXT): ChaCha20.cpp MathUtils.cpp
	$(CXX) $(PLUGIN_FLAGS) $(SHARED_FLAG) -o $@ ChaCha20.cpp MathUtils.cpp

$(PLUGIN_DIR)/librsa.$(PLUGIN_EXT): RSA.cpp MathUtils.cpp
	$(CXX) $(PLUGIN_FLAGS) $(SHARED_FLAG) -o $@ RSA.cpp MathUtils.cpp

$(PLUGIN_DIR)/libshamir.$(PLUGIN_EXT): Shamir.cpp MathUtils.cpp
	$(CXX) $(PLUGIN_FLAGS) $(SHARED_FLAG) -o $@ Shamir.cpp MathUtils.cpp

$(PLUGIN_DIR)/liba1ya33.$(PLUGIN_EXT): A1Ya33.cpp MathUtils.cpp
	$(CXX) $(PLUGIN_FLAGS) $(SHARED_FLAG) -o $@ A1Ya33.cpp MathUtils.cpp

clean:
	rm -f *.o $(TARGET)
	rm -rf $(PLUGIN_DIR)

.PHONY: all clean

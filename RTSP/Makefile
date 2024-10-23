CUDA_VER?=10.2
ifeq ($(CUDA_VER),)
		$(error "CUDA_VER is not set")
endif

APP := deepstream-test3-app
TARGET_DEVICE = $(shell gcc -dumpmachine | cut -f1 -d -)
NVDS_VERSION := 6.0
LIB_INSTALL_DIR ?= /opt/nvidia/deepstream/deepstream-$(NVDS_VERSION)/lib/
APP_INSTALL_DIR ?= /opt/nvidia/deepstream/deepstream-$(NVDS_VERSION)/bin/

# Use g++ for C++ code instead of gcc
CXX := g++

ifeq ($(TARGET_DEVICE),aarch64)
		CFLAGS := -DPLATFORM_TEGRA
endif

SRCS := $(wildcard *.cpp)
INCS := $(wildcard *.h)
PKGS := gstreamer-1.0
OBJS := $(SRCS:.cpp=.o)

CFLAGS += -I../../../includes \
					-I /usr/local/cuda-$(CUDA_VER)/include
CFLAGS += $(shell pkg-config --cflags $(PKGS))

LIBS := $(shell pkg-config --libs $(PKGS))
LIBS += -L/usr/local/cuda-$(CUDA_VER)/lib64/ -lcudart -lnvdsgst_helper -lm \
				-L$(LIB_INSTALL_DIR) -lnvdsgst_meta -lnvds_meta \
				-lcuda -Wl,-rpath,$(LIB_INSTALL_DIR)

CFLAGS += -I/opt/nvidia/deepstream/deepstream-$(NVDS_VERSION)/sources/includes
LIBS += -L/opt/nvidia/deepstream/deepstream-$(NVDS_VERSION)/lib

# Compile C++ files using g++
all: $(APP)

%.o: %.cpp $(INCS) Makefile
		$(CXX) -c -o $@ $(CFLAGS) $<

$(APP): $(OBJS) Makefile
		$(CXX) -o $(APP) $(OBJS) $(LIBS)

install: $(APP)
		cp -rv $(APP) $(APP_INSTALL_DIR)

clean:
		rm -rf $(OBJS) $(APP)
CC := g++
NVCC := /usr/local/cuda/bin/nvcc
PROG := run
CONFIG_PROG := config/run
SRC_DIR := src
OBJ_DIR := obj
INC_DIR := include
CUDA_PATH := /usr/local/cuda
ARCH_NUM = $(shell nvidia-smi --query-gpu=compute_cap --format=csv -i 0 | tail -n 1 | sed 's/\.//')
ifeq ($(ARCH_NUM), )
	ARCH := sm_80
else
	ARCH := sm_$(ARCH_NUM)
endif

DEBUG ?= 0

ifeq ($(DEBUG), 1)
CFLAGS    = -g -O0 -I$(INC_DIR) -std=c++20 -fopenmp -MMD -MP
NVCCFLAGS = -g -G -O0 -I$(INC_DIR) -std=c++20 -I$(CUDA_PATH)/samples/Common -I$(CUDA_PATH)/samples/common/inc -I$(CUDA_PATH)/include -arch $(ARCH) -dc -MMD -MP
LDFLAGS   = -lcublas -lcusparse -lcudart -lstdc++ -L$(CUDA_PATH)/lib64 -arch $(ARCH) -lcudss -lhdf5_cpp -lhdf5
else
CFLAGS    = -O3 -I$(INC_DIR) -std=c++20 -fopenmp -MMD -MP
NVCCFLAGS = -O3 -I$(INC_DIR) -std=c++20 -I$(CUDA_PATH)/samples/Common -I$(CUDA_PATH)/samples/common/inc -I$(CUDA_PATH)/include -arch $(ARCH) -dc -MMD -MP
LDFLAGS   = -lcublas -lcusparse -lcudart -lstdc++ -L$(CUDA_PATH)/lib64 -arch $(ARCH) -lcudss -lhdf5_cpp -lhdf5
endif

SRCS_CU  := $(wildcard $(SRC_DIR)/*.cu)
SRCS_CPP := $(wildcard $(SRC_DIR)/*.cpp)
OBJS_CU  := $(notdir $(SRCS_CU:.cu=.o))
OBJS_CPP := $(notdir $(SRCS_CPP:.cpp=.o))
OBJS     := $(OBJS_CU) $(OBJS_CPP)
OBJSDIR  := $(addprefix $(OBJ_DIR)/, $(OBJS))
CONFIG_OBJS := make_config_data.o spline.o domain.o init.o
CONFIG_OBJSDIR := $(addprefix $(OBJ_DIR)/, $(CONFIG_OBJS))
INCLUDE := $(wildcard $(INC_DIR)/*.h)

all: $(PROG)

sim: $(PROG)
	./$(PROG)

$(PROG): $(OBJSDIR)
	$(NVCC) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cu
	@mkdir -p $(OBJ_DIR)
	$(NVCC) $(NVCCFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: config/%.cu
	@mkdir -p $(OBJ_DIR)
	$(NVCC) $(NVCCFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: config/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(CONFIG_PROG) : $(CONFIG_OBJSDIR)
	$(NVCC) -o $@ $^ $(LDFLAGS)

config: $(CONFIG_PROG)
	rm -rf ./config/image/*.dat
	./$(CONFIG_PROG)
# 	python ./python/plot_map.py ./config/image
	@echo "**\(^_^)/** Config data has been created **\(^_^)/**"

clean:
	rm -f $(PROG) $(OBJ_DIR)/*.o $(OBJ_DIR)/*.d core *.ptx ./log/*.out

clean-result:
	rm -rf results
	rm -rf log

clean-config:
	rm -f config/*.json config/*.bin config/image/*.dat config/image/*.png

clean-all: clean clean-result

csan: all
	@echo "Running compute-sanitizer (memcheck). Build with make DEBUG=1 first for best stack info."
	compute-sanitizer --tool memcheck ./$(PROG)

debug: clean
	$(MAKE) DEBUG=1 csan

-include $(OBJSDIR:.o=.d)
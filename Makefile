NVCCFLAGS = -arch=sm_86  # Ampere (tested on RTX 3070)
CXXFLAGS = -std=c++20
LDFLAGS = -lcudart

BIN_DIR = bin
OBJ_DIR = obj
SRC_DIR = src

BIN = $(BIN_DIR)/bingo
SRCS = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*.cu)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter %.cpp, $(SRCS))) $(patsubst $(SRC_DIR)/%.cu,$(OBJ_DIR)/%.o,$(filter %.cu, $(SRCS)))

# Detect OS
ifeq ($(OS),Windows_NT)
	MKDIR = mkdir
	RM = rmdir /s /q
	PYTHON = py -3.11 -m
	ACTIVATE = .\venv\Scripts\activate.bat
	PIP = .\venv\Scripts\pip.exe
	VENV_PYTHON = .\venv\Scripts\python.exe
else
	MKDIR = mkdir -p
	RM = rm -rf
	PYTHON = python3 -m
	ACTIVATE = source ./venv/bin/activate
	PIP = ./venv/bin/pip
	VENV_PYTHON = ./venv/bin/python
endif

all: $(BIN)

$(BIN): $(OBJS)
	nvcc -O3 -I include -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	nvcc -O3 -I include $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cu
	nvcc -O3 -I include $(NVCCFLAGS) -c $< -o $@

init:
	$(MKDIR) $(BIN_DIR)
	$(MKDIR) $(OBJ_DIR)
	$(PYTHON) venv venv
	$(ACTIVATE)
	$(PIP) install -r requirements.txt

analyze:
	$(VENV_PYTHON) analyze.py

clean:
	$(RM) $(OBJ_DIR) $(BIN_DIR) venv

.PHONY: all init analyze clean

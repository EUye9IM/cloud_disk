CXX			= g++
CXX_FLAGS	= -Wall -Werror -O2 -std=c++11
EXTRA_NAME	= cpp

SRC			= src
INCLUDE		= include
BUILD		= build

EXECUTABLE	= clouddiskd.exe

LIBS		= -lmariadb -lpthread


OBJS		= $(patsubst $(SRC)/%.cpp,$(BUILD)/%.o,$(wildcard $(SRC)/*.cpp  $(SRC)/*/*.cpp $(SRC)/*/*/*.cpp $(SRC)/*/*/*/*.cpp))

ifeq ($(OS),Windows_NT)
# PLATFORM="Windows" 
RM			= del /s /q /f 
else
#  PLATFORM="Unix-Like"
RM			= rm -f
endif
ifeq ($(OS),Windows_NT)
# PLATFORM="Windows" 
RMDIR		= rmdir /s /q
else
#  PLATFORM="Unix-Like"
RMDIR		= rm -rf
endif

.PHONY: all run clean


all:$(EXECUTABLE)
	@echo Success.


run: all
	@./$(EXECUTABLE)


$(EXECUTABLE):$(OBJS)
	@echo Linking $@

ifeq ($(OS),Windows_NT)
	@if not exist $(dir $@) mkdir "$(dir $@)"
else
#  PLATFORM="Unix-Like"
	@mkdir -p $(dir $@)
endif
	@$(CXX) $(CXX_FLAGS) -I$(INCLUDE) $^ $(LIBS) -o $@


build/%.o: src/%.cpp 
	@echo Compiling $@

ifeq ($(OS),Windows_NT)
	@if not exist $(dir $@) mkdir "$(dir $@)"
else
#  PLATFORM="Unix-Like"
	@mkdir -p $(dir $@)
endif
	@$(CXX) $(CXX_FLAGS) -I$(INCLUDE) -c $< -o $@


clean:
	@echo remove $(BUILD)
	@$(RMDIR) $(BUILD)
	@echo remove $(EXECUTABLE)
	@$(RM) $(EXECUTABLE)





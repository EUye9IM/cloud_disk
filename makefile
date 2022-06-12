CXX			= g++
CXX_FLAGS	= -Wall -Werror -O2 -std=c++11
EXTRA_NAME	= cpp

SRC			= src
INCLUDE		= include
BUILD		= build

EXECUTABLE	= clouddiskd.exe


OBJS		= $(patsubst $(SRC)/%.cpp,$(BUILD)/%.o,$(wildcard $(SRC)/*.cpp  $(SRC)/*/*.cpp $(SRC)/*/*/*.cpp $(SRC)/*/*/*/*.cpp))

RM			=
ifeq ($(OS),Windows_NT)
# PLATFORM="Windows" 
RM			= del /s /q /f 
else
#  PLATFORM="Unix-Like"
RM			= rm -f
endif
RMDIR		=
ifeq ($(OS),Windows_NT)
# PLATFORM="Windows" 
RMDIR		= rmdir /s /q
else
#  PLATFORM="Unix-Like"
RMDIR		= rm -rf
endif

.PHONY: all run clean


all:$(EXECUTABLE)
	@cls
	@echo Success.


run: all
	@cls
	@./$(EXECUTABLE)


$(EXECUTABLE):$(OBJS)
	@cls
	@echo Linking $@

# ifeq ($(OS),Windows_NT)
# 	if not exist $(dir $@) mkdir "$(dir $@)"
# else
# #  PLATFORM="Unix-Like"
# 	@mkdir -p $(dir $@)
# endif
	@$(CXX) $(CXX_FLAGS) -I$(INCLUDE) $^ -o $@


build/%.o: src/%.cpp 
	@cls
	@echo Compiling...
	@echo $@
	@echo [$(foreach i,$(OBJS), $(if $(findstring $@, $(i)),*))]

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





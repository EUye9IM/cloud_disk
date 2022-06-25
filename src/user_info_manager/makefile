CXX			= g++ 
CXX_FLAGS	= -Wall -Werror -O2 -std=c++11

INCLUDE		= ../../include

EXECUTABLE	= test

LIBS		= -lmariadb

SRCS		= $(wildcard *.cpp)
OBJS		= $(patsubst %.cpp,%.o,$(SRCS))

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
	@echo Success.


run: all
	@./$(EXECUTABLE)


$(EXECUTABLE):$(OBJS)
	@echo Linking $@
	@$(CXX) $(CXX_FLAGS) -I$(INCLUDE) $^ $(LIBS) -o $@


%.o: %.cpp 
	@echo Compiling $@
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) -c $< -o $@


clean:
	@echo remove $(OBJS)
	@$(RM) $(OBJS)
	@echo remove $(EXECUTABLE)
	@$(RM) $(EXECUTABLE)


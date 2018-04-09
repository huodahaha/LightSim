CPP = g++
INC_PATH = -I./
# 带有外部Include时添加
# CPPFLAGS = -pg -g -Wall $(INC_PATH)
CPPFLAGS = -pg -g -Wall -std=c++11
RM = rm -rf
SVR_SRC = $(wildcard *.cpp)
SVR_OBJ = $(addprefix ./,$(subst .cpp,.o,$(SVR_SRC)))

AR = ar -curs
TARGET = out 
LDFLAGS = -g

.PHONY: all clean

all : $(TARGET)

$(TARGET) : $(SVR_OBJ)
	$(CPP) $(LDFLAGS) -o $@ $(SVR_OBJ) $(CPPFLAGS)

%.o : %.cpp
	$(CPP) $(CPPFLAGS) -o $@ -c $<
clean:
	$(RM) $(SVR_OBJ) $(TARGET)

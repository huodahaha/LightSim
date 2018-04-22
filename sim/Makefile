CPP = g++
INC_PATH = -I../include
CPPFLAGS = -pg -g -Wall -std=c++11 $(INC_PATH)
RM = rm -rf
SVR_SRC = $(wildcard *.cpp)
SVR_OBJ = $(addprefix ./,$(subst .cpp,.o,$(SVR_SRC)))
LIB_OBJ:= $(filter-out ./unit_test.o ./light_sim.o, $(SVR_OBJ))

AR = ar -crv
TEST_TARGET = unittest
MAIN_TARGET = lightsim
LIB_TARGET = sim.a
LDFLAGS = -g

.PHONY: all clean

all : $(TEST_TARGET) $(MAIN_TARGET)

$(TEST_TARGET) : $(LIB_TARGET) $(SVR_OBJ)
	$(CPP) -o $@ unit_test.o $(LIB_TARGET)

$(MAIN_TARGET) : $(LIB_TARGET) $(SVR_OBJ)
	$(CPP) -o $@ light_sim.o $(LIB_TARGET)

$(LIB_TARGET) : $(LIB_OBJ)
	$(AR) $(LIB_TARGET) $(LIB_OBJ)

%.o : %.cpp
	$(CPP) $(CPPFLAGS) -o $@ -c $<
clean:
	$(RM) $(SVR_OBJ) $(TEST_TARGET) $(LIB_TARGET) $(MAIN_TARGET)

CXX = gcc

INCLUDE = libmilter
OPTION = -lmilter -pthread
OBJ_DIR = obj
SRC_DIR = src
EXE = myFilter
MAIN = add_on

all: $(OBJ_DIR) $(MAIN)
	$(CXX) -o $(EXE) $(OBJ_DIR)/$(MAIN).o -L $(INCLUDE) $(OPTION)
$(MAIN): $(OBJ_DIR) 
	$(CXX) -I $(INCLUDE) -o $(OBJ_DIR)/$(MAIN).o -c $(SRC_DIR)/$(MAIN).c

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

clean:
	rm -r $(OBJ_DIR)
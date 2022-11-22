CC = gcc

INCLUDE = libmilter
OPTION = -lmilter -pthread -lcurl
OBJ_DIR = obj
SRC_DIR = src

EXE = myFilter
MAIN = add_on
PARSING = parsing

all: $(OBJ_DIR) $(MAIN) $(PARSING)
	$(CC) -o $(EXE) $(OBJ_DIR)/$(MAIN).o $(OBJ_DIR)/$(PARSING).o -L $(INCLUDE) $(OPTION)
$(MAIN): $(OBJ_DIR) 
	$(CC) -I $(INCLUDE) -o $(OBJ_DIR)/$(MAIN).o -c $(SRC_DIR)/$(MAIN).c

$(PARSING): $(OBJ_DIR)
	$(CC) -o $(OBJ_DIR)/$(PARSING).o -c $(SRC_DIR)/$(PARSING).c

$(OBJ_DIR):
	mkdir $(OBJ_DIR)
	
clean:
	rm -r $(OBJ_DIR) $(EXE)
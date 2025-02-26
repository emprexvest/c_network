TARGET_SRV = bin/dbserver

SRC_SRV = $(wildcard src/srv/*.c)
OBJ_SRV = $(SRC_SRV:src/srv/%.c=obj/srv/%.o)

run: clean default
	./$(TARGET_SRV) -f ./mynewdb.db -n -p 8080

default: $(TARGET_SRV)

clean:
	rm -f obj/srv/*.o
	rm -f bin/*
	rm -f *.db

$(TARGET_SRV): $(OBJ_SRV) 
	gcc -o $@ $(OBJ_SRV) -Iinclude

$(OBJ_SRV): obj/srv/%.o: src/srv/%.c
	gcc -c $< -o $@ -Iinclude

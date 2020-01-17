HEADERS=tm1638.h
OBJ=main.o tm1638.o
TARGET=/usr/local/bin/display
CFLAGS=-O -w -fstack-protector -lwiringPi

all: $(TARGET)

$(TARGET): $(OBJ)
	sudo cc -o $(TARGET) $(CFLAGS) $(OBJ)
	sudo chmod +s $(TARGET)
	sudo chmod 666 /dev/mem

clean:
	sudo rm -rf $(OBJ) $(TARGET)

main.o: main.c

tm1638.o: tm1638.c $(HEADERS)

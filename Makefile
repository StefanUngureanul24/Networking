CC = gcc

TARGET: GoBackNSender GoBackNReceiver SWReceiver SWSender 

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(TARGET).c -o $(TARGET)
	
clean:
	$(RM) $(TARGET)

ifdef debug
	FLAGS += -g
endif

FLAGS = -Wall -Wextra -Wvla -std=c++11 -c

all: whatsappServer whatsappClient

whatsappServer: whatsappServer.o whatsappio.o
	$(CXX) $^ -o $@

whatsappClient: whatsappClient.o whatsappio.o
	$(CXX) $^ -o $@

whatsappServer.o: whatsappServer.cpp whatsappio.h whatsappio.cpp whatsappServer.h
	$(CXX) $(FLAGS) whatsappServer.cpp

whatsappClient.o: whatsappClient.cpp whatsappClient.h whatsappio.h whatsappio.cpp
	$(CXX) $(FLAGS) whatsappClient.cpp

whatsappio.o: whatsappio.h whatsappio.cpp
	$(CXX) $(FLAGS) whatsappio.cpp

tar: whatsappClient.cpp whatsappServer.cpp README Makefile \
whatsappio.h whatsappio.cpp whatsappServer.h whatsappClient.h
	tar -cf ex4.tar $^

clean:
	rm -f ex4.tar *.o whatsappServer whatsappClient

.PHONY: clean tar val

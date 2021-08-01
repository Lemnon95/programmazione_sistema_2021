SERVER=$(realpath .)/server
CLIENT=$(realpath .)/client
SHARE=$(realpath .)/share
DEBUG=-g -O0 -D _DEBUG
RELESE=-O2


all: compileDEBUG run
#all: compile


compileDEBUG:
	g++ -I"$(SHARE)" "$(SERVER)/server.cpp" "$(SERVER)/share_lib_server.cpp" "$(SHARE)/wrapper.cpp" -o server.out -lpthread -std=c++17 $(DEBUG)
	g++ -I"$(SHARE)" "$(CLIENT)/client.cpp" "$(CLIENT)/share_lib_client.cpp" "$(SHARE)/wrapper.cpp" -o client.out -lpthread -std=c++17 $(DEBUG)

compile:
	g++ -I"$(SHARE)" "$(SERVER)/server.cpp" "$(SERVER)/share_lib_server.cpp" "$(SHARE)/wrapper.cpp" -o server.out -lpthread -std=c++17 $(RELESE)
	g++ -I"$(SHARE)" "$(CLIENT)/client.cpp" "$(CLIENT)/share_lib_client.cpp" "$(SHARE)/wrapper.cpp" -o client.out -lpthread -std=c++17 $(RELESE)

run:
	./server.out

clean:
	rm server.out
	rm client.out

all: compile run

compile:
	g++ "$(realpath .)/windows/server/server.cpp" "$(realpath .)/lib/share_lib_server.cpp" -o server -lpthread -std=c++17
	g++ "$(realpath .)/windows/client/client.cpp" "$(realpath .)/lib/share_lib_client.cpp" -o client -lpthread -std=c++17

run:
	./server

clean:
	rm server
	rm client

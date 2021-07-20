all: compile run

compile:
	g++ "$(realpath .)/windows/server/server.cpp" "$(realpath .)/lib/share_lib_server.cpp" -o server -g -O0 -lpthread -std=c++17 -D _DEBUG
	g++ "$(realpath .)/windows/client/client.cpp" "$(realpath .)/lib/share_lib_client.cpp" -o client -g -O0 -lpthread -std=c++17 -D _DEBUG

run:
	./server

clean:
	rm server
	rm client

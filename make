gcc -Wall -g -o server.out server.cpp common.cpp server_data.cpp -pthread -lncursesw -lrt
gcc -Wall -g -o client_human.out client_human.cpp client_common.cpp common.cpp -pthread -lncursesw -lrt

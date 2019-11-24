g++ -Wall -g -o server.out server.cpp common.cpp server_data.cpp map.cpp beast.cpp independant.cpp tiles.cpp -pthread -lncursesw -lrt
g++ -Wall -g -o client_human.out client_human.cpp client_common.cpp common.cpp client_data.cpp map.cpp tiles.cpp -pthread -lncursesw -lrt
g++ -Wall -g -o client_bot.out client_bot.cpp client_common.cpp independant.cpp common.cpp client_data.cpp map.cpp tiles.cpp -pthread -lncursesw -lrt

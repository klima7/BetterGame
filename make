gcc -Wall -g -o server.out server.c common.c server_data.c -pthread -lncursesw -lrt
gcc -Wall -g -o client_human.out client_human.c client_common.c common.c client_data.c -pthread -lncursesw -lrt

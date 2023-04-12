#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define MAX_CLIENT_CNT 500
#define MAX_KEY_SIZE 100
#define MAX_VALUE_SIZE 100

char keys[100][100];
char values[100][100];

const char* PORT = "12345";
int server_sock;

int client_sock[MAX_CLIENT_CNT];
struct sockaddr_in client_addr[MAX_CLIENT_CNT];

pthread_t tid[MAX_CLIENT_CNT];
int exitFlag[MAX_CLIENT_CNT];

// mutex 선언
pthread_mutex_t mutx;

void interrupt(int arg)
{
	printf("\nYou typed Ctrl + C\n");
	printf("Bye\n");

	for (int i = 0; i < MAX_CLIENT_CNT; i++)
	{
		if (client_sock[i] != 0)
		{
			pthread_cancel(tid[i]);
			pthread_join(tid[i], 0);
			close(client_sock[i]);
		}
	}
	close(server_sock);
	exit(1);
}

void removeEnterChar(char* buf)
{
	int len = strlen(buf);
	for (int i = len - 1; i >= 0; i--)
	{
		if (buf[i] == '\n')
			buf[i] = '\0';
		break;
	}
}

int getClientID()
{
	for (int i = 0; i < MAX_CLIENT_CNT; i++)
	{
		if (client_sock[i] == 0)
			return i;
	}
	return -1;
}

void* client_handler(void* arg)
{
	int id = *(int*)arg;

	char name[100];
	// inet_ntoa 는,빅 엔디안  long int ip 를 문자열로 바꿈
	strcpy(name, inet_ntoa(client_addr[id].sin_addr));
	printf("INFO :: Connect new Client (ID : %d, IP : %s)\n", id, name);

	// wait & write
	char buf[100];
	char tmp_buf[100];
	strcpy(tmp_buf, buf);
	while (1)
	{
		memset(buf, 0, 100);
		int len = read(client_sock[id], buf, 99);
		removeEnterChar(buf);

		if (len == 0)
		{
			printf("INFO :: Disconnect with client.. BYE\n");
			exitFlag[id] = 1;
			break;
		}

		if (!strcmp("exit", buf))
		{
			printf("INFO :: Client want close.. BYE\n");
			exitFlag[id] = 1;
			break;
		}

		char* order = strtok(tmp_buf, " ");
		char* key = strtok(NULL, "[]:"); // 없으면 NULL 반환
		char* value = strtok(NULL, "[]:"); // 없으면 NULL 반환
		
		int now = 0;
		if (!strcmp(order, "save")) { // save

			strcpy(keys[now], key);
			strcpy(values[now], value);

			now++; // 현재 save된 위치

		}

		else if (!strcmp(order, "read")) { // read

			for (int i = 0; i < MAX_KEY_SIZE; i++) {
				if (!strcmp(key, keys[i])) {
					strcpy(buf, values[i]);
				}
			}


		}

		else {
			printf("Invalid command");
		}

		// remove '\n'
		

		// send new message
		// mutex
		pthread_mutex_lock(&mutx);
		for (int i = 0; i < MAX_CLIENT_CNT; i++)
		{
			if (client_sock[i] != 0)
			{
				write(client_sock[i], buf, strlen(buf));
			}
		}
		pthread_mutex_unlock(&mutx);
	}
	close(client_sock[id]);
}

int main()
{
	// Ctrl + C 누를 경우 안전종료
	signal(SIGINT, interrupt);

	// mutex init
	pthread_mutex_init(&mutx, NULL);
	// socket create
	server_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (server_sock == -1)
	{
		printf("ERROR :: 1_Socket Create Error\n");
		exit(1);
	}

	// option setting
	// 종료 시 3분 정도 동일한 포트 배정 불가 에러 해결
	int optval = 1;
	setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&optval, sizeof(optval));

	// 주소 설정
	struct sockaddr_in server_addr = { 0 };
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(PORT));

	// bind
	if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		printf("ERROR :: 2_bind Error\n");
		exit(1);
	}

	// listen
	if (listen(server_sock, 5) == -1)
	{
		printf("ERROR :: 3_listen Error");
		exit(1);
	}

	socklen_t client_addr_len = sizeof(struct sockaddr_in);

	// pthread argument bug fix
	int id_table[MAX_CLIENT_CNT];
	printf("Wait for next client...\n");

	while (1)
	{
		// get Client ID
		int id = getClientID();
		id_table[id] = id;

		if (id == -1)
		{
			printf("WARNING :: Client FULL\n");
			sleep(1);
		}

		// 새로운 클라이언트를 위해 초기화
		memset(&client_addr[id], 0, sizeof(struct sockaddr_in));

		// accpet
		client_sock[id] = accept(server_sock, (struct sockaddr*)&client_addr[id], &client_addr_len);
		if (client_sock[id] == -1)
		{
			printf("ERROR :: 4_accept Error\n");
			break;
		}

		// Create Thread
		pthread_create(&tid[id], NULL, client_handler, (void*)&id_table[id]);

		// check ExitFlag
		for (int i = 0; i < MAX_CLIENT_CNT; i++)
		{
			if (exitFlag[i] == 1)
			{
				exitFlag[i] = 0;
				pthread_join(tid[i], 0);
				client_sock[i] = 0;
			}
		}
	}
	// 서버 소켓 close
	close(server_sock);
	return 0;
}

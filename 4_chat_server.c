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
#define PORT_SIZE 6

char PORT[PORT_SIZE];
int server_sock;

int client_sock[MAX_CLIENT_CNT];
struct sockaddr_in client_addr[MAX_CLIENT_CNT];

pthread_t tid[MAX_CLIENT_CNT];
int exitFlag[MAX_CLIENT_CNT];

// mutex 선언
pthread_mutex_t mutx;

char key[100][100];
char value[100][100];

int idx = 0;

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
            close(client_sock[i]); // server가 interrupt를 받으면 client부터 다 내보낸다.
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

int getClientID() // 접속한 클라이언트가 어느 소켓에 들어가야 할지 위치 반환
{
    for (int i = 0; i < MAX_CLIENT_CNT; i++)
    {
        if (client_sock[i] == 0) // client_socket을 돌면서 자리가 있으면 넣는다.
            return i;
    }
    return -1; // 자리가 없으면
}

void* client_handler(void* arg) // client를 받아와서 쓰레드 함수를 분석한다.
{
    int id = *(int*)arg;

    char name[100];
    // inet_ntoa 는,빅 엔디안  long int ip 를 문자열로 바꿈
    strcpy(name, inet_ntoa(client_addr[id].sin_addr));
    printf("INFO :: Connect new Client (ID : %d, IP : %s)\n", id, name);

    // wait & write
    char buf[100];
    char tmp_buf[100];
    char parsing[2][100];
        
	strcpy(tmp_buf, buf);

	printf("Hi there : before NULL");

	char* p = strtok(tmp_buf, " ");
	char* key = strtok(NULL, ":");
	char* word = strtok(NULL, " ");

	printf("Hi there : global");
	printf("key : %s", key);
	printf("word : %s", word);
    
    while (1)
    {
        memset(buf, 0, 100);
        int len = read(client_sock[id], buf, 99);
        if (len == 0) // EOF
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
        
        
        
	/*
        if (check_flag)
        {
            if (!strcmp("save", parsing[0]))
            {
                int case_flag = 0;
                
                printf("Hi there : save");

                for (int i = 0; i < idx; ++i)
                {
                    if (key[i] == tmp_key)
                    {
                        strcpy(value[i], tmp_value);
                        case_flag = 1;
                        break;
                    }
                }

                if (!case_flag)
                {
                    strcpy(key[idx], tmp_key);
                    strcpy(value[idx++], tmp_value);
                }

            }

            if (!strcmp("read", parsing[0]))
            {
                int case_flag = 0;

                for (int i = 0; i < idx; ++i)
                {
                    if (key[i] == parsing[1])
                    {
                        print_flag = 1;
                        case_flag = 1;
                        break;
                    }
                }

                if (!case_flag)
                    printf("No value for this key\n");
            }
            if (!strcmp("close", parsing[0]))
                break;
        }
        */



        // buf에 client에서 온 값을 받는다
        for (int i = 0; i < len; i++) {
            if (buf[i] >= 'a' && buf[i] <= 'z')
                buf[i] -= 32;
        }


        // remove '\n'
        removeEnterChar(buf);

        // send new message
        // mutex
        pthread_mutex_lock(&mutx);
        for (int i = 0; i < MAX_CLIENT_CNT; i++)
        {
            if (client_sock[i] != 0) // 클라이언트 서버에 자리가 있으면
            {
                write(client_sock[i], buf, strlen(buf)); // buf의 내용을 Client_sock에 옮긴다.
            }
        }
        pthread_mutex_unlock(&mutx);
    }
    close(client_sock[id]);
}

int main(int argc, char* argv[])
{
    // Ctrl + C 누를 경우 안전종료
    signal(SIGINT, interrupt);

    if (argc != 2)
    {
        printf("Usage : %s <PORT>\n", argv[0]);
        exit(1);
    }

    sprintf(PORT, "%s", argv[1]);

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
        id_table[id] = id; // echo와 다른 point
        // 각 클라이언트가 어떤 id에 배정되었는지 확인 용도

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


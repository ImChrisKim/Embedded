#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

const char *PORT = "12345";

int server_sock;
int client_sock;

void interrupt(int arg) // SIGINT 발생시 호출되는 함수
{
	printf("\nYou typed Ctrl + C\n");
	printf("Bye\n");

	close(client_sock); // interrupt 발생시 client_sock를 닫는다
	close(server_sock); // interrupt 발생시 server_sock을 받는다.
	exit(1); // 종료한다.
}

void removeEnterChar(char *buf) // '\n'로 인해 영향을 받지 않기 위해 '\n'을 NULL인 '\0'으로 대체
{
	int len = strlen(buf);
	for (int i = len - 1; i >= 0; i--)
	{
		if (buf[i] == '\n')
			buf[i] = '\0';
		break;
	}
}

int main()
{
	signal(SIGINT, interrupt); // SIGINT 발생시 호출될 함수의 포인터 interrupt 호출

	// socket() : 소켓을 생성하는 시스템콜
	// 
	// socket(int domain, int type, int protocol)
	// domain : 소켓의 주소 체계 (PF_INET : IPv4 주소 체계를 사용, PF_INET6 : IPv6 주소 체계를 사용)
	// type : 소켓의 유형을 결정하는 인자 (SOCK_STREAM : TCP, SOCK_DGRAM : UDP)
	// protocol : 소켓에서 프로토콜을 선택하는 인자
	server_sock = socket(PF_INET, SOCK_STREAM, 0); // socket을 생성, IPv4주소 체계를 사용하며 TCP protocol을 채용
	if (server_sock == -1)
	{
		printf("ERROR :: 1_Socket Create Error\n");
		exit(1);
	}

	// setsockopt() : 소켓에 대한 옵션을 설정하는 함수
	// 
	// setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
	// sockfd : 옵션을 적용할 소켓의 파일 디스크립터
	//   -> socket()으로 소켓을 생성, 초기화한 소켓의 파일 디스크립터를 반환
	//   -> 디스크립터 : 소켓을 식별하는 역할을 한다.
	// level : 소켓의 프로토콜 레벨
	// optname : 옵션의 이름
	//   -> SO_REUSEADDR : TIME_WAIT 상태의 포트가 머무르는 기간 동안 다른 소켓이 해당 포트를 사용할 수 있게 해주는 옵션.
	// optval : 옵션의 값을 저장한 버퍼의 포인터
	// optlen : 버퍼의 크기

	int optval = 1; // SO_REUSEADDR을 활용하기 위한 버퍼의 포인터
	setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (void *)&optval, sizeof(optval));

	struct sockaddr_in server_addr = {0}; // server_addr의 주소체계를 만든다
	server_addr.sin_family = AF_INET;  // IPv4의 주소체계를 사용한다.
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 할당된 모든 IP주소에 대해 서버를 바인드 한다.
	server_addr.sin_port = htons(atoi(PORT)); // 입력받은 포트번호를 정수형으로 변환 후, htons() 함수를 활용하여 네트워크 바이트 순서로 변환

	// bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
	if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) // socket bind()
	{
		printf("ERROR :: 2_bind Error\n");
		exit(1);
	}

	if (listen(server_sock, 5) == -1) // listen() : 대기 모드, 2번째 인자는 대기 queue로 연결 요청의 최대 개수
	{
		printf("ERROR :: 3_listen Error");
		exit(1);
	}

	client_sock = 0;
	struct sockaddr_in client_addr = {0};
	socklen_t client_addr_len = sizeof(client_addr);

	while (1)
	{
		memset(&client_addr, 0, sizeof(client_addr));

		// accpet() : 연결 요청을 수락하고, 클라이언트와 통신할 수 있는 새로운 소켓을 반환
		// accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
		// addr : 클라이언트의 주소 정보를 저장할 구조체
		client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len); // 클라이언트 주소 정보가 기록된다
		
		// 에러 캐치
		if (client_sock == -1)
		{
			printf("ERROR :: 4_accept Error\n");
			break;
		}

		char buf[100];
		while (1)
		{
			memset(buf, 0, 100);
			int len = read(client_sock, buf, 99); // client_sock의 내용을 읽어온다

			removeEnterChar(buf); // 개행문자 제거

			if (len == 0)
			{
				printf("INFO :: Disconnect with client... BYE\n");
				break;
			}
			
			printf("%s\n",buf);
			
			memset(buf, 0, 100);
			
			fgets(buf, 100, stdin);
			
			if (!strcmp("exit", buf))
			{
				printf("INFO :: Client want close... BYE\n");
				break;
			}
			write(client_sock, buf, strlen(buf));
		}
		close(client_sock);
	}
	close(server_sock);
	return 0;
}




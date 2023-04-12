#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

const char *IP = "127.0.0.1";
const char *PORT = "12345";

int sock;

void interrupt(int arg) // SIGINT -> interrupt가 발생할 때의 호출되는 함수
{
	printf("\nYou typped Ctrl + C\n");
	printf("Bye\n");

	close(sock);
	exit(1);
}

int main()
{
	signal(SIGINT, interrupt);

	sock = socket(PF_INET, SOCK_STREAM, 0); // IPv4 IP 체계 사용 
	if (sock == -1) // 에러 발생
	{
		printf("ERROR :: 1_Socket Create Error\n");
		exit(1);
	}

	struct sockaddr_in addr = {0}; //  informations
	addr.sin_family = AF_INET; // IPv4
	addr.sin_addr.s_addr = inet_addr(IP); // IP주소를 나타내는 문자열을 32비트 정수형 숫자로 변환
	addr.sin_port = htons(atoi(PORT)); // PORT 번호를 정수형으로 변환 후 빅엔디안으로 변환

	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) // 연결 에러 발생시 (= bind)
	{
		printf("ERROR :: 2_Connect Error\n");
		exit(1);
	}

	char buf[100];
	while (1)
	{	

		memset(buf, 0, 100);
		fgets(buf, 100, stdin);
		if (!strcmp(buf, "exit")) // exit일 경우
		{
			write(sock, buf, strlen(buf));
			break;
		}
		write(sock, buf, strlen(buf)); // socket에 쓴다
		memset(buf, 0, 100);
		int len = read(sock, buf, 99); // socket에 써있는걸 불러온다
		if (len == 0)
		{
			printf("INFO :: Server Disconnected\n"); // 아무것도 안써져 있으면 끝낸다
			break;
		}
		printf("%s\n", buf); // socket 내용을 출력한다.
	}

	close(sock);
	return 0;
}


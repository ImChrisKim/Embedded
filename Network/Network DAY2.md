# Day2

## Socket
- 프로세스(실행된 APP)가 네트워크(소켓, 이라는 "파일")를 통해 (파일을 열고 읽고 쓰고 닫아)데이터를 내보내거나 받을 때 사용한다.

- 네트워스 송수신 함수를 다시 해석
- read() 사용시 -> "수신", 데이터를 받는다
- write() 사용시 -> "송신", 데이터를 보낸다

## TCP 기반 서버 소켓 동작 순서
1.  socket()
2.  bind()
3.  listen()
4.  accept()
5.  read() / write()
6.  close()

## TCP 기반 Client 소켓 동작 순서
1. socket()
2. connect()
3. read() / write()
4. close()

## 서버와 Client
- IP주소와 PORT 번호를 이용해서 통신을 한다.

## client 소켓의 종료 과정
- ctrl + c 로 종료
- exit 입력
- 서버가 문을 닫았을 때

## 멀티 쓰레드
- 멀티 프로세스 => context switching이 빈번하게 일어나므로 시스템 성능을 저하시킨다.
  - > 멀티 프로세스보다 가능한 멀티 쓰레드로 작업한다.

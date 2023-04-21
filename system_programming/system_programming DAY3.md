# 23-04-05 System programming DAY_3

## Memory leak

- 딱히 적을게 없다..

## Process

- ps -ef 로 동작중인 process 를 나열한다.
- 나중에 복습하면서 채우자..


## Context Switching

- context -> 프로세스 안의 어떤 상태 (정보)
- swiching -> 바꾼다.
  - > 일이 발생한다.
- CPU는 기본적으로 하나의 프로세스를 실행
  - 매우 빠르게 여러 프로세스를 실행 -> context switching이 일어난다
  - > 동시에 동작하는 것처럼 보인다.

### Context Switching 동작 과정
- CPU가 동작을 하면서 context switching이 발생하면 어디까지 했는지 기록을 해야 한다.
  - > 스케줄러가 context switching을 명령한다.
  - > Process가 PCB라는 공간에 기록을 한다.
  - > 다시 넘어와서 PCB를 확인하고 일을 한다.

### process 스케쥴링
- IPC : 프로세스 끼리 데이터를 공유하는 방법
- context switching이 많아지면, CPU가 동시 동작하는것처럼 보여 더 좋아 보일 수 있으나,
- 그에 따른 Cost 가 더 많이 소요된다.

### process state
- R : Running / Runnable
- S : Interruptible sleep
- D : Uninterruptible sleep
- T : Stopped
- Z : Zombie
- 
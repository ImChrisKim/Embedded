## TIME

### 필요성
- 컴퓨터를 켠다
- 시스템 클럭이 있다
  - > CPU가 이 클럭에 맞춰 일을 한다
- CPU가 일을 한다
  - 발열이 발생
  - 열을 낮추기 위해 팬이 돌아간다
- 발열이 발생하면 느려진다
  - > cpu가 느려진다
  - > 클럭이 느려진다
- CPU 내부에는 자체적인 클럭이 있다
  - (WHY) 내부 클럭을 쓰냐?
    - > 지속적인 발열 문제로 외부 클럭 사용
    - > 지속적으로 실시간성 동기화 작업이 이뤄진다.

### RTC
- 전원이 없어도 시간 계산을 한다.

### Linux
- date 명령어
  - 부팅시 RTC 정보를 받아 리눅스에서 시간 정보를 관리
- hwclock 명령어
  - HW 장치가 가지고 있는 시간 정보 값
  - > 리눅스에서는 date와 RTC가 맞지 않으면 한쪽으로 sync를 맞춰주는 명령어를 써야 한다.

### time.h


### gettimeofday

- time() -> 초 단위로 시간 측정

```c

struct tm;
localtime(time_t); // struct tm 구조체에 값이 들어간다

```

- gettimeofday() -> us 단위로 시간 측정

```c
struct tm;
gettimeofday(struct timeval, NULL) // us 단위로 시간 측정
```
# Proj1.

## 개발 목표

- 앞으로 수행할 SIC/XE머신 구현을 위한 전 단계로서 어셈블러, 링커, 로더들을 실행하게 될 셸**(shell)**과 컴파일을 통해서 만들어진 object코드가 적재되고 실행될 메모리공간, mnemonic (ADD, COMP, FLOAT, etc …)을 opcode값으로 변환하는 **OPCODE 테이블과 관련 명령어들을 구현**

## Repo Link

- [Here](https://github.com/AlphaTechnic/2021-1_system_programming_proj/tree/master/sp_proj1)

# Proj2.

## 개발 목표

- proj1에서 구현한 셸(shell)에 **assemble 기능**을 추가하는 프로그램을 만든다. 
- SIC/XE의 assembly program source 파일을 입력 받아서 object 파일을 생성하고, 어셈블리 과정 중 생성된 **symbol table**과 결과물인 **object 파일**을 볼 수 있는 기능을 제공

## Repo Link

- [Here](https://github.com/AlphaTechnic/2021-1_system_programming_proj/tree/master/sp_proj2)

# Proj3.

## 개발 목표

- 프로젝트 1, 2에서 구현한 셸에 linking과 loading 기능을 추가하는 프로그램이다.
- 프로젝트 2에서 구현한 assemble 명령을 통해서 생성된 object 파일을 link 시켜 메모리에 올리는 일을 수행한다.
- 이를 위해 4가지 기능을 구현하는데 첫째로 주소를 지정하는 명령인 `progaddr`과 linking loader 명령인 `loader`, 프로그램 실행 명령인 `run`, debug 명령인 `bp`이다.

## Repo Link

- [Here](https://github.com/AlphaTechnic/2021-1_system_programming_proj/tree/master/sp_proj3)



------------------------------------------------------------------------------

# Proj4.

## 개발 목표

- linux의 shell과 동일하게 동작하는 customized 된 myshell을 구현한다. myshell 개발을 통해 시스템 레벨에서의 process control, process signalling, interprocess communication 그리고 background job에 대해 이해한다.

## Repo Link

- [Here](https://github.com/AlphaTechnic/2021-1_system_programming_proj/tree/master/sp_proj4)

# Proj5.

## 개발 목표

- 주식 서버 프로그램을 구현한다. 단일 프로세스 혹은 thread 기반 stockserver는 여러 client와 connection이 불가능하다. 
- 해당 프로젝트에서는 concurrent network programming을 이용하여 주식 서버를 구축한다. 
- concurrent programming을 구현하기 위해서는 2가지 방식이 있는데, select()를 이용한 Event-based Approach가 있고, pthread를 이용한 Thread-based Approach가 있다. 
- 각각의 방식으로 주식 서버를 구축해보고, 각 프로그램의 수행 결과를 비교, 대조하면서 성능을 분석해본다.

## Repo Link

- [Here](https://github.com/AlphaTechnic/2021-1_system_programming_proj/tree/master/sp_proj5)


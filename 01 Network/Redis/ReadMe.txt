라이브러리 빌드 환경 : 64bit, Release
프로젝트 적용 방법
1. cpp_redis.lib, tacopie.lib 프로젝트에 추가할것.
2. 프로젝트 속성 - 구성 속성 - VC++ 디렉터리 - 일반 - 포함 디렉터리 - 해당 폴더(Redis) 경로 추가
	redis/tacopie에서 파일을 "<>"로 include 하고있음. 해당 작업 필요
NetworkLib 적용 방법.

1. NetworkLib.lib
	- 프로젝트 포함 시킬것. (위치 상관 x)

2. 모든 헤더파일 (권장)
	- 모든 헤더파일은 같은 위치에 둘것.

3. libmysql.dll // DBConnector 에서 사용함	
	- 실행 위치에 둘것. (ex. exe, vcxproj 위치)
	
4. mysql 폴더 // DBConnector 에서 사용함
	- 헤더파일 위치에 'mysql 폴더' 위치시킬것
	- mysql/lib/libmysql.lib 프로젝트 포함 시킬것. (위치 상관 x)
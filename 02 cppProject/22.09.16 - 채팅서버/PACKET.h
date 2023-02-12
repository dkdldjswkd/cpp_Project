#pragma once
#define PACKET_CODE		0x89

//------------------------------------------------------
//  패킷헤더
//
//	| PacketCode | CheckSum | MsgType | PayloadSize | * Payload * |
//		1Byte        1Byte	   2Byte      2Byte        Size Byte     
//
//	checkSum - 각 MsgType, Payload 의 각 바이트 더하기 % 256
//
//------------------------------------------------------

#pragma pack(push, 1)

struct PACKET_HEADER {
	BYTE	code;
	BYTE	checkSum;
	WORD	msg_type;
	WORD	payloadSize;

	PACKET_HEADER() {}
	PACKET_HEADER(BYTE	code,
		BYTE	checkSum,
		WORD	msg_type,
		WORD	payloadSize) {
		Set(code, checkSum, msg_type, payloadSize);
	}

	void Set(BYTE	code,
		BYTE	checkSum,
		WORD	msg_type,
		WORD	payloadSize) {
		this->code = code;
		this->checkSum = checkSum;
		this->msg_type = msg_type;
		this->payloadSize = payloadSize;
	}
};

#pragma pack(pop)

//------------------------------------------------------------
// 1 Req 로그인
//
//
// WCHAR[15]	: 닉네임 (유니코드)
//------------------------------------------------------------
#define REQ_LOGIN	1

// *** 닉네임으로 검색해야함 (세션에 한번)
//------------------------------------------------------------
// 2 Res 로그인              
// 
// 1Byte	: 결과 (1:OK / 2:중복닉네임 / 3:사용자초과 / 4:기타오류)
// 4Byte	: 사용자 NO
//------------------------------------------------------------
#define RES_LOGIN	2

enum : char {
	RESULT_LOGIN_OK = 1,
	RESULT_LOGIN_DNICK = 2,
	RESULT_LOGIN_MAX = 3,
	RESULT_LOGIN_ETC = 4,
};

//------------------------------------------------------------
// 3 Req 대화방 리스트
//
//	None
//------------------------------------------------------------
#define REQ_ROOM_LIST 3

//------------------------------------------------------------
// 4 Res 대화방 리스트
//
//  2Byte	: 개수
//  {
//		4Byte : 방 No
//		2Byte : 방 이름 size
//		{
//			Size Byte  : 방 이름 (유니코드)
//		}
// 
//		1Byte : 참여인원
//		{
//			WHCAR[15] : 닉네임
//		}
//	
//	}
//------------------------------------------------------------
#define RES_ROOM_LIST 4

//------------------------------------------------------------
// 5 Req 대화방 생성
//
// 2Byte : 방 이름 size
// {
//		Size  : 방 이름 (유니코드)
// }
//------------------------------------------------------------
#define REQ_ROOM_CREATE	5

// *** 방 이름으로 검색해야함
//------------------------------------------------------------
// 6 Res 대화방 생성 (수시로)
//
// 1Byte : 결과 (1:OK / 2:방이름 중복 / 3:개수초과 / 4:기타오류)
//
// 4Byte : 방 No
// 2Byte : 방 이름 byte size
// {
//		Size  : 방 이름 (유니코드)
// }
//------------------------------------------------------------
#define RES_ROOM_CREATE	6

#define RESULT_ROOM_CREATE_OK		1
#define RESULT_ROOM_CREATE_DNICK	2
#define RESULT_ROOM_CREATE_MAX		3
#define RESULT_ROOM_CREATE_ETC		4

// 
//------------------------------------------------------------
// 7 Req 대화방 입장
//
//	4Byte : 방 No
//------------------------------------------------------------
#define REQ_ROOM_ENTER		7

// 
//------------------------------------------------------------
// 8 Res 대화방 입장
//
// 1 Byte : 결과 (1:OK / 2:방No 오류 / 3:인원초과 / 4:기타오류)
// 
//	4Byte : 방 No
//	2Byte : 방 이름 size
//	{
//		Size  : 방 이름 (유니코드)
//	}
//	
//	1Byte : 참여인원
//	{
//		WHCAR[15] : 닉네임
//	}
//
//------------------------------------------------------------
#define RES_ROOM_ENTER		8

#define RESULT_ROOM_ENTER_OK			1
#define RESULT_ROOM_ENTER_NOT		2
#define RESULT_ROOM_ENTER_MAX		3
#define RESULT_ROOM_ENTER_ETC		4

//------------------------------------------------------------
// 9 Req 채팅송신
//
// 2Byte : 메시지 Size
// {
//		Size  : 대화내용(유니코드)
// }
//------------------------------------------------------------
#define REQ_CHAT				9

//------------------------------------------------------------
// 10 Res 채팅수신 (아무때나 올 수 있음)  (나에겐 오지 않음)
//
// 4Byte : 송신자 No
// 2Byte : 메시지 Size
// {
//		Size  : 대화내용(유니코드)
// }
//------------------------------------------------------------
#define RES_CHAT				10

//------------------------------------------------------------
// 11 Req 방퇴장 
//
// None
//------------------------------------------------------------
#define REQ_ROOM_LEAVE		11

//------------------------------------------------------------
// 12 Res 방퇴장 (수시)
//
// 4Byte : 사용자 No
//------------------------------------------------------------
#define RES_ROOM_LEAVE		12


//------------------------------------------------------------
// 13 Res 방삭제 (수시)
//
// 4Byte : 방 No
//------------------------------------------------------------
#define RES_ROOM_DELETE		13

//------------------------------------------------------------
// 14 Res 타 사용자 입장 (수시)
//
// WCHAR[15] : 닉네임(유니코드)
// 4Byte : 사용자 No
//------------------------------------------------------------
#define RES_USER_ENTER		14
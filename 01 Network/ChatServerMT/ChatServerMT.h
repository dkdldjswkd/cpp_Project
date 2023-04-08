#pragma once
#include <Windows.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include "../NetworkLib/NetServer.h"
#include "../../00 lib_jy/LFObjectPool.h"
#include "../../00 lib_jy/LFQueue.h"
#include "../../00 lib_jy/RecursiveLock.h"
#include "Player.h"

class ChatServerMT : public NetServer {
public:
	ChatServerMT(const char* systemFile, const char* server);
	~ChatServerMT();

private:
	static struct Token {
		char buf[64];
	};

	static struct Job {
	public:
		SessionId sessionId;
		WORD type;
		PacketBuffer* p_packet;

	public:
		void Set(SessionId sessionId, WORD type, PacketBuffer* p_packet = nullptr) {
			this->sessionId = sessionId;
			this->type = type;
			this->p_packet = p_packet;
		}
	};

private:
	// Player
	LFObjectPool<Player> playerPool;
	RecursiveLock playerMapLock;
	std::unordered_map<DWORD64, Player*> playerMap;			
	RecursiveLock sectorLock[SECTOR_MAX_Y][SECTOR_MAX_X];
	std::unordered_set<Player*> sectorSet[SECTOR_MAX_Y][SECTOR_MAX_X];

	// JOB
	LFObjectPool<Job> jobPool;
	LFQueue<Job*> jobQ;

private:
	// Lib callback
	bool OnConnectionRequest(in_addr IP, WORD Port);
	void OnClientJoin(SessionId sessionId);
	void OnRecv(SessionId sessionId, PacketBuffer* contents_packet);
	void OnClientLeave(SessionId sessionId);
	void OnServerStop();

	// Job
	bool ProcPacket(SessionId sessionId, WORD type, PacketBuffer* csContentsPacket);
	bool ProcPacket_en_PACKET_CS_CHAT_REQ_LOGIN(SessionId sessionId, PacketBuffer* csContentsPacket);	// 1
	bool ProcPacket_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE(SessionId sessionId, PacketBuffer* csContentsPacket);	// 3
	bool ProcPacket_en_PACKET_CS_CHAT_REQ_MESSAGE(SessionId sessionId, PacketBuffer* csContentsPacket);	// 5

	// Send
	void SendSectorAround(Player* p_player, PacketBuffer* send_packet);
	void SendSector(PacketBuffer* send_packet, Sector sector);

public:
	// 모니터링
	void UpdateTPS();
	DWORD GetPlayerCount();
	DWORD GetPlayerPoolCount();
};

inline void ChatServerMT::UpdateTPS(){
}

inline DWORD ChatServerMT::GetPlayerCount() {
	return playerMap.size();
}

inline DWORD ChatServerMT::GetPlayerPoolCount(){
	return playerPool.GetUseCount();
}
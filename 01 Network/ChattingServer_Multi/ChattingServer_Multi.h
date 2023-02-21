#pragma once
#include <Windows.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include "../NetworkLib/LFObjectPool.h"
#include "../NetworkLib/LFQueue.h"
#include "../NetworkLib/NetworkLib.h"
#include "../NetworkLib/RecursiveLock.h"
#include "Player.h"

class ChattingServer_Multi : public NetworkLib {
public:
	ChattingServer_Multi(const char* systemFile, const char* server);
	~ChattingServer_Multi();

private:
	static struct Job {
	public:
		SESSION_ID session_id;
		WORD type;
		PacketBuffer* p_packet;

	public:
		void Set(SESSION_ID session_id, WORD type, PacketBuffer* p_packet = nullptr) {
			this->session_id = session_id;
			this->type = type;
			this->p_packet = p_packet;
		}
	};

private:
	// Player
	J_LIB::LFObjectPool<Player> playerPool;
	RecursiveLock playerMap_lock;
	std::unordered_map<DWORD64, Player*> player_map;			
	RecursiveLock sector_lock[SECTOR_MAX_Y][SECTOR_MAX_X];
	std::unordered_set<Player*> sectors_set[SECTOR_MAX_Y][SECTOR_MAX_X];

private:
	// JOB
	J_LIB::LFObjectPool<Job> jobPool;
	LFQueue<Job*> jobQ;

private:
	// Lib callback
	bool OnConnectionRequest(in_addr IP, WORD Port);
	void OnClientJoin(SESSION_ID session_id);
	void OnRecv(SESSION_ID session_id, PacketBuffer* contents_packet);
	void OnClientLeave(SESSION_ID session_id);
	void OnError(int errorcode);

private:
	void Disconnect_Player(Player* p_player);

private:
	void SendSectorAround(Player* p_player, PacketBuffer* send_packet);
	void SendSector(PacketBuffer* send_packet, Sector sector);

private:
	// JOB 처리
	bool ProcPacket(SESSION_ID session_id, WORD type, PacketBuffer* cs_contentsPacket);
	// 패킷(JOB) 처리
	bool ProcPacket_en_PACKET_CS_CHAT_REQ_LOGIN		(SESSION_ID session_id, PacketBuffer* cs_contentsPacket);	// 1
	bool ProcPacket_en_PACKET_CS_CHAT_REQ_SECTOR_MOVE	(SESSION_ID session_id, PacketBuffer* cs_contentsPacket);	// 3
	bool ProcPacket_en_PACKET_CS_CHAT_REQ_MESSAGE		(SESSION_ID session_id, PacketBuffer* cs_contentsPacket);	// 5

public:
	// 모니터링
	DWORD Get_playerCount();
	DWORD Get_playerPoolCount();
};

inline DWORD ChattingServer_Multi::Get_playerCount() {
	return player_map.size();
}

inline DWORD ChattingServer_Multi::Get_playerPoolCount(){
	return playerPool.Get_UseCount();
}

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
	// Player
	LFObjectPool<Player> playerPool;
	std::unordered_map<DWORD64, Player*> playerMap;			
	RecursiveLock playerMapLock;
	std::unordered_set<Player*> sectorSet[SECTOR_MAX_Y][SECTOR_MAX_X];
	RecursiveLock sectorLock[SECTOR_MAX_Y][SECTOR_MAX_X];

	// Lib callback
	bool OnConnectionRequest(in_addr IP, WORD Port);
	void OnClientJoin(SessionId sessionId);
	void OnRecv(SessionId sessionId, PacketBuffer* contents_packet);
	void OnClientLeave(SessionId sessionId);
	void OnServerStop();

	// Send
	void SendSectorAround(Player* p_player, PacketBuffer* send_packet);
	void SendSector(PacketBuffer* send_packet, Sector sector);

	// 모니터링
	int updateTPS = 0;
	alignas(64) int updateCount = 0;

public:
	// 모니터링
	void UpdateTPS();
	DWORD GetUpdateTPS();
	DWORD GetUserCount();
	DWORD GetUserPoolCount();
};

inline void ChatServerMT::UpdateTPS() {
	updateTPS = InterlockedExchange((LONG*)&updateCount, 0);
}

inline DWORD ChatServerMT::GetUpdateTPS() {
	return updateTPS;
}

inline DWORD ChatServerMT::GetUserCount() {
	return playerMap.size();
}

inline DWORD ChatServerMT::GetUserPoolCount() {
	return playerPool.GetUseCount();
}

#pragma once
#include "../NetworkLib/NetClient.h"
#include <thread>

class MonitoringClient : public NetClient {
public:
	MonitoringClient(const char* systemFile, const char* client);
	~MonitoringClient();

private:
	std::thread updateThread;
	int serverNo;

private:
	void OnConnect();
	void OnRecv(PacketBuffer* contents_packet);
	void OnDisconnect();

private:
	void UpdateFunc();
};
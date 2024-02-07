#pragma once
#include <cstdint>
#include <memory>

typedef struct _ENetHost ENetHost;
typedef struct _ENetPeer ENetPeer;

class Client
{
public:

	static Client connect_localhost(bool& succeeded);

	static Client connect_host(bool& succeeeded, const char* ip, int32_t port);

	static constexpr int WINDOW_SIZE_X = 1000;

	static constexpr int WINDOW_SIZE_Y = 1000;

	static constexpr const char* TITLE = "DeviousMud Replicant";

	void start_ticking();

	void quit();


public:
	~Client() = default;

protected:
	Client& operator=(Client const&) = delete;
	Client(ENetHost* c);

private:
	void init();

private:

	ENetHost* host;
	ENetPeer* peer;
};
#include "../main/precompiled.hpp"
#include "../main/singletons/epoll_daemon.hpp"
#include "../main/config_file.hpp"
#include "../main/log.hpp"
#include "../main/exception.hpp"
#include "../main/module_raii.hpp"
using namespace Poseidon;

MODULE_RAII(
	LOG_INFO("Initializing player server...");

	ConfigFile config("config/player_server.conf");

	std::string bind;
	boost::uint16_t port;
	std::string certificate;
	std::string privateKey;

	config.get(bind, "player_server_bind", "0.0.0.0");
	config.get(port, "player_server_port", 8850);
	config.get(certificate, "player_server_certificate", "");
	config.get(privateKey, "player_server_private_key", "");

	return EpollDaemon::registerPlayerServer(bind, port, certificate, privateKey);
)
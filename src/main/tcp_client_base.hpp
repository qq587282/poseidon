#ifndef POSEIDON_TCP_CLIENT_BASE_HPP_
#define POSEIDON_TCP_CLIENT_BASE_HPP_

#include "tcp_session_base.hpp"
#include <string>
#include <boost/shared_ptr.hpp>

namespace Poseidon {

class TcpClientBase : public TcpSessionBase {
protected:
	static void connect(ScopedFile &socket, const std::string &ip, unsigned port);

protected:
	explicit TcpClientBase(Move<ScopedFile> socket);

public:
	void addIntoEpoll();
};

}

#endif

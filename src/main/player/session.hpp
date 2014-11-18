// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef POSEIDON_PLAYER_SESSION_HPP_
#define POSEIDON_PLAYER_SESSION_HPP_

#include <cstddef>
#include <boost/cstdint.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/utility/enable_if.hpp>
#include "../tcp_session_base.hpp"
#include "../stream_buffer.hpp"
#include "status.hpp"

namespace Poseidon {

class ProtocolBase;

class PlayerSession : public TcpSessionBase {
	friend class PlayerServer;

private:
	const std::size_t m_category;

	int m_payloadLen;
	unsigned m_protocolId;
	StreamBuffer m_payload;

public:
	PlayerSession(std::size_t category, ScopedFile socket);
	~PlayerSession();

private:
	void onReadAvail(const void *data, std::size_t size);

public:
	std::size_t getCategory() const {
		return m_category;
	}

	bool send(boost::uint16_t protocolId, StreamBuffer contents, bool final = false);

	template<class ProtocolT>
	typename boost::enable_if<boost::is_base_of<ProtocolBase, ProtocolT>, bool>::type
		send(const ProtocolT &contents, bool final = false)
	{
		return send(ProtocolT::ID, StreamBuffer(contents), final);
	}

	bool sendError(boost::uint16_t protocolId, PlayerStatus status, std::string reason, bool final = false);
	bool sendError(boost::uint16_t protocolId, PlayerStatus status, bool final = false){
		return sendError(protocolId, status, std::string(), final);
	}
};

}

#endif

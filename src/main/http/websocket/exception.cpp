// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "../../precompiled.hpp"
#include "exception.hpp"
using namespace Poseidon;

WebSocketException::WebSocketException(const char *file, std::size_t line,
	WebSocketStatus status, SharedNtmbs message)
	: ProtocolException(file, line, STD_MOVE(message), static_cast<unsigned>(status))
{
}
WebSocketException::~WebSocketException() NOEXCEPT {
}

// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef POSEIDON_RAII_HPP_
#define POSEIDON_RAII_HPP_

#include "cxx_ver.hpp"
#include <utility>
#include <ostream>

namespace Poseidon {

template<typename CloserT>
class ScopedHandle {
public:
	typedef VALUE_TYPE(DECLREF(CloserT)()) Handle;

private:
	Handle m_handle;

public:
	ScopedHandle() NOEXCEPT
		: m_handle(CloserT()())
	{
	}
	explicit ScopedHandle(Handle handle) NOEXCEPT
		: m_handle(CloserT()())
	{
		reset(handle);
	}
	~ScopedHandle() NOEXCEPT {
		reset();
	}

#ifdef POSEIDON_CXX11
	ScopedHandle(const ScopedHandle &) = delete;
	void operator=(const ScopedHandle &) = delete;

	ScopedHandle(ScopedHandle &&rhs) noexcept
		: m_handle(CloserT()())
	{
		swap(rhs);
	}
	ScopedHandle &operator=(ScopedHandle &&rhs) noexcept {
		swap(rhs);
		return *this;
	}
#else
	// public 但是没有定义。仅作为 RVO 转移使用，如果拷贝构造会导致错误。
	ScopedHandle(const ScopedHandle &)
		__attribute__((__error__("Use explicit STD_MOVE() to transfer ownership.")));
	void operator=(const ScopedHandle &)
		__attribute__((__error__("Use explicit STD_MOVE() to transfer ownership.")));
#endif

public:
	Handle get() const NOEXCEPT {
		return m_handle;
	}
	Handle release() NOEXCEPT {
		const Handle ret = m_handle;
		m_handle = CloserT()();
		return ret;
	}
	void reset(Handle handle = CloserT()()) NOEXCEPT {
		const Handle old = m_handle;
		m_handle = handle;
		if(old != CloserT()()){
			CloserT()(old);
		}
	}
	void reset(Move<ScopedHandle> rhs) NOEXCEPT {
		rhs.swap(*this);
	}

	void swap(ScopedHandle &rhs) NOEXCEPT {
		using std::swap;
		swap(m_handle, rhs.m_handle);
	}

#ifdef POSEIDON_CXX11
	explicit operator bool() const noexcept {
		return get() != CloserT()();
	}
#else
	typedef Handle (ScopedHandle::*DummyBool_)() const;
	operator DummyBool_() const NOEXCEPT {
		return (get() != CloserT()()) ? &ScopedHandle::get : 0;
	}
#endif
};

template<typename CloserT>
void swap(ScopedHandle<CloserT> &lhs, ScopedHandle<CloserT> &rhs) NOEXCEPT {
	lhs.swap(rhs);
}

template<typename CloserT>
bool operator==(const ScopedHandle<CloserT> &lhs, const ScopedHandle<CloserT> &rhs){
	return lhs.get() == rhs.get();
}
template<typename CloserT>
bool operator==(const ScopedHandle<CloserT> &lhs, typename ScopedHandle<CloserT>::Handle rhs){
	return lhs.get() == rhs;
}
template<typename CloserT>
bool operator==(typename ScopedHandle<CloserT>::Handle lhs, const ScopedHandle<CloserT> &rhs){
	return lhs == rhs.get();
}
template<typename CloserT>
bool operator!=(const ScopedHandle<CloserT> &lhs, const ScopedHandle<CloserT> &rhs){
	return lhs.get() != rhs.get();
}
template<typename CloserT>
bool operator!=(const ScopedHandle<CloserT> &lhs, typename ScopedHandle<CloserT>::Handle rhs){
	return lhs.get() != rhs;
}
template<typename CloserT>
bool operator!=(typename ScopedHandle<CloserT>::Handle lhs, const ScopedHandle<CloserT> &rhs){
	return lhs != rhs.get();
}

template<typename CloserT>
bool operator<(const ScopedHandle<CloserT> &lhs, const ScopedHandle<CloserT> &rhs){
	return lhs.get() < rhs.get();
}
template<typename CloserT>
bool operator<(const ScopedHandle<CloserT> &lhs, typename ScopedHandle<CloserT>::Handle rhs){
	return lhs.get() < rhs;
}
template<typename CloserT>
bool operator<(typename ScopedHandle<CloserT>::Handle lhs, const ScopedHandle<CloserT> &rhs){
	return lhs < rhs.get();
}
template<typename CloserT>
bool operator>(const ScopedHandle<CloserT> &lhs, const ScopedHandle<CloserT> &rhs){
	return lhs.get() > rhs.get();
}
template<typename CloserT>
bool operator>(const ScopedHandle<CloserT> &lhs, typename ScopedHandle<CloserT>::Handle rhs){
	return lhs.get() > rhs;
}
template<typename CloserT>
bool operator>(typename ScopedHandle<CloserT>::Handle lhs, const ScopedHandle<CloserT> &rhs){
	return lhs > rhs.get();
}

template<typename CloserT>
bool operator<=(const ScopedHandle<CloserT> &lhs, const ScopedHandle<CloserT> &rhs){
	return lhs.get() <= rhs.get();
}
template<typename CloserT>
bool operator<=(const ScopedHandle<CloserT> &lhs, typename ScopedHandle<CloserT>::Handle rhs){
	return lhs.get() <= rhs;
}
template<typename CloserT>
bool operator<=(typename ScopedHandle<CloserT>::Handle lhs, const ScopedHandle<CloserT> &rhs){
	return lhs <= rhs.get();
}
template<typename CloserT>
bool operator>=(const ScopedHandle<CloserT> &lhs, const ScopedHandle<CloserT> &rhs){
	return lhs.get() >= rhs.get();
}
template<typename CloserT>
bool operator>=(const ScopedHandle<CloserT> &lhs, typename ScopedHandle<CloserT>::Handle rhs){
	return lhs.get() >= rhs;
}
template<typename CloserT>
bool operator>=(typename ScopedHandle<CloserT>::Handle lhs, const ScopedHandle<CloserT> &rhs){
	return lhs >= rhs.get();
}

template<typename CloserT>
std::ostream &operator<<(std::ostream &os, const ScopedHandle<CloserT> &handle){
	return os <<handle.get();
}
template<typename CloserT>
std::wostream &operator<<(std::wostream &os, const ScopedHandle<CloserT> &handle){
	return os <<handle.get();
}

extern void closeFile(int fd) NOEXCEPT;

struct FileCloser {
	CONSTEXPR int operator()() const NOEXCEPT {
		return -1;
	}
	void operator()(int fd) const NOEXCEPT {
		closeFile(fd);
	}
};

typedef ScopedHandle<FileCloser> ScopedFile;

}

#endif

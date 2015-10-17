#include "../precompiled.hpp"
#include "client.hpp"
#include "exception.hpp"
#include "status_codes.hpp"
#include "../log.hpp"
#include "../job_base.hpp"
#include "../profiler.hpp"

namespace Poseidon {

namespace Http {
	class Client::SyncJobBase : public JobBase {
	private:
		const boost::weak_ptr<Client> m_client;

	protected:
		explicit SyncJobBase(const boost::shared_ptr<Client> &client)
			: m_client(client)
		{
		}

	private:
		boost::weak_ptr<const void> getCategory() const FINAL {
			return m_client;
		}
		void perform() FINAL {
			PROFILE_ME;

			const AUTO(client, m_client.lock());
			if(!client){
				return;
			}

			try {
				reallyPerform(client);
			} catch(std::exception &e){
				LOG_POSEIDON(Logger::SP_MAJOR | Logger::LV_INFO, "std::exception thrown: what = ", e.what());
				client->forceShutdown();
				throw;
			} catch(...){
				LOG_POSEIDON(Logger::SP_MAJOR | Logger::LV_INFO, "Unknown exception thrown.");
				client->forceShutdown();
				throw;
			}
		}

	protected:
		virtual void reallyPerform(const boost::shared_ptr<Client> &client) = 0;
	};

	class Client::ConnectJob : public Client::SyncJobBase {
	public:
		explicit ConnectJob(const boost::shared_ptr<Client> &client)
			: SyncJobBase(client)
		{
		}

	protected:
		void reallyPerform(const boost::shared_ptr<Client> &client) OVERRIDE {
			PROFILE_ME;

			client->onSyncConnect();
		}
	};

	class Client::ResponseHeadersJob : public Client::SyncJobBase {
	private:
		ResponseHeaders m_responseHeaders;
		std::string m_transferEncoding;
		boost::uint64_t m_contentLength;

	public:
		ResponseHeadersJob(const boost::shared_ptr<Client> &client,
			ResponseHeaders responseHeaders, std::string transferEncoding, boost::uint64_t contentLength)
			: SyncJobBase(client)
			, m_responseHeaders(STD_MOVE(responseHeaders)), m_transferEncoding(STD_MOVE(transferEncoding)), m_contentLength(contentLength)
		{
		}

	protected:
		void reallyPerform(const boost::shared_ptr<Client> &client) OVERRIDE {
			PROFILE_ME;

			client->onSyncResponseHeaders(STD_MOVE(m_responseHeaders), STD_MOVE(m_transferEncoding), m_contentLength);
		}
	};

	class Client::ResponseEntityJob : public Client::SyncJobBase {
	private:
		boost::uint64_t m_contentOffset;
		bool m_isChunked;
		StreamBuffer m_entity;

	public:
		ResponseEntityJob(const boost::shared_ptr<Client> &client, boost::uint64_t contentOffset, bool isChunked, StreamBuffer entity)
			: SyncJobBase(client)
			, m_contentOffset(contentOffset), m_isChunked(isChunked), m_entity(STD_MOVE(entity))
		{
		}

	protected:
		void reallyPerform(const boost::shared_ptr<Client> &client) OVERRIDE {
			PROFILE_ME;

			client->onSyncResponseEntity(m_contentOffset, m_isChunked, STD_MOVE(m_entity));
		}
	};

	class Client::ResponseEndJob : public Client::SyncJobBase {
	private:
		boost::uint64_t m_contentLength;
		bool m_isChunked;
		OptionalMap m_headers;

	public:
		ResponseEndJob(const boost::shared_ptr<Client> &client, boost::uint64_t contentLength, bool isChunked, OptionalMap headers)
			: SyncJobBase(client)
			, m_contentLength(contentLength), m_isChunked(isChunked), m_headers(STD_MOVE(headers))
		{
		}

	protected:
		void reallyPerform(const boost::shared_ptr<Client> &client) OVERRIDE {
			PROFILE_ME;

			client->onSyncResponseEnd(m_contentLength, m_isChunked, STD_MOVE(m_headers));
		}
	};

	Client::Client(const SockAddr &addr, bool useSsl)
		: TcpClientBase(addr, useSsl)
	{
	}
	Client::Client(const IpPort &addr, bool useSsl)
		: TcpClientBase(addr, useSsl)
	{
	}
	Client::~Client(){
	}

	void Client::onConnect(){
		PROFILE_ME;

		enqueueJob(boost::make_shared<ConnectJob>(
			virtualSharedFromThis<Client>()));

		TcpClientBase::onConnect();
	}
	void Client::onReadHup() NOEXCEPT {
		PROFILE_ME;

		try {
			if(ClientReader::isContentTillEof()){
				terminateContent();
			}
		} catch(std::exception &e){
			LOG_POSEIDON_WARNING("std::exception thrown: what = ", e.what());
			forceShutdown();
		} catch(...){
			LOG_POSEIDON_WARNING("Unknown exception thrown");
			forceShutdown();
		}

		TcpClientBase::onReadHup();
	}

	void Client::onReadAvail(StreamBuffer data){
		PROFILE_ME;

		ClientReader::putEncodedData(STD_MOVE(data));
	}

	void Client::onResponseHeaders(ResponseHeaders responseHeaders, std::string transferEncoding, boost::uint64_t contentLength){
		PROFILE_ME;

		enqueueJob(boost::make_shared<ResponseHeadersJob>(
			virtualSharedFromThis<Client>(), STD_MOVE(responseHeaders), STD_MOVE(transferEncoding), contentLength));
	}
	void Client::onResponseEntity(boost::uint64_t entityOffset, bool isChunked, StreamBuffer entity){
		PROFILE_ME;

		enqueueJob(boost::make_shared<ResponseEntityJob>(
			virtualSharedFromThis<Client>(), entityOffset, isChunked, STD_MOVE(entity)));
	}
	bool Client::onResponseEnd(boost::uint64_t contentLength, bool isChunked, OptionalMap headers){
		PROFILE_ME;

		enqueueJob(boost::make_shared<ResponseEndJob>(
			virtualSharedFromThis<Client>(), contentLength, isChunked, STD_MOVE(headers)));

		return true;
	}

	long Client::onEncodedDataAvail(StreamBuffer encoded){
		PROFILE_ME;

		return TcpClientBase::send(STD_MOVE(encoded));
	}

	void Client::onSyncConnect(){
	}

	bool Client::sendHeaders(RequestHeaders requestHeaders){
		return ClientWriter::putRequestHeaders(STD_MOVE(requestHeaders));
	}
	bool Client::sendEntity(StreamBuffer data){
		return ClientWriter::putEntity(STD_MOVE(data));
	}

	bool Client::send(RequestHeaders requestHeaders, StreamBuffer entity){
		return ClientWriter::putRequest(STD_MOVE(requestHeaders), STD_MOVE(entity));
	}

	bool Client::sendChunkedHeader(RequestHeaders requestHeaders){
		return ClientWriter::putChunkedHeader(STD_MOVE(requestHeaders));
	}
	bool Client::sendChunk(StreamBuffer entity){
		return ClientWriter::putChunk(STD_MOVE(entity));
	}
	bool Client::sendChunkedTrailer(OptionalMap headers){
		return ClientWriter::putChunkedTrailer(STD_MOVE(headers));
	}
}

}

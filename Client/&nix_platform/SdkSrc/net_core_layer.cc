#include "net_core_layer.h"
#include <sys/types.h>
#include <assert.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include "defines.h"
#include "net_inter_layer.h"
#include "net_interface_defines.h"
#include "utils.h"
#include "message.h"

bool CNetCoreLayer::persist_connection_has_ = false;
bool CNetCoreLayer::persist_connection_libevent_= false;

CNetCoreLayer::CNetCoreLayer(void)
{
	local_read_event_ = NULL;
	base_ = NULL;
	pNetInterLayer_ = NULL;
	memset(&inData_persist_conn_, 0, sizeof(inData_persist_conn_));
	persist_sfd_ = 0;
	error_code_  = 0;
	port_ = 0;
}

CNetCoreLayer::~CNetCoreLayer(void)
{
	event_del(local_read_event_);
	evutil_closesocket(pipe_[0]);
	evutil_closesocket(pipe_[1]);

	if (-1 == event_base_loopexit(base_, NULL))
	{
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::~CNetCoreLayer:event_base_loopexit error.");
	}

	event_base_free(base_);
}

bool CNetCoreLayer::InitNetCore(CNetInterLayer* pNetInterLayer, const std::string& ip, const int port)
{
	pNetInterLayer_ = pNetInterLayer;

	const char* libevent_version = event_get_version();
	LOG4CXX_TRACE(g_logger, "CNetCoreLayer::InitNetCore:libevent version = " << libevent_version);

	base_ = event_base_new();
	assert(base_ != NULL);

	int ret = pipe(pipe_);
	if (0 != ret)
	{
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::InitNetCore:pipe errorCode = " << ret);
		return false;
	}

	server_ip_ 	= ip;
	port_		= port;
	LOG4CXX_TRACE(g_logger, "CNetCoreLayer::InitNetCore server_ip = "<< server_ip_ << ", server_port = " << port_);

	return true;
}

void CNetCoreLayer::Run()
{
	local_read_event_ = event_new(base_, pipe_[0], EV_READ | EV_PERSIST, DoLocalRead, (void*) this);
	assert(local_read_event_ != NULL);
	event_add(local_read_event_, NULL);

	LOG4CXX_INFO(g_logger, "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^CNetCoreLayer::Run....^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");

	int i = event_base_dispatch(base_);

	LOG4CXX_INFO(g_logger, "CNetCoreLayer::event_base_dispatch = " << i);
}

void CNetCoreLayer::DoLocalRead(evutil_socket_t local_tcp_server, short event, void *arg)
{
	CNetCoreLayer* pThis = static_cast<CNetCoreLayer*>(arg);

	char recv_buf[1];
	int size = read(local_tcp_server, recv_buf, 1);
	if (size != 1)
	{
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::DoLocalRead recv error. error = " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		return;
	}

	REQUEST_INFO requestInfo;
	if (!pThis->threadSafeList_.pop_front(requestInfo))
	{
		LOG4CXX_WARN(g_logger, "CNetCoreLayer::DoLocalRead pop_front NULL.");
		return;
	}

	if (SHORT_CONNECTION == requestInfo.tcp_connect_type)
	{
		pThis->ShortConnectionOpt(requestInfo);
	}

	if (!persist_connection_libevent_)
	{
		if (PERSIST_CONNECTION == requestInfo.tcp_connect_type)
		{
			pThis->PersistConnectionOpt(requestInfo);
		}
	}

}

void CNetCoreLayer::ShortConnectionOpt(const REQUEST_INFO& requestInfo)
{
	short flag = EV_READ | EV_PERSIST;
	IN_DATA* ptr_inData;
	try
	{
		ptr_inData = new IN_DATA;
	}
	catch (std::bad_alloc &)
	{
		LOG4CXX_WARN(g_logger, "CNetCoreLayer::ShortConnectionOpt:IN_DATA not enough memory!");
		return;
	}

	ptr_inData->client_tcp_sock = requestInfo.sfd;
	ptr_inData->tcp_connect_type = requestInfo.tcp_connect_type;
	bzero(ptr_inData->buf, BUF_SIZE);
	ptr_inData->buf_len = 0;
	ptr_inData->base_ptr = pNetInterLayer_;

	struct bufferevent *short_tcp_recv_event = bufferevent_socket_new(this->base_, requestInfo.sfd,
			BEV_OPT_CLOSE_ON_FREE);
	if (short_tcp_recv_event == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::ShortConnectionOpt:bufferevent_socket_new");
		utils::SafeDelete(ptr_inData);
		close(requestInfo.sfd);
		return;
	}
	bufferevent_setcb(short_tcp_recv_event, DoRemoteShortTcpRead, NULL, DoRemoteShortTcpError, (void*) ptr_inData);

	struct timeval read_sec;
	read_sec.tv_sec = 10;
	read_sec.tv_usec = 0;
	bufferevent_set_timeouts(short_tcp_recv_event, &read_sec, NULL);

	bufferevent_enable(short_tcp_recv_event, flag);

	/** 向服务端发送数据 (对特殊字符进行转义处理\r\n) */
	std::string request_append = utils::ReplaceString(requestInfo.message, "\\r\\n", "\\\\r\\\\n");
	request_append = request_append + std::string(CRLF);
	int send_size = send(requestInfo.sfd, request_append.c_str(), request_append.length(), 0);
	if (send_size < 0)
	{
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::AddShortConnectionResquest:send failed. error = " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		close(requestInfo.sfd);
		return;
	}
}

void CNetCoreLayer::PersistConnectionOpt(const REQUEST_INFO& requestInfo)
{
	short flag = EV_READ | EV_PERSIST;
	struct bufferevent *persist_tcp_recv_event = bufferevent_socket_new(this->base_, requestInfo.sfd,
			BEV_OPT_CLOSE_ON_FREE);
	if (persist_tcp_recv_event == NULL)
	{
		LOG4CXX_FATAL(g_logger, "CNetCoreLayer::PersistConnectionOpt!!!!!!");
		return;
	}
	bufferevent_setcb(persist_tcp_recv_event, DoRemotePersistTcpRead, NULL, DoRemotePersistTcpError, (void*) this);
	bufferevent_enable(persist_tcp_recv_event, flag);
	persist_connection_libevent_ = true;
}

int CNetCoreLayer::AddShortConnectionResquest(const std::string& request)
{
	/** 如果是短连接，每次生成一个生 新的tcp socket 向服务端发送数据 */
	evutil_socket_t short_sfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(short_sfd > 0);

	/** 初始化连接 */
	struct sockaddr_in server_addr_;
	bzero(&server_addr_, sizeof(server_addr_));
	server_addr_.sin_family = AF_INET;
	std::string server_ip = server_ip_;
	int port = port_;
	server_addr_.sin_addr.s_addr = inet_addr(server_ip.c_str());
	server_addr_.sin_port = htons(port);

	int ret = 0;
	int timeout_sec = 10;
	if ((ret = connect_nonb(short_sfd, (struct sockaddr *) &server_addr_, sizeof(server_addr_), timeout_sec)) < 0)
	{
		if (-2 == ret)
		{
			close(short_sfd);
			return CANT_CONNECT_SERVER;
		}
		else
		{
			close(short_sfd);
			return CONN_OTHER_ERROR;
		}
	}

	if (!SetTCP_NODELAY(short_sfd))
	{
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::AddShortConnectionResquest:setsockopt(TCP_NODELAY) short_sfd failed. error = "
								<<evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		close(short_sfd);
		return CONN_OTHER_ERROR;
	}

	/** 通知 libevent线程，加入监控此socket */
	REQUEST_INFO requestInfo;
	requestInfo.sfd = short_sfd;
	requestInfo.tcp_connect_type = SHORT_CONNECTION;
	requestInfo.message = std::string(TOKEN_STR) + request;

	threadSafeList_.push_back(requestInfo);
	int size = write(pipe_[1], CONNECTION_FLAG, 1);;
	if (size < 0)
	{
		LOG4CXX_WARN(g_logger,
				"CNetCoreLayer::AddShortConnectionResquest:send error. errorCode = " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		close(short_sfd);
		return CONN_OTHER_ERROR;
	}

	return SUCCESS;
}

int CNetCoreLayer::AddPersistConnectionRequest(const std::string& request)
{

	std::string request_add_token = request;

	/** 如果是长连接，只生成一个tcp socket 向服务端发送数据 */
	if (!persist_connection_has_)
	{
		//request_add_token = std::string(TOKEN_STR) + request_add_token;
		persist_sfd_ = socket(AF_INET, SOCK_STREAM, 0);
		assert(persist_sfd_ != 0);

		/** 初始化连接 */
		struct sockaddr_in server_addr_;
		bzero(&server_addr_, sizeof(server_addr_));
		server_addr_.sin_family = AF_INET;
		std::string server_ip = server_ip_;
		int port = port_;
		server_addr_.sin_addr.s_addr = inet_addr(server_ip.c_str());
		server_addr_.sin_port = htons(port);

		int ret = 0;
		int timeout_sec = 10;
		if ((ret = connect_nonb(persist_sfd_, (struct sockaddr *) &server_addr_, sizeof(server_addr_), timeout_sec))
				< 0)
		{
			if (-2 == ret)
			{
				close(persist_sfd_);
				return CANT_CONNECT_SERVER;
			}
			else
			{
				close(persist_sfd_);
				return CONN_OTHER_ERROR;
			}
		}

		if (!SetTCP_NODELAY(persist_sfd_))
		{
			LOG4CXX_ERROR(g_logger, "CNetCoreLayer::AddPersistConnectioRequest:setsockopt(TCP_NODELAY)  \
									 persist_sfd failed. errorCode = " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
			close(persist_sfd_);
			return CONN_OTHER_ERROR;
		}

		persist_connection_has_ = true;

		/** 通知 libevent线程，加入监控此socket */
		REQUEST_INFO requestInfo;
		requestInfo.sfd = persist_sfd_;
		requestInfo.tcp_connect_type = PERSIST_CONNECTION;
		requestInfo.message = request_add_token;
		threadSafeList_.push_back(requestInfo);
		int size = write(pipe_[1], CONNECTION_FLAG, 1);;
		if (size < 0)
		{
			LOG4CXX_ERROR(g_logger, "CNetCoreLayer::AddPersistConnectioRequest:send error. errorCode = " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
			close(persist_sfd_);
			return CONN_OTHER_ERROR;
		}

		LOG4CXX_INFO(g_logger, "CNetCoreLayer::AddPersistConnectioRequest:persist_connection_has_ = " << persist_connection_has_);

		inData_persist_conn_.client_tcp_sock = persist_sfd_;
		inData_persist_conn_.tcp_connect_type = PERSIST_CONNECTION;
		bzero(inData_persist_conn_.buf, BUF_SIZE);
		inData_persist_conn_.buf_len = 0;
	}

	std::string request_append = utils::ReplaceString(request_add_token, "\\r\\n", "\\\\r\\\\n");
	request_append = request_append + std::string(CRLF);
	int send_size = send(persist_sfd_, request_append.c_str(), request_append.length(), 0);
	if (send_size < 0)
	{
		LOG4CXX_WARN(g_logger,
				"CNetCoreLayer::AddPersistConnectioRequest:send failed. error = " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		return CONN_OTHER_ERROR;
	}

	return SUCCESS;

}

void CNetCoreLayer::DoRemoteShortTcpRead(struct bufferevent *bev, void *arg)
{
	IN_DATA* ptr_inData = static_cast<IN_DATA*>(arg);

	int recv_size = 0;
	if ((recv_size = bufferevent_read(bev, ptr_inData->buf + ptr_inData->buf_len, RECIVE_BUF_SIZE - ptr_inData->buf_len))
			> 0)
	{
		ptr_inData->buf_len = ptr_inData->buf_len + recv_size;
	}

	std::string str_recv(ptr_inData->buf, ptr_inData->buf_len);
	if (utils::FindCRLF(str_recv))
	{
		bufferevent_free(bev);

		/** 回调业务层接口，将收到回复数据返回给业务层 */
		str_recv = str_recv.substr(0, str_recv.find(CRLF));
		ptr_inData->base_ptr->ReciveData(str_recv, ptr_inData->tcp_connect_type);
		utils::SafeDelete(ptr_inData);
		return;
	}
}

void CNetCoreLayer::DoRemoteShortTcpError(struct bufferevent *bev, short event, void *arg)
{
	IN_DATA* ptr_inData = static_cast<IN_DATA*>(arg);

	evutil_socket_t socket_fd = bufferevent_getfd(bev);

	LOG4CXX_INFO(g_logger, "CNetCoreLayer::DoRemoteShortTcpError:fd = " << socket_fd);

	if (event & BEV_EVENT_TIMEOUT)
	{
		LOG4CXX_WARN(g_logger, "CNetCoreLayer::DoRemoteShortTcpError:TimeOut.");
	}
	else if (event & BEV_EVENT_EOF)
	{
		LOG4CXX_WARN(g_logger, "CNetCoreLayer::DoRemoteShortTcpError:connection closed.");
	}
	else if (event & BEV_EVENT_ERROR)
	{
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::DoRemoteShortTcpError:some other error = " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
	}

	utils::SafeDelete(ptr_inData);
	bufferevent_free(bev);
}

void CNetCoreLayer::DoRemotePersistTcpRead(struct bufferevent *bev, void *arg)
{
	CNetCoreLayer* pThis = static_cast<CNetCoreLayer*>(arg);

	int recv_size = 0;
	if ((recv_size = bufferevent_read(bev, pThis->inData_persist_conn_.buf + pThis->inData_persist_conn_.buf_len,
			RECIVE_BUF_SIZE - pThis->inData_persist_conn_.buf_len)) > 0)
	{
		pThis->inData_persist_conn_.buf_len = pThis->inData_persist_conn_.buf_len + recv_size;
	}

	std::string str_recv(pThis->inData_persist_conn_.buf, pThis->inData_persist_conn_.buf_len);
	if (utils::FindCRLF(str_recv))
	{
		/** 回调业务层接口，将收到回复数据返回给业务层 */

		// 可能同时会收到多条数据
		std::vector<std::string> vec_data;
		utils::SplitData(str_recv, CRLF, vec_data);
		for (unsigned int i = 0; i < vec_data.size(); ++i)
		{
			pThis->pNetInterLayer_->ReciveData(vec_data.at(i), pThis->inData_persist_conn_.tcp_connect_type);
		}

		int len = str_recv.find_last_of(CRLF) + 1;
		memmove(pThis->inData_persist_conn_.buf, pThis->inData_persist_conn_.buf + len, RECIVE_BUF_SIZE - len);
		pThis->inData_persist_conn_.buf_len = pThis->inData_persist_conn_.buf_len - len;
		return;
	}

}

void CNetCoreLayer::DoRemotePersistTcpError(struct bufferevent *bev, short event, void *arg)
{
	CNetCoreLayer* pThis = static_cast<CNetCoreLayer*>(arg);
	evutil_socket_t socket_fd = bufferevent_getfd(bev);

	LOG4CXX_INFO(g_logger, "CNetCoreLayer::DoRemotePersistTcpError:fd = " << socket_fd);

	if (event & BEV_EVENT_EOF)
	{
		pThis->pNetInterLayer_->ReciveData(std::string(STR_PTCP_HAS_CLOSED), PERSIST_CONNECTION);
		LOG4CXX_WARN(g_logger, "CNetCoreLayer::DoRemotePersistTcpError:connection closed.");
	}
	else if (event & BEV_EVENT_ERROR)
	{
		pThis->pNetInterLayer_->ReciveData(std::string(STR_PTCP_HAS_ERROR), PERSIST_CONNECTION);
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::DoRemotePersistTcpError:some other error = " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
	}

	pThis->persist_connection_has_ = false;
	pThis->persist_connection_libevent_ = false;

	bufferevent_free(bev);
}

int CNetCoreLayer::ClosePersistConnection()
{
	int ret = evutil_closesocket(persist_sfd_);
	if (ret != 0)
	{
		LOG4CXX_ERROR(g_logger, " CNetCoreLayer::ClosePersistConnection error = " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
	}
	persist_connection_has_ = false;
	return ret;
}

int CNetCoreLayer::connect_nonb( int sockfd, struct sockaddr* saptr, socklen_t salen, int nsec )
{
	int flags, n, error = 0;
	socklen_t len;
	fd_set rset, wset;
	struct timeval tval;

	flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	if ((n = ::connect(sockfd, saptr, salen) < 0))
	{
		if (EINPROGRESS != errno)
		{
			LOG4CXX_ERROR(g_logger, "CNetCoreLayer::connect_nonb:connect failed. errorcode = " << errno);
			return (-1);
		}
	}

	if(0 == n)
		goto done;

	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	wset = rset;
	tval.tv_sec = nsec;
	tval.tv_usec = 0;

	if ((n = select(sockfd+1, &rset, &wset, NULL, nsec?&tval:NULL)) == 0)
	{
		close(sockfd);
		errno = ETIMEDOUT;
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::connect_nonb:select failed. errorcode = " << errno);
		return -1;
	}

	if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset))
	{
		len = sizeof(error);
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&error, &len) < 0)
		{
			LOG4CXX_ERROR(g_logger, "CNetCoreLayer::connect_nonb:getsockopt failed. errorcode = " << errno);
			return -1;
		}
	}
	else
		LOG4CXX_FATAL(g_logger, "CNetCoreLayer::connect_nonb:select error: sockfd not set");

done:
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
	if (error)
	{
		close(sockfd);
		errno = error;
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::connect_nonb:ioctlsocket1 failed. errorcode = " << errno);
		return -1;
	}

	return 0;
}


bool CNetCoreLayer::SetTCP_NODELAY(evutil_socket_t sfd)
{
	int flags = 1;
	int nErr = setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (void *) &flags, sizeof(flags));
	if (-1 == nErr)
	{
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::connect_nonb:SetTCP_NODELAY failed. error = " << strerror(errno));
		return false;
	}

	return true;
}

bool CNetCoreLayer::GetIpByDomain(const std::string& domain, std::string& ip)
{
	struct hostent *st_host = NULL;
	st_host = gethostbyname(domain.c_str());
	if (NULL == st_host)
	{
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::GetIpByDomain failed.domain = " << domain
								<<" errorCode = " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
		return false;
	}

	ip = inet_ntoa(*(struct in_addr *)(st_host->h_addr));

	return true;
}

















#include "net_core_layer.h"
#include <assert.h>
#include <sys/types.h>
#include "message.h"
#include "net_inter_layer.h"
#include "../common/config_file.h"
#include "../common/defines.h"
#include "../common/utils.h"


bool CNetCoreLayer::persist_connection_has_ = false;
bool CNetCoreLayer::persist_connection_libevent_ = false;

inline bool SetTCP_NODELAY(evutil_socket_t sfd)
{
	const char chOpt=1;   
	int   nErr=setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char));   
	if(-1 == nErr)   
	{   
		return false;   
	} 
	
	return true;
}

CNetCoreLayer::CNetCoreLayer(void)
{
	local_read_event_ = NULL;
	base_ = NULL;
	pNetInterLayer_ = NULL;
	memset(&inData_persist_conn_, 0, sizeof(inData_persist_conn_));
}

CNetCoreLayer::~CNetCoreLayer(void)
{
	event_del(local_read_event_);
	evutil_closesocket(pipe_[0]);
	evutil_closesocket(pipe_[1]);

	if(-1 == event_base_loopexit(base_, NULL))
	{
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::~CNetCoreLayer:event_base_loopexit error.");
	}

	WSACleanup();

	event_base_free(base_);
}

bool CNetCoreLayer::InitNetCore(CNetInterLayer* pNetInterLayer)
{
	pNetInterLayer_= pNetInterLayer;

	const char* libevent_version = event_get_version();
	LOG4CXX_TRACE(g_logger, "CNetCoreLayer::InitNetCore:libevent version = " << libevent_version);

	base_ = event_base_new();
	assert(base_ != NULL);

	WSADATA  Ws;
	if ( WSAStartup(MAKEWORD(2,2), &Ws) != 0 )
	{
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::InitNetCore failed. errorCode = " << WSAGetLastError());
        return false;
	}

	int ret = pipe(pipe_);
	if (0 != ret)
	{
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::InitNetCore:pipe errorCode = " << ret);
		return false;
	}

	std::string basicinfo_server_domain = utils::G<ConfigFile>().read<std::string>("basicinfo.server.ip", "cloudsee.jovemail.com");
	std::string business_server_domain	= utils::G<ConfigFile>().read<std::string>("business.server.ip", "cloudsee.jovemail.com");

	/* 风险： 域名解析调用为阻塞调用，最好把配置ip暴漏在业务层 */
	if (!GetIpByDomain(basicinfo_server_domain, basicinfo_server_ip_))
	{
		return false;
	}

	if (!GetIpByDomain(business_server_domain, business_server_ip_))
	{
		return false;
	}
		

	LOG4CXX_TRACE(g_logger, "CNetCoreLayer::InitNetCore basicinfo_server_ip = " << basicinfo_server_ip_ << ", business_server_ip = " << business_server_ip_);

	return true;
}

void CNetCoreLayer::Run()
{
	local_read_event_ = event_new(base_, pipe_[1], EV_READ|EV_PERSIST, DoLocalRead, (void*)this);
	assert(local_read_event_ != NULL);
	event_add(local_read_event_, NULL);

	/* 通知上层初始化完成*/
	if (!::SetEvent(pNetInterLayer_->initSdk_done_event_))
	{
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::Run:SetEvent failed. errorCode = " << GetLastError());	
	}

	LOG4CXX_INFO(g_logger, "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^CNetCoreLayer::Run....^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");

	int i = event_base_dispatch(base_);

	LOG4CXX_INFO(g_logger, "CNetCoreLayer::event_base_dispatch = " << i);
}


void CNetCoreLayer::DoLocalRead(evutil_socket_t local_tcp_server, short event, void *arg)
{
	CNetCoreLayer* pThis = static_cast<CNetCoreLayer*>(arg);


	char recv_buf[1];
	int size = recv(local_tcp_server, recv_buf, 1, 0 );
	if (size != 1)
	{
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::DoLocalRead recv error. errorCode = " << WSAGetLastError());
		return;
	}

	REQUEST_INFO requestInfo;
	if(!pThis->threadSafeList_.pop_front(requestInfo))
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

void CNetCoreLayer::ShortConnectionOpt( const REQUEST_INFO& requestInfo)
{
	short flag = EV_READ | EV_PERSIST;
	IN_DATA* ptr_inData;
	try
	{
		ptr_inData = new IN_DATA;
	}
	catch(std::bad_alloc &)
	{
		LOG4CXX_WARN(g_logger, "CNetCoreLayer::ShortConnectionOpt:IN_DATA not enough memory!");
		return;
	}

	ptr_inData->client_tcp_sock = requestInfo.sfd;
	ptr_inData->tcp_connect_type= requestInfo.tcp_connect_type;
	ZeroMemory(ptr_inData->buf, BUF_SIZE);
	ptr_inData->buf_len			= 0;
	ptr_inData->base_ptr		= pNetInterLayer_;

	struct bufferevent *short_tcp_recv_event = bufferevent_socket_new(this->base_, requestInfo.sfd, BEV_OPT_CLOSE_ON_FREE);
	if(short_tcp_recv_event == NULL)
	{
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::ShortConnectionOpt:bufferevent_socket_new");
		utils::SafeDelete(ptr_inData);
		::closesocket(requestInfo.sfd);
		return;
	}
	bufferevent_setcb(short_tcp_recv_event, DoRemoteShortTcpRead, NULL, DoRemoteShortTcpError, (void*)ptr_inData);

	struct timeval read_sec;
	read_sec.tv_sec	= utils::G<ConfigFile>().read<int>("request.timeout.ms", 10000)/1000;
	read_sec.tv_usec	= 0;
	bufferevent_set_timeouts(short_tcp_recv_event, &read_sec, NULL);

	bufferevent_enable(short_tcp_recv_event, flag);

	/** 向服务端发送数据 (对特殊字符进行转义处理\r\n) */
	std::string request_append = utils::ReplaceString(requestInfo.message, "\\r\\n", "\\\\r\\\\n");
	request_append = request_append + std::string(CRLF);
	int send_size = send(requestInfo.sfd, request_append.c_str(), request_append.length(), 0);
	if (send_size < 0)
	{
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::AddShortConnectionResquest:send failed. errorcode = " << WSAGetLastError());
		::closesocket(requestInfo.sfd);
		return;
	}
}

void CNetCoreLayer::PersistConnectionOpt( const REQUEST_INFO& requestInfo )
{
	short flag = EV_READ | EV_PERSIST;
	struct bufferevent *persist_tcp_recv_event = bufferevent_socket_new(this->base_, requestInfo.sfd, BEV_OPT_CLOSE_ON_FREE);
	if(persist_tcp_recv_event == NULL)
	{
		LOG4CXX_FATAL(g_logger, "CNetCoreLayer::PersistConnectionOpt!!!!!!");
		return;
	}
	bufferevent_setcb(persist_tcp_recv_event, DoRemotePersistTcpRead, NULL, DoRemotePersistTcpError, (void*)this);
	bufferevent_enable(persist_tcp_recv_event, flag);
	persist_connection_libevent_ = true;
}

int CNetCoreLayer::AddShortConnectionResquest(const std::string& request )
{
	/** 如果是短连接，每次生成一个生 新的tcp socket 向服务端发送数据 */
	evutil_socket_t short_sfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(short_sfd > 0);

	/** 初始化连接 */
	struct sockaddr_in server_addr_;
	ZeroMemory(&server_addr_, sizeof(server_addr_));
	server_addr_.sin_family = AF_INET;
	std::string server_ip = basicinfo_server_ip_;
	int port = utils::G<ConfigFile>().read<int>("basicinfo.server.port", 12002);
	server_addr_.sin_addr.s_addr = inet_addr(server_ip.c_str());
	server_addr_.sin_port = htons(port);

	int ret = 0;
	int timeout_sec = utils::G<ConfigFile>().read<int>("request.timeout.ms", 10000)/1000;
	if((ret = connect_nonb(short_sfd, (struct sockaddr *)&server_addr_, sizeof(server_addr_), timeout_sec)) < 0)
	{
		if (-2 == ret)
		{
			::closesocket(short_sfd);
			return CANT_CONNECT_SERVER;
		}
		else
		{
			::closesocket(short_sfd);
			return CONN_OTHER_ERROR;
		}
	}

	if(!SetTCP_NODELAY(short_sfd))   
	{   
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::AddShortConnectionResquest:setsockopt(TCP_NODELAY) short_sfd failed. errorCode = " <<WSAGetLastError());   
		::closesocket(short_sfd);
		return CONN_OTHER_ERROR;   
	} 

	/** 通知 libevent线程，加入监控此socket */
	REQUEST_INFO requestInfo;
	requestInfo.sfd		= short_sfd;
	requestInfo.tcp_connect_type = SHORT_CONNECTION;
	requestInfo.message	= std::string(TOKEN_STR) + request;

	threadSafeList_.push_back(requestInfo);
	int size = send(pipe_[0], CONNECTION_FLAG, 1, 0);
	if (size < 0)
	{
		LOG4CXX_WARN(g_logger, "CNetCoreLayer::AddShortConnectionResquest:send error. errorCode = " << WSAGetLastError());
		::closesocket(short_sfd);
		return CONN_OTHER_ERROR;
	}

	return SUCCESS;
}

int CNetCoreLayer::AddPersistConnectionRequest(const std::string& request )
{

	std::string request_add_token = request;

	/** 如果是长连接，只生成一个tcp socket 向服务端发送数据 */
	if (!persist_connection_has_)
	{
		request_add_token = std::string(TOKEN_STR) + request_add_token;
		persist_sfd = socket(AF_INET, SOCK_STREAM, 0);
		assert(persist_sfd != NULL);

		/** 初始化连接 */
		struct sockaddr_in server_addr_;
		ZeroMemory(&server_addr_, sizeof(server_addr_));
		server_addr_.sin_family = AF_INET;
		std::string server_ip = business_server_ip_;
		int port = utils::G<ConfigFile>().read<int>("business.server.port", 12004);
		server_addr_.sin_addr.s_addr = inet_addr(server_ip.c_str());
		server_addr_.sin_port = htons(port);

		int ret = 0;
		int timeout_sec = utils::G<ConfigFile>().read<int>("request.timeout.ms", 10000)/1000;
		if((ret = connect_nonb(persist_sfd, (struct sockaddr *)&server_addr_, sizeof(server_addr_), timeout_sec)) < 0)
		{
			if (-2 == ret)
			{
				::closesocket(persist_sfd);
				return CANT_CONNECT_SERVER;
			}
			else
			{
				::closesocket(persist_sfd);
				return CONN_OTHER_ERROR;
			}
		}

		if(!SetTCP_NODELAY(persist_sfd))   
		{   
			LOG4CXX_ERROR(g_logger, "CNetCoreLayer::AddPersistConnectioRequest:setsockopt(TCP_NODELAY) persist_sfd failed. errorCode = " << WSAGetLastError());
			::closesocket(persist_sfd);
			return CONN_OTHER_ERROR;   
		}  

		persist_connection_has_ = true;


		/** 通知 libevent线程，加入监控此socket */
		REQUEST_INFO requestInfo;
		requestInfo.sfd		= persist_sfd;
		requestInfo.tcp_connect_type = PERSIST_CONNECTION;
		requestInfo.message	= request_add_token;
		threadSafeList_.push_back(requestInfo);
		int size = send(pipe_[0], CONNECTION_FLAG, 1, 0);
		if (size < 0)
		{
			LOG4CXX_ERROR(g_logger, "CNetCoreLayer::AddPersistConnectioRequest:send error. errorCode = " << WSAGetLastError());
			::closesocket(persist_sfd);
			return CONN_OTHER_ERROR;
		}


		LOG4CXX_INFO(g_logger, "CNetCoreLayer::AddPersistConnectioRequest:persist_connection_has_ = " << persist_connection_has_);

		inData_persist_conn_.client_tcp_sock	= persist_sfd;
		inData_persist_conn_.tcp_connect_type	= PERSIST_CONNECTION;
		ZeroMemory(inData_persist_conn_.buf, BUF_SIZE);
		inData_persist_conn_.buf_len = 0;
	}

	std::string request_append = utils::ReplaceString(request_add_token, "\\r\\n", "\\\\r\\\\n");
	request_append = request_append + std::string(CRLF);
	int send_size = send(persist_sfd, request_append.c_str(), request_append.length(), 0);
	if (send_size < 0)
	{
		LOG4CXX_WARN(g_logger, "CNetCoreLayer::AddPersistConnectioRequest:send failed. errorcode = " << WSAGetLastError());
		return CONN_OTHER_ERROR;
	}

	return SUCCESS;

}

void CNetCoreLayer::DoRemoteShortTcpRead(struct bufferevent *bev, void *arg)
{
	IN_DATA* ptr_inData = static_cast<IN_DATA*>(arg);

	int recv_size = 0;
	if((recv_size = bufferevent_read(bev, ptr_inData->buf + ptr_inData->buf_len, RECIVE_BUF_SIZE - ptr_inData->buf_len)) > 0)
	{
		ptr_inData->buf_len = ptr_inData->buf_len + recv_size;
	}

	std::string str_recv(ptr_inData->buf, ptr_inData->buf_len);
	if(utils::FindCRLF(str_recv))
	{
		bufferevent_free(bev);

		/** 回调业务层接口，将收到回复数据返回给业务层 */
		str_recv = str_recv.substr(0, str_recv.find(CRLF));
		ptr_inData->base_ptr->ReciveData(str_recv, ptr_inData->tcp_connect_type);
		utils::SafeDelete(ptr_inData);
		return;
	}
}

void CNetCoreLayer::DoRemoteShortTcpError( struct bufferevent *bev,short event, void *arg )
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
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::DoRemoteShortTcpError:some other error = " << GetLastError());
	}

	utils::SafeDelete(ptr_inData);
	bufferevent_free(bev); 
}

void CNetCoreLayer::DoRemotePersistTcpRead(struct bufferevent *bev, void *arg)
{
	CNetCoreLayer* pThis = static_cast<CNetCoreLayer*>(arg);

	int recv_size = 0;
	if((recv_size = bufferevent_read(bev, pThis->inData_persist_conn_.buf + pThis->inData_persist_conn_.buf_len, RECIVE_BUF_SIZE - pThis->inData_persist_conn_.buf_len)) > 0)
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

void CNetCoreLayer::DoRemotePersistTcpError( struct bufferevent *bev, short event, void *arg )
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
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::DoRemotePersistTcpError:some other error = " << GetLastError());
	}

	pThis->persist_connection_has_ = false;
	pThis->persist_connection_libevent_ = false;

	bufferevent_free(bev); 
}

int CNetCoreLayer::pipe(int fildes[2])
{
	int local_tcp_client, local_tcp_server;
	sockaddr_in localAddr;
	int ret = 0;

	memset(&localAddr, 0, sizeof(localAddr));
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	int namelen = sizeof(localAddr);

	local_tcp_client = local_tcp_server = -1;
	int local_tcp_listen = socket(AF_INET, SOCK_STREAM, 0);
	if (local_tcp_listen == -1)
	{
		ret = 1;
		goto clean;
	}
	if (bind(local_tcp_listen, (sockaddr*)&localAddr, namelen) == -1)
	{
		ret = 2;
		goto clean;
	}
	if (listen(local_tcp_listen, 5) == -1)
	{
		ret = 3;
		goto clean;
	}
	if (getsockname(local_tcp_listen, (sockaddr*)&localAddr, &namelen) == -1)
	{
		ret = 4;
		goto clean;
	}

	local_tcp_client = socket(AF_INET, SOCK_STREAM, 0);
	if (local_tcp_client == -1)
	{
		ret = 5;
		goto clean;
	}
	if (-1 == connect(local_tcp_client, (sockaddr*)&localAddr, namelen))
	{
		ret = 6;
		goto clean;
	}
	if(!SetTCP_NODELAY(local_tcp_client))   
	{   
		ret = 7;
		goto clean;  
	} 


	local_tcp_server = accept(local_tcp_listen, (sockaddr*)&localAddr, &namelen);
	if (local_tcp_server == -1)
	{
		ret = 8;
		goto clean;
	}
	if (closesocket(local_tcp_listen) == -1)
	{
		ret = 9;
		goto clean;
	}

	fildes[0] = local_tcp_client;
	fildes[1] = local_tcp_server;

	return ret;

clean:
	if (local_tcp_listen != -1)
	{
		closesocket(local_tcp_listen);
	}
	if (local_tcp_server != -1)
	{
		closesocket(local_tcp_server);
	}
	if (local_tcp_client != -1)
	{
		closesocket(local_tcp_client);
	}
	return ret;
}

int CNetCoreLayer::connect_nonb( int sockfd, struct sockaddr* saptr, socklen_t salen, int nsec )
{
	int ret, n, error = 0;
	socklen_t len;
	FD_SET rset, wset;
	struct timeval tval;
	unsigned long ul=1;  
	unsigned long ul1=0;

	ret = ioctlsocket(sockfd, FIONBIO, (unsigned long *)&ul);	//设置成非阻塞模式
	if (SOCKET_ERROR == ret)
	{
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::connect_nonb:ioctlsocket failed. errorcode = " << WSAGetLastError());
		return -1;
	}

	if ((n = ::connect(sockfd, saptr, salen) < 0))
	{
		error = WSAGetLastError();
		if (WSAEWOULDBLOCK != error)
		{
			LOG4CXX_ERROR(g_logger, "CNetCoreLayer::connect_nonb:connect failed. errorcode = " << error);
			return -1;
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
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::connect_nonb:select failed. errorcode = " << WSAGetLastError());
		::closesocket(sockfd);
		error = WSAETIMEDOUT;
		return -2;
	}

	if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset))
	{
		len = sizeof(error);
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char*)&error, &len) < 0)
		{
			LOG4CXX_ERROR(g_logger, "CNetCoreLayer::connect_nonb:getsockopt failed. errorcode = " << WSAGetLastError());
			return -1;
		}
	}
	else
	{
		LOG4CXX_FATAL(g_logger, "CNetCoreLayer::connect_nonb:FD_ISSET");
		return -1;
	}
	
done:
	ret = ioctlsocket(sockfd, FIONBIO, (unsigned long*)&ul1);
	if (SOCKET_ERROR == ret)
	{
		::closesocket(sockfd);
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::connect_nonb:ioctlsocket1 failed. errorcode = " << WSAGetLastError());
		return -1;
	}

	return 0;
}

int CNetCoreLayer::ClosePersistConnection()
{
	int ret =  evutil_closesocket(persist_sfd);	
	if (ret != 0)
	{
		LOG4CXX_ERROR(g_logger, " CNetCoreLayer::ClosePersistConnection errorcode = " << WSAGetLastError());
	}
	persist_connection_has_ = false;
	return ret;
}

bool CNetCoreLayer::GetIpByDomain(const std::string& domain, std::string& ip)
{
	struct hostent *st_host = NULL;
	st_host = gethostbyname(domain.c_str());
	if (NULL == st_host)
	{
		LOG4CXX_ERROR(g_logger, "CNetCoreLayer::GetIpByDomain failed.domain = " << domain <<" errorCode = " << WSAGetLastError());
		return false;
	}

	ip = inet_ntoa(*(struct in_addr *)(st_host->h_addr));

	return true;
}



















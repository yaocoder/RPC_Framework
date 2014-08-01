#ifndef net_core_h__
#define net_core_h__

#include <string>
#include <WS2tcpip.h>  
#include <event2/event.h>
#include <event2/bufferevent.h>
#include "../common/threadSafe_list.h"

#define  BUF_SIZE		 1024*10
#define  RECIVE_BUF_SIZE 1024*10

class CNetInterLayer;
typedef struct in_data
{
	evutil_socket_t client_tcp_sock;
	int	  tcp_connect_type;
	char  buf[BUF_SIZE];
	int	  buf_len;
	CNetInterLayer* base_ptr;
}IN_DATA;


typedef struct
{
	evutil_socket_t sfd;
	int tcp_connect_type;
	std::string message;
}REQUEST_INFO;


#define CONNECTION_FLAG	"c"


class CNetCoreLayer
{
public:
	CNetCoreLayer(void);
	virtual ~CNetCoreLayer(void);

	bool InitNetCore(CNetInterLayer* pNetInterLayer);

	void Run();

	int AddShortConnectionResquest(const std::string& request);

	int AddPersistConnectionRequest(const std::string& request);

	int ClosePersistConnection();

private:

	CThreadSafeList<REQUEST_INFO> threadSafeList_;

	int pipe(int fildes[2]);

	int connect_nonb(int sockfd, struct sockaddr* saptr, socklen_t salen, int nsec);

	bool GetIpByDomain(const std::string& domain, std::string& ip);

	static bool persist_connection_has_;

	static bool persist_connection_libevent_;

	static void DoLocalRead(evutil_socket_t udp_listener, short event, void *arg);
	void ShortConnectionOpt(const REQUEST_INFO& requestInfo);
	void PersistConnectionOpt(const REQUEST_INFO& requestInfo);

	static void DoRemoteShortTcpRead(struct bufferevent *bev, void *arg);
	static void DoRemoteShortTcpError(struct bufferevent *bev, short event, void *arg);


	static void DoRemotePersistTcpRead(struct bufferevent *bev, void *arg);
	static void DoRemotePersistTcpError(struct bufferevent *bev, short event, void *arg);


	struct event		*local_read_event_;

	struct event_base	*base_;

	CNetInterLayer*		pNetInterLayer_;

	evutil_socket_t		persist_sfd;

	IN_DATA				inData_persist_conn_;

	int					error_code_;

	int					pipe_[2];

	std::string			basicinfo_server_ip_;
	std::string			business_server_ip_;
	

};
#endif // net_core_h__

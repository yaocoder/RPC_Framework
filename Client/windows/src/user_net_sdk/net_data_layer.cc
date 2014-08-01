#include "net_data_layer.h"
#include "message.h"
#include "../common/utils.h"

#define MESSAGE_ID			"message_id"
#define SOCKET_FD			"socket_fd"
#define TCP_CONNECT_TYPE    "tcp_connect_type"

CNetDataLayer::CNetDataLayer(void)
{
}

CNetDataLayer::~CNetDataLayer(void)
{
}

bool CNetDataLayer::VerifyCommonJson(JSONNode& in)
{
	return VerifyJsonField(in, JK_P2RELAY_MESSAGE_TYPE) && VerifyJsonField(in, JK_RESULT);
}

bool CNetDataLayer::VerifyJsonField(JSONNode& in, std::string field)
{
	try
	{
		return (in.end() != in.find(field));
	}
	catch (...)
	{
		return false;
	}
}

bool CNetDataLayer::JsonParseResult( const std::string in_json_string, int& result )
{
	JSONNode in;
	try
	{
		in = libjson::parse(in_json_string);
	}
	catch(...)
	{
		LOG4CXX_ERROR(g_logger, "CNetDataLayer::JsonParseResult std::invalid_argument");
		return false;
	}

	if (!VerifyCommonJson(in))
	{
		return false;
	}

	result = in[JK_RESULT].as_int();

	return true;
}

std::string CNetDataLayer::JsonJoinGetLiveStatus( const int message_id )
{
	JSONNode out;
	JsonJoinImDirectPublic(message_id, HEATBEAT_DETECT, out);
	return out.write();
}

bool CNetDataLayer::JsonParseMessageId( const std::string& in_json_string, int& message_id )
{
	JSONNode in;
	try
	{
		in = libjson::parse(in_json_string);
	}
	catch(...)
	{
		LOG4CXX_ERROR(g_logger, "CNetDataLayer::JsonParseMessageId std::invalid_argument");
		return false;
	}

	if (!VerifyJsonField(in, JK_MESSAGE_ID))
	{
		return false;
	}

	message_id = in[JK_MESSAGE_ID].as_int();

	return true;
}

void CNetDataLayer::JsonJoinImDirectPublic( const int message_id, const int message_type, JSONNode& out )
{
	out.push_back(JSONNode(JK_MESSAGE_ID, message_id));
	out.push_back(JSONNode(JK_P2RELAY_MESSAGE_TYPE, message_type));
	out.push_back(JSONNode(JK_PROTO_VERSION, PROTO_VERSION));
	out.push_back(JSONNode(JK_LOGIC_PROCESS_TYPE, ALARM_SERVER_RELAY));
}

std::string CNetDataLayer::JsonJoinGetResponseByRequest(const int message_id, const std::string& custom_string)
{
	JSONNode out;
	out.push_back(JSONNode(JK_MESSAGE_ID, message_id));
	out.push_back(JSONNode(JK_PROTO_VERSION, PROTO_VERSION));
	out.push_back(JSONNode(JK_CUSTOM_STRING, custom_string));

	return out.write();
}

bool CNetDataLayer::JsonParseGetResponseByRequest(const std::string& in_json_string, int& result, std::string& custom_string)
{
	JSONNode in;
	try
	{
		in = libjson::parse(in_json_string);
	}
	catch (...)
	{
		LOG4CXX_ERROR(g_logger, "CNetDataLayer::JsonParseGetResponseByRequest std::invalid_argument");
		return false;
	}

	if(!VerifyJsonField(in, JK_RESULT))
		return false;

	result = in[JK_RESULT].as_int();

	if(!VerifyJsonField(in, JK_CUSTOM_STRING))
		return false;

	custom_string = in[JK_CUSTOM_STRING].as_string();

	return true;
}

std::string CNetDataLayer::JsonJoinSendAsynRequest(const int asyn_request_id, const std::string& request)
{
	JSONNode out;
	out.push_back(JSONNode(JK_ASYN_REQUEST_ID, asyn_request_id));
	out.push_back(JSONNode(JK_ASYN_REQUEST, request));
	return out.write();
}
#ifndef json_opt_h__
#define json_opt_h__
#include <libjson/libjson.h>
#include <libjson/Source/JSONNode.h>
#include "../common/defines.h"
#include "user_interface_defines.h"

class CNetDataLayer
{
public:
	CNetDataLayer(void);
	~CNetDataLayer(void);


	bool		JsonParseResult(const std::string in_json_string, int& result);

	std::string JsonJoinGetLiveStatus(const int message_id);

	bool		JsonParseMessageId(const std::string& in_json_string, int& message_id);

	std::string JsonJoinGetResponseByRequest(const int message_id, const std::string& custom_string);
	bool JsonParseGetResponseByRequest(const std::string& in_json_string, int& result, std::string& custom_string);

	std::string JsonJoinSendAsynRequest(const int asyn_request_id, const std::string& request);

private:

	bool VerifyCommonJson(JSONNode& in);

	bool VerifyJsonField(JSONNode& in, std::string field);


	void JsonJoinImDirectPublic(const int message_id, const int message_type, JSONNode& out);

	void JsonJoinImRelayPublic( const int message_id, const int message_type, JSONNode& out );

	std::string session_id_;

};
#endif // json_opt_h__
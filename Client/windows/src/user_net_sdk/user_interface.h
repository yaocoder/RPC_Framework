#ifndef user_interface_h__
#define user_interface_h__

#include "user_interface_defines.h"

#ifndef UNIT_TEST

#ifdef _DLL_MODE
#define DLL_MODE_API __declspec(dllexport)
#else
#define DLL_MODE_API __declspec(dllimport)
#endif

#endif


/*注册的推送接口，使用者可以自己实现内部逻辑 */
class IPushMessageOpt
{
public:
	virtual ~IPushMessageOpt() {};

	/**
	 * @brief sdk内部推送出来的消息(包括sdk本身的出错消息，及连接异常消息）
	 * @param 消息类型
	 * @param 消息内容
	 */
	virtual void LocalPushMessageOpt(const int message_type, const PUSH_INFO& pushInfo) = 0;

	/**
	 * @brief 维持心跳
	 * @param 心跳状态 SUCCESS为正常（处理死连接）
	 */
	virtual void LiveStatusCB(const int ret) = 0;

	/**
	 * @brief 异步请求的消息回应
	 * @param 异步过程标识
	 * @param 消息内容
	 */
	virtual void AsynServerResponseOpt(const int asyn_id, const PUSH_INFO& pushInfo) = 0;

	/**
	 * @brief 服务端主动推送的消息
	 * @param 消息类型
	 * @param 消息内容
	 */
	virtual void ServerPushMessageOpt(const int message_type, const PUSH_INFO& pushInfo) = 0;
};


#ifndef UNIT_TEST
class  DLL_MODE_API IUserInterface
#else
class  IUserInterface
#endif
{
public:
	virtual ~IUserInterface() {}

	static IUserInterface* GetInstance();

	static void DestroyImplInstance(const IUserInterface* pImpl);

	/**
	 * @brief 初始化SDK（在此接口设计中，相关配置参数全部通过配置文件加载）
	 */
	virtual bool InitSDK() = 0;

	virtual void UninitSDK() = 0;


	/**
	 * @brief 注册推送逻辑（在Init成功后必须调用）
	 * @param [in] 见类IPushMessageOpt
	 */
	virtual void RegisterPushFunc(IPushMessageOpt* pPushMessageOpt) = 0;

	/**
	* @brief 和服务端建立起长连接
	*/
	virtual int EstablishPersistentChannel() = 0;

	/**
	 * @brief 远程过程调用接口实现(长连接，阻塞接口）
	 * @param [in]	发送请求（请求只要与服务端协议对应即可）
	 * @param [out] 回应
	 */
	virtual int GetResponseByRequestPersistentConnection(const std::string& request, std::string& response) = 0;

	/**
	 * @brief 远程过程调用接口实现(短连接，阻塞接口）
	 * @param [in]	发送请求（请求只要与服务端协议对应即可）
	 * @param [out] 回应
	 */
	virtual int GetResponseByRequestShortConnection(const std::string& request, std::string& response) = 0;


	/**
	 * @brief 发送异步请求(发送后立马返回，非阻塞接口，在IPushMessageOpt:AsynServerResponseOpt中根据异步过程标识处理回应）
	 * @param [in]	异步过程标识(必须保证唯一）
	 * @param [in]  请求
	 */
	virtual int SendAsynRequest(const int asyn_request_id, const std::string& request) = 0;


private:

	static IUserInterface* instance_;

};

#endif // user_interface_h__
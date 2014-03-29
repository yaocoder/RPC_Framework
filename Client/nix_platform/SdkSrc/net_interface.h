/*****************************************************************************
**
** Copyright (C) 2014 yaocoder.
** All rights reserved.
** @author : yaowei
** @Contact: yaocoder@gmail.com
**
** @brief: This file is network interface library for  client
**
****************************************************************************/
#ifndef user_interface_h__
#define user_interface_h__

#include "net_interface_defines.h"

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

class  INetChannelInterface
{
public:

	virtual ~INetChannelInterface() {}

	static INetChannelInterface* GetImplInstance();

	static void DestroyImplInstance(const INetChannelInterface* pImpl);

	/**
	* @brief 初始化接口SDK（初始化日志，网络配置）
	*/
#ifndef USE_LOG4CXX
	virtual bool Init(const std::string& ip, const int port) = 0;
#else
	#ifdef USE_LOG4CXX_PTR
		virtual bool Init(const std::string& ip, const int port, const LoggerPtr loggerPrt) = 0;
	#else
		virtual bool Init(const std::string& ip, const int port, const std::string& log_path) = 0;
	#endif
#endif

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
	 * @brief 远程过程调用接口实现(长连接）
	 * @param [in]	发送请求（请求只要与服务端协议对应即可）
	 * @param [out] 回应
	 */
	virtual int GetResponseByRequestPersistentConnection(const std::string& request, std::string& response) = 0;

	/**
	 * @brief 远程过程调用接口实现(短连接）
	 * @param [in]	发送请求（请求只要与服务端协议对应即可）
	 * @param [out] 回应
	 */
	virtual int GetResponseByRequestShortConnection(const std::string& request, std::string& response) = 0;



private:

	static INetChannelInterface*  instance_;

};

#endif // user_interface_h__

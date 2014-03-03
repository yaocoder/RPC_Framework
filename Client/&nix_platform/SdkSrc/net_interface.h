/*****************************************************************************
**
** Copyright (C) 2014 yaocoder Corporation and/or its subsidiary(-ies).
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
	* @brief 用户上线
	* @param [in] 心跳状态回调函数(心跳检测与服务器的连接状态是否正常,对服务器后台起到防止半开连接的作用），
				  需要在上层实现LiveStatusCB来实时判断与服务器通信是否正常，累计统计达到一定次数时认为和
				  服务器连接中断
	*/
	virtual int EstablishPersistentChannel(const LiveStatusCB liveStatusCb) = 0;

	/**
	* @brief 主动接收通道消息
	* @param [in] 推送消息回调函数（响应长连接关闭，长连接错误，或者对端推送消息）
	*/
	virtual void RegisterServerPushFunc(ServerPushCallBack_Info sp_cb_info) = 0;

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

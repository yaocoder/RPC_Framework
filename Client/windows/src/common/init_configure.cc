/*
 * Init_log4cxx.cc
 *
 *  Created on: 2012-9-12
 *      Author: yaowei
 */

#include "init_configure.h"
#include "config_file.h"
#include "defines.h"
#include "utils.h"
#include "EncodingConverter.h"

LoggerPtr g_logger;

CInitConfig::CInitConfig()
{

}

CInitConfig::~CInitConfig()
{

}

void CInitConfig::InitLog4cxx(const std::string& project_name, const bool is_need_configure, const std::string& log_conf_fullpath) 
{
	/* 根据调用进程来判定 */
	if (is_need_configure)
	{
		std::wstring ws_log_conf_fullpath = EncodingConverter::ToWideString(log_conf_fullpath.c_str());
		PropertyConfigurator::configure(ws_log_conf_fullpath);
	}

	g_logger = Logger::getLogger(project_name);

	LOG4CXX_INFO(g_logger, "Run.");
}

bool CInitConfig::LoadConfiguration(const std::string& conf_fullpath )
{
	std::locale old_locale = std::locale::global(std::locale(""));
	std::ifstream conf_file(conf_fullpath.c_str());
	std::locale::global(old_locale);
	if(!conf_file)
	{
	    LOG4CXX_ERROR(g_logger, "CInitConfig::LoadConfiguration failed. conf_fullpath = " << conf_fullpath);
		return false;
	}
	conf_file >> utils::G<ConfigFile>();
	return true;
}




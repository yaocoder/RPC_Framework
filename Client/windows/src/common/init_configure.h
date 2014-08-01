/*
 * init_log4cxx.h
 *
 *  Created on: 2012-9-12
 *      Author: yaowei
 */

#ifndef INIT_LOG4CXX_H_
#define INIT_LOG4CXX_H_

#include <string>


class CInitConfig {
public:
	CInitConfig();
	virtual ~CInitConfig();

public:

	void InitLog4cxx(const std::string& project_name, const bool is_need_configure, const std::string& log_conf_fullpath);

	bool LoadConfiguration(const std::string& conf_fullpath );

};

#endif /* INIT_LOG4CXX_H_ */

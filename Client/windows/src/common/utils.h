#ifndef SECPLATFORM_COMMON_UTILS_H
#define SECPLATFORM_COMMON_UTILS_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <vector>
#include <sstream> 
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

namespace utils
{


template <class T> 
class Singleton : private T
{
public:
	static T &Instance()
	{
		static Singleton<T> _instance;
		return _instance;
	}
private:
	Singleton(){}
	~Singleton(){}
};

template <typename T>
T& G()
{
	return Singleton<T>::Instance();
}

template<typename T>inline void SafeDelete(T*& p)
{
	if (NULL != p)
	{
		delete p; p = 0; 
	}
}

static inline void SplitData(const std::string& str, const std::string& delimiter, std::vector<std::string>& vec_data)
{
	std::string s = str;
	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delimiter)) != std::string::npos) 
	{
		token = s.substr(0, pos);
		vec_data.push_back(token);
		s.erase(0, pos + delimiter.length());
	}
}

static inline bool FindCRLF(const std::string& s)
{
	if(s.find("\r\n") != std::string::npos)
		return true;
	else
		return false;
}

static inline std::string ReplaceString(const std::string& str,
										const std::string& flag,
										const std::string& replaceFlag)
{
	boost::regex expressionReplace(flag);
	std::string strTemp = boost::regex_replace(str, expressionReplace, replaceFlag);
	return strTemp;
}

inline std::string JoinListBySeparator(const std::vector<std::string>&vec, const std::string& separator)
{
	std::string str;
	for(unsigned int i = 0; i < vec.size(); ++i)
	{
		str = str + vec.at(i) + separator;
	}

	return str;
}

inline std::string JoinStrByDelimiter(const std::vector<std::string>& vec_str, const std::string& delimiter)
{
	std::string str;
	std::vector<std::string>::const_iterator iter;
	for (iter = vec_str.begin(); iter != vec_str.end(); ++iter)
	{
		str = str + *iter + delimiter;
	}

	return str;
}


template<class T>
inline std::string t2string(T i)
{
	std::stringstream ss;
	std::string s;
	ss << i;
	s = ss.str();
	return s;
}

inline int string2int(std::string& str)  
{  
	std::istringstream in_stream(str);  
	int res;  
	in_stream >> res;  
	return res;  
} 

inline HMODULE GetCurrentModule()
{ // NB: XP+ solution!
	HMODULE hModule = NULL;
	GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCTSTR)GetCurrentModule,
		&hModule);

	return hModule;
}

//inline std::string NewFormattedGuid()
//{
//	GUID guid = GUID_NULL;
//	if (S_OK != ::CoCreateGuid(&guid))
//		throw (std::runtime_error("Could not generate guid."));
//
//	char buffer[100] = {0};
//	sprintf(buffer, "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
//		guid.Data1, guid.Data2, guid.Data3, 
//		guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
//		guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
//
//	return buffer;
//}

}

#endif // SECPLATFORM_COMMON_UTILS_H
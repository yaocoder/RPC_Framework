#ifndef threadSafe_list_h__
#define threadSafe_list_h__

#include <boost/thread.hpp>
#include <list>
#include <map>

template<typename T>
class CThreadSafeList
{

public:
	CThreadSafeList() {}
	~CThreadSafeList() 
	{
		if (!list_.empty())
		{
			list_.clear();
		}

	}

	void push_back(const T &pt) 
	{
		boost::mutex::scoped_lock oLock(mutex_);
		list_.push_back(pt);
	}

	bool pop_front(T &pt) 
	{
		boost::mutex::scoped_lock oLock(mutex_);
		if (list_.size() > 0) 
		{
			pt = list_.front();
			list_.pop_front();
			return true;
		}

		return false;
	}

	void erase(T &Object) 
	{
		boost::mutex::scoped_lock oLock(mutex_);
		list_.remove(Object);
	}

	void clear()
	{    
		boost::mutex::scoped_lock oLock(mutex_);
		if (!list_.empty())
		{
			list_.clear();
		}

		return;
	}

	int size() 
	{
		boost::mutex::scoped_lock oLock(mutex_);
		return list_.size();
	}

	bool empty()
	{
		boost::mutex::scoped_lock oLock(mutex_);
		return list_.empty();
	}


private:
	std::list<T> list_;
	boost::mutex mutex_;
};

template<typename K, typename V>
class CThreadSafeMap
{

public:
	CThreadSafeMap() {}
	~CThreadSafeMap() 
	{
		if (!map_.empty())
		{
			map_.clear();
		}
	}

	void insert(const K& key, const V& value)
	{
		boost::mutex::scoped_lock oLock(mutex_);
		map_.insert(std::pair<K, V>(key, value));
	}

	bool find(const K& key, V& value)
	{
		bool ret = false;
		boost::mutex::scoped_lock oLock(mutex_);
		if (map_.size() > 0)
		{
			typedef typename std::map<K, V>::iterator iter_thread;
			iter_thread iter= map_.find(key);
			if(iter != map_.end())
			{
				value = iter->second;
				ret = true;
			}
		}

		return ret;
	}

	bool findAndSet(const K& key, const V& new_value, V& old_value)
	{
		bool ret = false;
		boost::mutex::scoped_lock oLock(mutex_);
		if (map_.size() > 0)
		{
			typedef typename std::map<K, V>::iterator iter_thread;
			iter_thread iter= map_.find(key);
			if(iter != map_.end())
			{
				old_value = iter->second;
				map_.erase(iter);
				map_.insert(std::pair<K, V>(key, new_value));
				ret = true;
			}
		}

		return ret;
	}

	void erase(const K& key)
	{
		boost::mutex::scoped_lock oLock(mutex_);
		map_.erase(key);
	}

	void clear()
	{
		boost::mutex::scoped_lock oLock(mutex_);
		if (!map_.empty())
		{
			map_.clear();
		}

		return;
	}

	int size()
	{
		boost::mutex::scoped_lock oLock(mutex_);
		return map_.size();
	}

	bool empty()
	{
		boost::mutex::scoped_lock oLock(mutex_);
		return map_.empty();
	}

private:
	boost::mutex mutex_;
	std::map<K, V> map_;
};
#endif // threadSafe_list_h__
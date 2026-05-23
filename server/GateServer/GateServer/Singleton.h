#pragma once
#include "const.h"

template <typename T>
class Singleton 
{
protected:
	Singleton() = default;
	Singleton(const Singleton&) = delete;
	Singleton& operator =(const Singleton&) = delete;
	static std::shared_ptr<T> _instance;
public:
	static std::shared_ptr<T> GetInstance()
	{
		static std::once_flag _s_flag;
		std::call_once(_s_flag, [&]() {
			_instance = std::shared_ptr<T>(new T);
		});
		return _instance;
	}
	~Singleton()
	{
		std::cout << "this is singleton destruct" << std::endl;
	}

};
template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;
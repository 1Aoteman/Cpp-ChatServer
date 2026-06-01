#pragma once
#include <memory>
#include <mutex>
#include <iostream>
template <typename T>
class Singleton {
public:
	~Singleton() {
		std::cout << "µ¥ÀýÄ£°åÎö¹¹" << std::endl;
	}
	static std::shared_ptr<T>  GetInstance() {
		static std::once_flag _flag;
		std::call_once(_flag,[] {
			_instance = std::shared_ptr<T>(new T());
		});
		return _instance;
	}
protected:
	Singleton() = default;
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
	static std::shared_ptr<T> _instance;

};
template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;
#pragma once
#include <memory>
#include <iostream>
//创建一个单例模板类
template <typename T>
class SingleTon {
public:
	static std::shared_ptr<T> GetInstance() {
		static std::once_flag _s_flag;
		std::call_once(_s_flag,[&] {
			_instance = std::shared_ptr<T>(new T);
		
			});
		return _instance;
	}
	~SingleTon() {
		std::cout << "单例模板析构" << std::endl;
	}

protected:
	SingleTon() = default;
	SingleTon(const SingleTon&) = delete;
	SingleTon& operator=(const SingleTon&) = delete;
	static std::shared_ptr<T> _instance;
};
//static变量必须在类内初始化
template <typename T>
std::shared_ptr<T> SingleTon<T>::_instance = nullptr;
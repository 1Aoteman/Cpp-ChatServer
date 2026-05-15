// ChatServer1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "boost/asio.hpp"
#include "AsioIOServicePool.h"
#include "ConfigMgr.h"
#include "CServer.h"
int main()
{
    try {
        auto& con = ConfigMgr::Inst();
        auto pool = AsioIOServicePool::GetInstance();
        boost::asio::io_context ioc;
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc, pool](const boost::system::error_code& error, int signal_number) {
            pool->Stop();
            ioc.stop();
            });
        auto port_str = con["SelfServer"]["Port"];
        CServer s(ioc, atoi(port_str.c_str()));
        s.StartAccept();
        ioc.run();

    }
    catch (std::exception& e) {
        std::cout << "exception code is" << e.what() << std::endl;
    }
}
   



// GateServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "const.h"
#include <iostream>
#include "CServer.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"

int main()
{
    //TestRedisMgr();
    ConfigMgr &grpccon = ConfigMgr::Inst();
    std::string  gate_port_str = grpccon["GateServer"]["Port"];
    unsigned short gate_port = atoi(gate_port_str.c_str());
    try
    {
        
        net::io_context ioc{1};
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const boost::system::error_code& error, int signal_number) {
            if (error)
            {
                return;
            }
            ioc.stop();
        });
        std::make_shared<CServer>(ioc, gate_port)->Start();
        ioc.run();
    }
    catch (std::exception e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}




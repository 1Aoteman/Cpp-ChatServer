// ChatServer1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "boost/asio.hpp"
#include "AsioIOServicePool.h"
#include "ConfigMgr.h"
#include "CServer.h"
#include "RedisMgr.h"
#include "ChatServiceImpl.h"
int main()
{
    auto& conf = ConfigMgr::Inst();
    auto server_name = conf["SelfServer"]["Name"];
    try {
        
        RedisMgr::GetInstance()->HSet(LOGIN_COUNT,server_name,"0");
        //创捷grpc服务
        std::string server_address = conf["SelfServer"]["Host"] + ":" + conf["SelfServer"]["RPCPort"];
        ChatServiceImpl service;
        grpc::ServerBuilder builder;
        // 监听端口和添加服务
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        // 构建并启动gRPC服务器
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << "RPC Server listening on " << server_address << std::endl;
        //创建一个线程专门管理grpc服务
        std::thread _grpc_thread([&server]() {
            server->Wait();
            });

        auto pool = AsioIOServicePool::GetInstance();
        boost::asio::io_context ioc;
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc, pool,&server](const boost::system::error_code& error, int signal_number) {
            pool->Stop();
            ioc.stop();
            server->Shutdown();
            });
        auto port_str = conf["SelfServer"]["Port"];
        CServer s(ioc, atoi(port_str.c_str()));
        s.StartAccept();
        ioc.run();
        //如果服务器关掉，对应的redis信息也全部清空
        RedisMgr::GetInstance()->HDel(LOGIN_COUNT, server_name);
        RedisMgr::GetInstance()->Close();
        _grpc_thread.join();
    }
    catch (std::exception& e) {
        std::cout << "exception code is" << e.what() << std::endl;
    }
}
   



#include "VarifyGrpcClient.h"
#include "ConfigMgr.h"
GrpcconPool::GrpcconPool(std::size_t poolsize, std::string host, std::string port) :
_poolsize(poolsize),_host(host),_port(port)
{
	for (int i = 0; i < poolsize; ++i)//按照池子大小创建链接
	{
		std::shared_ptr<Channel> channel = grpc::CreateChannel(host+":"+port,
			grpc::InsecureChannelCredentials());
		_connections.push(VarifyService::NewStub(channel));//思考，为什么这里可以直接push，是拷贝构造还是什么？
	}	
}
GrpcconPool::~GrpcconPool()
{
	std::lock_guard<std::mutex> lock(_mutex);
	Close();
	while(!_connections.empty())
	{
		_connections.pop();
	}
}
std::unique_ptr<VarifyService::Stub> GrpcconPool::GetConnection(){
	std::unique_lock<std::mutex> lock(_mutex);
	_con.wait(lock, [this]()
		{
			if (_b_stop)
			{
				return true;
			}
			return !_connections.empty();

		});
	if (_b_stop)
	{
		return nullptr;
	}
	auto context = std::move(_connections.front());
	_connections.pop();
	return context;
}
void GrpcconPool::Close() {
	_b_stop = true;
	_con.notify_all();
}
void GrpcconPool::ReturnConnection(std::unique_ptr<VarifyService::Stub> context)
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (_b_stop) {
		return;
	}
	_connections.push(std::move(context));
	_con.notify_one();
}
VarifyGrpcClient::VarifyGrpcClient()
{
	auto& gCfgMgr = ConfigMgr::Inst();
	std::string host = gCfgMgr["VarifyServer"]["Host"];
	std::string port = gCfgMgr["VarifyServer"]["Port"];
	pool_.reset(new GrpcconPool(5, host, port));
}
#include "CSession.h"
#include "CServer.h"
#include "LogicSystem.h"
CSession::CSession(boost::asio::io_context &ioc,CServer* server):_socket(ioc),_cserver(server)
{
    // 1. 创建一个随机生成器对象
    boost::uuids::random_generator gen;
    // 2. 调用生成器，产生一个 UUID 对象
    boost::uuids::uuid u = gen();
    // 3. 将 UUID 转化为我们熟悉的字符串格式 
     _session_id = boost::uuids::to_string(u);
     //先读完头节点，在处理剩下的
     _recv_head_node = std::make_shared<MessageNode>(HEAD_TOTAL_LEN);

}
tcp::socket& CSession::GetSocket() {
    return _socket;
}
void CSession::SetUserId(int uid)
{
    _user_id = uid;
}
std::string& CSession::GetSessionId() {
    return _session_id;
}
int CSession::GetUserId() {
    return _user_id;
}
void CSession::Start()
{
    AsyncReadHead(HEAD_TOTAL_LEN);
}

void CSession::AsyncReadHead(int head_len)

{
    std::cout << "开始读取数据" << std::endl;
    auto self = shared_from_this();
    AsyncReadFull(HEAD_TOTAL_LEN, [self,this](boost::system::error_code ec,size_t bytetransfered) {
        try {
            if (ec) {
                std::cout << "read error is" << ec.what() << std::endl;
                Close();
                _cserver->ClearSession(_session_id);
                return;
            }
            if (bytetransfered < HEAD_TOTAL_LEN)//再次判断读到的数据头长度是否小于规定的长度
            {
                std::cout << "read length not match..." << std::endl;
                Close();
                _cserver->ClearSession(_session_id);
                return;
            }
            _recv_head_node->Clear();
            memcpy(_recv_head_node->_data, _data, HEAD_TOTAL_LEN);
            //取出msg_id
           short msg_id =0;
           memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);

           msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
           std::cout << "recv data msg_id is" << msg_id << std::endl;
           if (msg_id > MAX_LENGTH) {//判断msg_id是否合法
               std::cout << "msg id length not match..." << std::endl;
               _cserver->ClearSession(_session_id);
               return;
           }
           //取出msg_len
           short msg_len = 0;
           memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
           msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
           std::cout << "recv msg len is " <<msg_len<<std::endl;
           if (msg_len > MAX_LENGTH) {//判断msg_len是否合法
               std::cout << "msg len length not match..." << std::endl;
               Close();
               _cserver->ClearSession(_session_id);
               return;
           }
           //读完消息头开始读消息体
           _recv_msg_node = std::make_shared<RecvMsgNode>(msg_len, msg_id);
           AsyncReadBody(msg_len);
        }
        catch (std::exception& e) {
            std::cout << "Exception code is " << e.what() << std::endl;
        }
        });

}

void CSession::AsyncReadBody(short msg_len)
{
    auto self = shared_from_this();
    AsyncReadFull(msg_len,[msg_len,self,this](boost::system::error_code ec, size_t bytetransfered) {
        if (ec) {
            std::cout << "read msg body error is" << ec.what() << std::endl;
            Close();
            _cserver->ClearSession(_session_id);
            return;
        }
        if (bytetransfered<msg_len) {////再次判断读到的数据长度是否小于规定的长度
            std::cout << "read msg length is not match....." << ec.what() << std::endl;
            Close();
            _cserver->ClearSession(_session_id);
            return;
        }
        memcpy(_recv_msg_node->_data, _data, bytetransfered);
        _recv_msg_node->_cur_len += bytetransfered;
        _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
        std::cout << "receive data is " << _recv_msg_node->_data << std::endl;
        //此处将消息投递到逻辑队列中
        LogicSystem::GetInstance()->PostMsgToQue(std::make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
        //继续监听头部接受事件
        AsyncReadHead(HEAD_TOTAL_LEN);
        });
}

void CSession::AsyncReadFull(int max_len, std::function<void(boost::system::error_code err, size_t bytestransfered)> handler)
{
    //清空缓存中的数据
    memset(_data, 0, MAX_LENGTH);
    //读固定长度的数据
    AsyncReadlen(0, max_len, handler);

}
//read_len是已经读到的长度，而bytestansfered是read_some此次读到的长度
void CSession::AsyncReadlen(int read_len, int total_len, 
    std::function<void(boost::system::error_code err, size_t bytestransfered)> handler)
{
    auto self = shared_from_this();
    _socket.async_read_some(boost::asio::buffer(_data + read_len, total_len - read_len),
        [read_len,self,handler,total_len](boost::system::error_code ec, size_t bytestransfered){
            if (ec)//如果有错 
            {
                handler(ec,read_len+bytestransfered);
                return;
            }
            if (read_len + bytestransfered >= total_len) //如果读完的长度等于要读的长度
            {
                handler(ec, read_len + bytestransfered);
                return;
            }
            // 没有错误，且长度不足则继续读取
            self->AsyncReadlen(read_len + bytestransfered, total_len, handler);
    });
}
void CSession::Close() {
    _socket.close();
}

void CSession::Send(std::string msg,int msg_id)
{
    std::unique_lock<std::mutex> lock(_send_mutex);
    int que_size = _send_que.size();
    if (que_size > MAX_SENDQUE) {
        std::cout << "session: " << _session_id << " send que fulled, size is " << MAX_SENDQUE << std::endl;
        return;
    }
    _send_que.push(std::make_shared<SendMsgNode>(msg.c_str(), msg.length(), msg_id));
    if (que_size > 0) {
        return;
    }
    auto& msgnode = _send_que.front();
    boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
        std::bind(&CSession::HandleWrite, this, std::placeholders::_1));

}

void CSession::HandleWrite(boost::system::error_code ec) {
    try
    {
        auto self = shared_from_this();
        //如果没错误
        if (!ec) {
            std::lock_guard<std::mutex> lock(_send_mutex);
            //cout << "send data " << _send_que.front()->_data+HEAD_LENGTH << endl;
            _send_que.pop();
            if (!_send_que.empty()) {
                auto& msgnode = _send_que.front();
                boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
                    std::bind(&CSession::HandleWrite, this, std::placeholders::_1));
            }
        }
        else {
            std::cout << "handle write failed, error is " << ec.what() << std::endl;
            Close();
        }
    }
    catch (std::exception& e) {
        std::cout << "exception is" << e.what() << std::endl;
    }
}
LogicNode::LogicNode(std::shared_ptr<CSession> session, std::shared_ptr<RecvMsgNode> recvnode):_session(session),
_recv_node(recvnode)
{

}

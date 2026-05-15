#include "MessageNode.h"

RecvMsgNode::RecvMsgNode(short maxlen, short msgid):MessageNode(maxlen),_msg_id(msgid)
{
}

SendMsgNode::SendMsgNode(const char* msg, short maxlen, short msgid):
	MessageNode(maxlen+HEAD_TOTAL_LEN),_msg_id(msgid)
{
	//先将id转为网络字节序
	short msg_id_len = boost::asio::detail::socket_ops::host_to_network_short(msgid);
	//将数据id放入消息节点上
	memcpy(_data, &msg_id_len, HEAD_ID_LEN);
	short msg_data_len = boost::asio::detail::socket_ops::host_to_network_short(maxlen );
	memcpy(_data + HEAD_ID_LEN, &msg_data_len, HEAD_DATA_LEN);
	memcpy(_data + HEAD_ID_LEN+HEAD_DATA_LEN, msg, maxlen);

}

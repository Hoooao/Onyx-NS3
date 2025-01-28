#include "onyxProxyApp.h"

using namespace ns3;
using namespace Onyx;

NS_LOG_COMPONENT_DEFINE("OnyxProxyApp");
NS_OBJECT_ENSURE_REGISTERED(OnyxProxyApp);

void OnyxProxyApp::Setup(Ptr<Socket> recvSocket, const std::vector<Ptr<Socket>>& forwardSockets, uint32_t initReceiverId, uint32_t receiverNum, std::string name) {
    OnyxConfigs& configs = OnyxConfigs::GetInstance();
    _recvSocket = recvSocket;
    _forwardSockets = forwardSockets;
    _packetSize = configs.packetSize;
    _initReceiverId = initReceiverId;
    _receiverNum = receiverNum;
    _rr = configs.rr;
    _name = name;
    NS_LOG_INFO(name<<" initReceiverId: " << _initReceiverId << " receiverNum: " << _receiverNum << " forwardSockets size: " << _forwardSockets.size());
}

void OnyxProxyApp::ReceivePacket(Ptr<Socket> socket) {
    Address from;
    Ptr<Packet> packet = socket->RecvFrom(from);
    Address to;
    socket->GetPeerName(to);
    OnyxHeader header;
    packet->PeekHeader(header);
    NS_LOG_DEBUG_WITH_TIME("seq="<<header.GetSeq()<<" -> " << GetNodeNameFromIP(InetSocketAddress::ConvertFrom(to).GetIpv4())<< " from " << GetNodeNameFromIP(InetSocketAddress::ConvertFrom(from).GetIpv4()));

    for (size_t idx = _initReceiverId; idx < _initReceiverId + _receiverNum; ++idx) {
        size_t socketIdx = idx % _forwardSockets.size();
        Ptr<Packet> forwardPacket = packet->Copy();
        _forwardSockets[socketIdx]->Send(forwardPacket);
        Address to;
        _forwardSockets[socketIdx]->GetPeerName(to);
        NS_LOG_DEBUG_WITH_TIME("Forwarding packet to " << GetNodeNameFromIP(InetSocketAddress::ConvertFrom(to).GetIpv4()));
    }
    if (_rr) {
        _initReceiverId = (_initReceiverId + 1) % _forwardSockets.size();
    }
}
void OnyxProxyApp::StartApplication() {
    NS_LOG_INFO("Starting proxy application");
    _recvSocket->SetRecvCallback(MakeCallback(&OnyxProxyApp::ReceivePacket, this));
}

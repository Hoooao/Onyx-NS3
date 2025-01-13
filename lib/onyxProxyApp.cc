#include "onyxProxyApp.h"

using namespace ns3;
using namespace Onyx;

NS_LOG_COMPONENT_DEFINE("OnyxProxyApp");
NS_OBJECT_ENSURE_REGISTERED(OnyxProxyApp);

void OnyxProxyApp::Setup(Ptr<Socket> recvSocket, const std::vector<Ptr<Socket>>& forwardSockets, uint32_t packetSize, uint32_t initReceiverId, uint32_t receiverNum, bool rr) {
    _recvSocket = recvSocket;
    _forwardSockets = forwardSockets;
    _packetSize = packetSize;
    _initReceiverId = initReceiverId;
    _receiverNum = receiverNum;
    _rr = rr;
    NS_LOG_INFO("initReceiverId: " << _initReceiverId << " receiverNum: " << _receiverNum << " forwardSockets size: " << _forwardSockets.size());
}

void OnyxProxyApp::ReceivePacket(Ptr<Socket> socket) {
    Address from;
    Ptr<Packet> packet = socket->RecvFrom(from);
    NS_LOG_INFO_WITH_TIME("Packet received at proxy " << GetNode()->GetId() << " from " << GetNodeNameFromIP(InetSocketAddress::ConvertFrom(from).GetIpv4()));
    
    for (size_t idx = _initReceiverId; idx < _initReceiverId + _receiverNum; ++idx) {
        size_t socketIdx = idx % _forwardSockets.size();
        Ptr<Packet> forwardPacket = packet->Copy();
        _forwardSockets[socketIdx]->Send(forwardPacket);
        Address to;
        _forwardSockets[socketIdx]->GetPeerName(to);
        NS_LOG_INFO_WITH_TIME("Forwarding packet to " << GetNodeNameFromIP(InetSocketAddress::ConvertFrom(to).GetIpv4()));
    }
    if (_rr) {
        _initReceiverId = (_initReceiverId + 1) % _forwardSockets.size();
    }
}
void OnyxProxyApp::StartApplication() {
    NS_LOG_INFO("Starting proxy application");
    _recvSocket->SetRecvCallback(MakeCallback(&OnyxProxyApp::ReceivePacket, this));
}

#include "onyxProxyApp.h"

using namespace ns3;
using namespace Onyx;

NS_LOG_COMPONENT_DEFINE("OnyxProxyApp");
NS_OBJECT_ENSURE_REGISTERED(OnyxProxyApp);

void OnyxProxyApp::Setup(Ptr<Socket> recvSocket, const std::vector<Ptr<Socket>>& forwardSockets, uint32_t packetSize, uint32_t initReceiverId, uint32_t receiverNum) {
    NS_LOG_INFO("Setting up proxy application");
    NS_LOG_DEBUG("initReceiverId: " << _initReceiverId << " receiverNum: " << _receiverNum << " forwardSockets size: " << _forwardSockets.size());
    _recvSocket = recvSocket;
    _forwardSockets = forwardSockets;
    _packetSize = packetSize;
    _initReceiverId = initReceiverId;
    _receiverNum = receiverNum;
}

void OnyxProxyApp::ReceivePacket(Ptr<Socket> socket) {
    Address from;
    Ptr<Packet> packet = socket->RecvFrom(from);
    NS_LOG_DEBUG("Packet received at proxy " << GetNode()->GetId() << " from " << InetSocketAddress::ConvertFrom(from).GetIpv4());
    // TODO(Hao): hard coded targets, need to implement RR
    for (size_t idx = _initReceiverId; idx < _initReceiverId + _receiverNum; ++idx) {
        Ptr<Packet> forwardPacket = packet->Copy();
        _forwardSockets[idx]->Send(forwardPacket);
        NS_LOG_DEBUG("Forwarded packet to node " << idx);
    }
}
void OnyxProxyApp::StartApplication() {
    NS_LOG_INFO("Starting proxy application");
    _recvSocket->SetRecvCallback(MakeCallback(&OnyxProxyApp::ReceivePacket, this));
}

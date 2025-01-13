#include "onyxReceiverApp.h"

using namespace ns3;
using namespace Onyx;

NS_LOG_COMPONENT_DEFINE("OnyxReceiverApp");
NS_OBJECT_ENSURE_REGISTERED(OnyxReceiverApp);

void OnyxReceiverApp::Setup(std::vector<Ptr<Socket>> recvSockets, uint32_t packetSize) {
    _recvSockets = recvSockets;
    _packetSize = packetSize;
}

void OnyxReceiverApp::ReceivePacket(Ptr<Socket> socket) {
    Address from;
    Ptr<Packet> packet = socket->RecvFrom(from);
    NS_LOG_INFO_WITH_TIME("Packet received at receiver " << GetNode()->GetId() << " from " << GetNodeNameFromIP(InetSocketAddress::ConvertFrom(from).GetIpv4()));
}
void OnyxReceiverApp::StartApplication() {
    NS_LOG_INFO("Starting receiver application");
    for (auto& socket: _recvSockets) {
        socket->SetRecvCallback(MakeCallback(&OnyxReceiverApp::ReceivePacket, this));
    }
}

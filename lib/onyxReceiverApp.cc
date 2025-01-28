#include "onyxReceiverApp.h"

using namespace ns3;
using namespace Onyx;

NS_LOG_COMPONENT_DEFINE("OnyxReceiverApp");
NS_OBJECT_ENSURE_REGISTERED(OnyxReceiverApp);

void OnyxReceiverApp::Setup(Ptr<Socket> recvSocket, std::string name) {
    OnyxConfigs& configs = OnyxConfigs::GetInstance();
    _recvSocket = recvSocket;
    _packetSize = configs.packetSize;
    _name = name;
}

void OnyxReceiverApp::ReceivePacket(Ptr<Socket> socket) {
    Address from;
    Ptr<Packet> packet = socket->RecvFrom(from);
    OnyxHeader header;
    packet->PeekHeader(header);
    std::string name = GetNodeAllNamesFromIP(GetIPFromNodeName(Names::FindName(socket->GetNode())));    
    NS_LOG_INFO_WITH_TIME("seq="<<header.GetSeq()<<"->" << name << " <~ " << GetNodeAllNamesFromIP(InetSocketAddress::ConvertFrom(from).GetIpv4()));
}
void OnyxReceiverApp::StartApplication() {
    NS_LOG_INFO("Starting receiver application");
    _recvSocket->SetRecvCallback(MakeCallback(&OnyxReceiverApp::ReceivePacket, this));
    
}

#include "onyxClientApp.h"

using namespace Onyx;

NS_LOG_COMPONENT_DEFINE("OnyxClientApp");
NS_OBJECT_ENSURE_REGISTERED(OnyxClientApp);

void OnyxClientApp::Setup(const std::vector<Ptr<Socket>>& proxySockets, const OnyxLatencyGenerator& latGenerator){
    OnyxConfigs& configs = OnyxConfigs::GetInstance();
    _proxySockets = proxySockets;
    _packetSize = configs.packetSize;
    _frequency = configs.clientSendFreq;
    _latGenerator = latGenerator;
    NS_LOG_INFO("Client setup with packet size " << _packetSize << " and frequency " << _frequency);
}

void OnyxClientApp::StartApplication() {
    NS_LOG_INFO("Starting client application");
    _running = true;
    _sent = 0;
    SendMessage();
}

void OnyxClientApp::StopApplication() {
    _running = false;
    if (_event.IsRunning()) {
        Simulator::Cancel(_event);
    }
}

void OnyxClientApp::SendMessage() {
    NS_LOG_INFO_WITH_TIME("Client broadcasting seq=" << _sent);
    _latGenerator.GenerateSpike();
    for (auto& socket: _proxySockets) {
        Ptr<Packet> packet = Create<Packet>(_packetSize);
        OnyxHeader header(_sent);
        packet->AddHeader(header);
        socket->Send(packet);
        Address from;
        socket->GetSockName(from);
        Address to;
        socket->GetPeerName(to);
        NS_LOG_DEBUG_WITH_TIME("Client sent packet to " << GetNodeNameFromIP(InetSocketAddress::ConvertFrom(to).GetIpv4()));
    }
    _sent++;
    _event = Simulator::Schedule(Seconds(1.0 / _frequency), &OnyxClientApp::SendMessage, this);
    
    Simulator::Schedule(MilliSeconds(50), &OnyxLatencyGenerator::GenerateSmallLatency, this->_latGenerator);
}
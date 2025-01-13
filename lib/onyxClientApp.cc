#include "onyxClientApp.h"

using namespace Onyx;

NS_LOG_COMPONENT_DEFINE("OnyxClientApp");
NS_OBJECT_ENSURE_REGISTERED(OnyxClientApp);

void OnyxClientApp::Setup(const std::vector<Ptr<Socket>>& proxySockets, uint32_t packetSize, uint32_t packetCount, double frequency){
    _proxySockets = proxySockets;
    _packetSize = packetSize;
    _packetCount = packetCount;
    _frequency = frequency;
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
    if (_sent < _packetCount) {
        NS_LOG_INFO_WITH_TIME("Client broadcasting packet " << _sent);
        for (auto& socket: _proxySockets) {
            Ptr<Packet> packet = Create<Packet>(_packetSize);
            socket->Send(packet);
            Address from;
            socket->GetSockName(from);
            Address to;
            socket->GetPeerName(to);
            NS_LOG_INFO_WITH_TIME("Client sent packet of size " << _packetSize << " to " << GetNodeNameFromIP(InetSocketAddress::ConvertFrom(to).GetIpv4()));

        }
        _sent++;
        _event = Simulator::Schedule(Seconds(1.0 / 1), &OnyxClientApp::SendMessage, this);
    }
}
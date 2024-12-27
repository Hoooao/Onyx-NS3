#ifndef ONYX_RECEIVER_APP_H
#define ONYX_RECEIVER_APP_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

namespace Onyx{
    class OnyxReceiverApp : public Application {
    public:
        OnyxReceiverApp() : _recvSockets(0), _packetSize(1024) {}
        static TypeId GetTypeId(){
            static TypeId tid = TypeId("OnyxReceiverApp")
                                .SetParent<Application>()
                                .SetGroupName("Onyx")
                                .AddConstructor<OnyxReceiverApp>();
            return tid;
        }
        void Setup(std::vector<Ptr<Socket>> recvSockets, uint32_t packetSize);
        virtual void StartApplication();

        virtual void StopApplication() {
            if (!_recvSockets.empty()) {
                for (auto& socket: _recvSockets) {
                    socket->Close();
                }   
            }
        }
    private:
        std::vector<Ptr<Socket>> _recvSockets;
        uint32_t _packetSize;

        void ReceivePacket(Ptr<Socket> socket);


    };
}

#endif
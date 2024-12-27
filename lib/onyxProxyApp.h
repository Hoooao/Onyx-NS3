#ifndef ONYX_PROXY_APP_H
#define ONYX_PROXY_APP_H
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;


namespace Onyx{
    class OnyxProxyApp : public Application {
    public:
        OnyxProxyApp() : _recvSocket(nullptr), _packetSize(1024), _initReceiverId(0), _receiverNum(0) {}
        static TypeId GetTypeId(){
            static TypeId tid = TypeId("OnyxProxyApp")
                                .SetParent<Application>()
                                .SetGroupName("Onyx")
                                .AddConstructor<OnyxProxyApp>();
            return tid;
        }
        void Setup(Ptr<Socket> recvSocket, const std::vector<Ptr<Socket>>& forwardSockets, uint32_t packetSize, uint32_t initReceiverId, uint32_t receiverNum);
        virtual void StartApplication();

        virtual void StopApplication() {
            if (_recvSocket) {
                _recvSocket->Close();
            }
        }
    private:
        Ptr<Socket> _recvSocket;
        std::vector<Ptr<Socket>> _forwardSockets;
        uint32_t _packetSize;
        uint32_t _initReceiverId;
        uint32_t _receiverNum;

        void ReceivePacket(Ptr<Socket> socket);


    };
}

#endif
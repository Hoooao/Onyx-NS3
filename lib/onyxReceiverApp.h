#ifndef ONYX_RECEIVER_APP_H
#define ONYX_RECEIVER_APP_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

#include "common.h"

using namespace ns3;

namespace Onyx{
    class OnyxReceiverApp : public Application {
    public:
        OnyxReceiverApp() :_packetSize(1024) {}
        static TypeId GetTypeId(){
            static TypeId tid = TypeId("OnyxReceiverApp")
                                .SetParent<Application>()
                                .SetGroupName("Onyx")
                                .AddAttribute("Name", "The name of the application", 
                                        StringValue("OnyxReceiverApp"),
                                        MakeStringAccessor(&OnyxReceiverApp::_name),
                                        MakeStringChecker())
                                .AddConstructor<OnyxReceiverApp>();
            return tid;
        }
        void Setup(Ptr<Socket> recvSocket, std::string name);
        virtual void StartApplication();

        virtual void StopApplication() {
            _recvSocket->Close();
        }
    private:
        Ptr<Socket> _recvSocket;
        uint32_t _packetSize;
        std::string _name;

        void ReceivePacket(Ptr<Socket> socket);


    };
}

#endif
// add a if define to avoid multiple include
#ifndef ONYX_CLIENT_APP_H
#define ONYX_CLIENT_APP_H
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

#include "common.h"

using namespace ns3;
namespace Onyx{
    class OnyxClientApp : public Application {
    public:
        OnyxClientApp(){}
        static TypeId GetTypeId(){
            static TypeId tid = TypeId("OnyxClientApp")
                                    .SetParent<Application>()
                                    .SetGroupName("Onyx")
                                    .AddConstructor<OnyxClientApp>();
            return tid;
        }
        void Setup(const std::vector<Ptr<Socket>>& proxySockets, uint32_t packetSize, uint32_t packetCount, double frequency);
        virtual void StartApplication() override;
        virtual void StopApplication() override;

    private:
        std::vector<Ptr<Socket>> _proxySockets;
        std::vector<Address> _receivers; // List of receiver addresses
        EventId _event;
        bool _running;
        uint32_t _packetSize;
        uint32_t _packetCount;
        Time _interval;
        uint32_t _sent;
        double _frequency; // Frequency in Hz

        void SendMessage();

    };
}
#endif
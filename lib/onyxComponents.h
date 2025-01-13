#ifndef ONYX_COMPONENTS_H
#define ONYX_COMPONENTS_H
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

#include "onyxClientApp.h"
#include "onyxProxyApp.h"
#include "onyxReceiverApp.h"

#include "common.h"

#include <vector>
#include <iostream>

using namespace ns3;
namespace Onyx {

    // For now it is just a wrapper for NodeContainer
    // Hao: do we need inter proxy conn? 
    class OnyxTreeLayer {
        public:
            OnyxTreeLayer(uint32_t nNodes);
            ~OnyxTreeLayer() = default;
        private:
            uint32_t _nNodes;
            NodeContainer _nodes;
    };

    class OnyxTree{
        public:
            OnyxTree(uint32_t nProxy, uint32_t nReceiver, std::string dataRate, uint64_t delay_ms, bool rr);
            ~OnyxTree() = default;
            // TODO(Hao): make it more general using polymorphism (to add more layers), not needed now tho..
            void SetupTopology();
            void StartApplications();
        private:
            uint32_t _nProxy;
            uint32_t _nReceiver;
            std::string _dataRate;
            uint64_t _delay_ms;
            bool _rr;

            std::vector<NodeContainer> _nodesLayers;

            Ptr<OnyxClientApp> _clientApp;
            std::vector<Ptr<OnyxProxyApp>> _proxyApps;
            std::vector<Ptr<OnyxReceiverApp>> _receiverApps;
    };

    std::vector<Ptr<Socket>> CreateSocketsFromNodeToIPs(Ptr<Node> node, const std::vector<Ipv4InterfaceContainer>& receivers, uint32_t port);

}


#endif
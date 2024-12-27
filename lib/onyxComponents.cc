#include "onyxComponents.h"
#define CLI_PROXY_PORT 5000
#define PROXY_RECVR_PORT 6000
// adjust this later
#define NUM_OF_RECEIVERS_PER_PROXY 3

NS_LOG_COMPONENT_DEFINE("OnyxTree");
namespace Onyx {

    OnyxTreeLayer::OnyxTreeLayer(uint32_t nNodes) : _nNodes(nNodes) {
        std::cout << "Creating a layer with " << nNodes << " nodes" << std::endl;
        _nodes.Create(nNodes);
    }

    OnyxTree::OnyxTree(uint32_t nProxy, 
        uint32_t nReceiver, 
        std::string dataRate = "100Mbps",
        uint64_t delay_ms = 1000): _nodesLayers(3), _nProxy(nProxy), _nReceiver(nReceiver), _dataRate(dataRate), _delay_ms(delay_ms) {
        NS_LOG_INFO("Creating OnyxTree with " << nProxy << " proxies and " << nReceiver << " receivers");
        // TODO: make NUM_OF_RECEIVERS_PER_PROXY a parameter
        if(_nReceiver != _nProxy * NUM_OF_RECEIVERS_PER_PROXY){
            NS_LOG_ERROR("Number of receivers should be multiple of " <<  NUM_OF_RECEIVERS_PER_PROXY << " of proxies");
            exit(1);
        }
        // root node
        _nodesLayers[0] = NodeContainer(1);
        // proxy nodes
        _nodesLayers[1] = NodeContainer(_nProxy);
        // receiver nodes
        _nodesLayers[2] = NodeContainer(_nReceiver);
    }

    void OnyxTree::SetupTopology() {
        PointToPointHelper p2p;
        p2p.SetDeviceAttribute("DataRate", StringValue(_dataRate));
        p2p.SetChannelAttribute("Delay", TimeValue(MilliSeconds(_delay_ms)));
        
        std::vector<NetDeviceContainer> cli2ProxyDevices;
        std::vector<std::vector<NetDeviceContainer>> proxy2RecvrDevices(_nProxy);
        // Install the P2P links
        auto cliNode = _nodesLayers[0].Get(0);
        for (uint32_t i = 0; i < _nProxy; i++) {
            cli2ProxyDevices.emplace_back(p2p.Install(cliNode, _nodesLayers[1].Get(i)));
        }
        for( uint32_t i = 0; i < _nProxy; i++) {
            for (uint32_t j = 0; j < _nReceiver; j++) {
                proxy2RecvrDevices[i].emplace_back(p2p.Install(_nodesLayers[1].Get(i), _nodesLayers[2].Get(j)));
            }
        }
 
        
        // Install the internet stack on all nodes
        InternetStackHelper stack;
        stack.Install(_nodesLayers[0]);
        stack.Install(_nodesLayers[1]);
        stack.Install(_nodesLayers[2]);

        std::vector<Ipv4InterfaceContainer> cli2ProxyIFs;
        std::vector<std::vector<Ipv4InterfaceContainer>> proxies2RecvrIFs;

        // Assign IP addresses
        Ipv4AddressHelper address;
        address.SetBase("10.1.1.0", "255.255.255.0");
        for (uint32_t i = 0; i < _nProxy; i++) 
            cli2ProxyIFs.emplace_back(address.Assign(cli2ProxyDevices[i]));
        address.SetBase("10.1.2.0", "255.255.255.0");
        for (uint32_t i = 0; i < _nProxy; i++) {
            std::vector<Ipv4InterfaceContainer> proxy2RecvrIFs;
            for (uint32_t j = 0; j < _nReceiver; j++) 
                proxy2RecvrIFs.emplace_back(address.Assign(proxy2RecvrDevices[i][j]));
            proxies2RecvrIFs.emplace_back(proxy2RecvrIFs);
        }
        for (auto& ifc: cli2ProxyIFs) {
            std::cout << "Client to proxy interface: " << ifc.GetAddress(0) << " -> " << ifc.GetAddress(1) << std::endl;
        }
        for (auto& ifc: proxies2RecvrIFs) {
            for (auto& ifc2: ifc) {
                std::cout << "Proxy to receiver interface: " << ifc2.GetAddress(0) << " -> " << ifc2.GetAddress(1) << std::endl;
            }
        }
        // Setup receivers
        std::vector<Ptr<OnyxReceiverApp>> receiverApps;
        std::vector<std::vector<Ptr<Socket>>> proxyId2RcvrSockets(_nProxy);
        for (uint32_t i = 0; i < _nReceiver; i++) {
            std::vector<Ptr<Socket>> recvSockets;
            for(uint32_t j = 0; j < _nProxy; j++) {
                Ptr<Socket> socket = Socket::CreateSocket(_nodesLayers[2].Get(i), TypeId::LookupByName("ns3::UdpSocketFactory"));
                socket->Bind(InetSocketAddress(proxies2RecvrIFs[j][i].GetAddress(1), PROXY_RECVR_PORT + j*10));
                recvSockets.emplace_back(socket);

                NS_LOG_DEBUG("Receiver " << i << " bound to receiving socket at " << proxies2RecvrIFs[j][i].GetAddress(1));

                Ptr<Socket> sendSocket = Socket::CreateSocket(_nodesLayers[1].Get(j), TypeId::LookupByName("ns3::UdpSocketFactory"));
                sendSocket->Connect(InetSocketAddress(proxies2RecvrIFs[j][i].GetAddress(1), PROXY_RECVR_PORT + j*10));
                proxyId2RcvrSockets[j].emplace_back(sendSocket);
            }
            Ptr<OnyxReceiverApp> receiverApp = CreateObject<OnyxReceiverApp>();
            receiverApp->Setup(recvSockets, 1024);
            _nodesLayers[2].Get(i)->AddApplication(receiverApp);  
            receiverApps.emplace_back(receiverApp);
        }


        // Setup proxies
        std::vector<Ptr<OnyxProxyApp>> proxyApps;
        std::vector<Ptr<Socket>> proxySockets(_nProxy);
        for (uint32_t i = 0; i < _nProxy; i++) {
            Ptr<Socket> recvSocket = Socket::CreateSocket(_nodesLayers[1].Get(i), TypeId::LookupByName("ns3::UdpSocketFactory"));
            recvSocket->Bind(InetSocketAddress(cli2ProxyIFs[i].GetAddress(1), CLI_PROXY_PORT));
            
            NS_LOG_DEBUG("Proxy " << i << " bound to receiving socket at " << cli2ProxyIFs[i].GetAddress(1));
            
            Ptr<OnyxProxyApp> proxyApp = CreateObject<OnyxProxyApp>();
            proxyApp->Setup(recvSocket, proxyId2RcvrSockets[i], 1024, i * NUM_OF_RECEIVERS_PER_PROXY, NUM_OF_RECEIVERS_PER_PROXY);
            _nodesLayers[1].Get(i)->AddApplication(proxyApp);
            proxyApps.emplace_back(proxyApp);

            // Create the counterpart sockets for the client to connect from
            proxySockets[i] = Socket::CreateSocket(cliNode, TypeId::LookupByName("ns3::UdpSocketFactory"));
            proxySockets[i]->Connect(InetSocketAddress(cli2ProxyIFs[i].GetAddress(1), CLI_PROXY_PORT ));
        }

        // Setup clients
        Ptr<OnyxClientApp> clientApp = CreateObject<OnyxClientApp>();
        clientApp->Setup(proxySockets, 1024, 100, 1.0);
        cliNode->AddApplication(clientApp);

        _clientApp = std::move(clientApp);
        _proxyApps = std::move(proxyApps);
        _receiverApps = std::move(receiverApps);

        // Add exact match routes
        Ipv4StaticRoutingHelper routingHelper;
        Ptr<Ipv4StaticRouting> routing = routingHelper.GetStaticRouting(cliNode->GetObject<Ipv4>());
        for(uint32_t i = 0; i < _nProxy; i++) {
            // cli -> proxy
            routing->AddHostRouteTo(cli2ProxyIFs[i].GetAddress(1), Ipv4Address::GetAny(), i+1);
            Ptr<Ipv4StaticRouting> proxyRouting = routingHelper.GetStaticRouting(_nodesLayers[1].Get(i)->GetObject<Ipv4>());
            for(uint32_t j = 0; j < _nReceiver; j++) {
                // proxy -> rcvr
                proxyRouting = routingHelper.GetStaticRouting(_nodesLayers[1].Get(i)->GetObject<Ipv4>());
                // j + 2 for interface as 1 is the client
                proxyRouting->AddHostRouteTo(proxies2RecvrIFs[i][j].GetAddress(1), Ipv4Address::GetAny(), j + 2);
            }
        }

        //p2p.EnablePcapAll("onyx_pcap/onyx");
        for(uint32_t i = 0; i < _nProxy; i++) {
            p2p.EnablePcap("onyx_pcap/onyx_cli", cli2ProxyDevices[i].Get(0), true);
            p2p.EnablePcap("onyx_pcap/onyx_proxy", cli2ProxyDevices[i].Get(1), true);
        }
        for(uint32_t i = 0; i < _nProxy; i++) {
            for(uint32_t j = 0; j < _nReceiver; j++) {
                p2p.EnablePcap("onyx_pcap/onyx_proxy_to_rcvr", proxy2RecvrDevices[i][j].Get(0), true);
                p2p.EnablePcap("onyx_pcap/onyx_recvr", proxy2RecvrDevices[i][j].Get(1), true);
            }
        }   
    }

    // TODO: make times configurable with cmd args
    void OnyxTree::StartApplications() {
        NS_LOG_INFO("Starting applications");
        _clientApp->SetStartTime(Seconds(2.0));
        _clientApp->SetStopTime(Seconds(10.0));
        for (auto& proxyApp: _proxyApps) {
            proxyApp->SetStartTime(Seconds(0));
            proxyApp->SetStopTime(Seconds(10.5));
        }
        for (auto& receiverApp: _receiverApps) {
            receiverApp->SetStartTime(Seconds(0));
            receiverApp->SetStopTime(Seconds(10.5));
        }

        //Ipv4GlobalRoutingHelper::PopulateRoutingTables();
        Ptr<ns3::OutputStreamWrapper> stream = Create<OutputStreamWrapper>("onyx_pcap/routing-table-cli", std::ios::out);
        _clientApp->GetNode()->GetObject<Ipv4>()->GetRoutingProtocol()->PrintRoutingTable(stream);

        Ptr<ns3::OutputStreamWrapper> stream1 = Create<OutputStreamWrapper>("onyx_pcap/routing-table-proxy0", std::ios::out);
        _proxyApps[0]->GetNode()->GetObject<Ipv4>()->GetRoutingProtocol()->PrintRoutingTable(stream1);
        Simulator::Run();
        Simulator::Destroy();
    }


} // namespace Onyx
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

    OnyxTree::OnyxTree(){
        OnyxConfigs& configs = OnyxConfigs::GetInstance();
        _nProxy = configs.nProxy;
        _nReceiver = configs.nReceiver;
        _dataRate = std::to_string(configs.dataRate) + "Mbps";
        _delay_us = configs.delay;
        _rr = configs.rr;
        _latencySpikePossibility = configs.latencySpikePossibility;
        _nodesLayers = std::vector<NodeContainer>(3);
        if(_nReceiver != _nProxy * configs.numOfReceiversPerProxy){
            NS_LOG_ERROR("Number of receivers should be multiple of " <<  NUM_OF_RECEIVERS_PER_PROXY << " of proxies");
            exit(1);
        }
    }

    OnyxTree::OnyxTree(uint32_t nProxy, 
        uint32_t nReceiver, 
        std::string dataRate = "100Mbps",
        uint64_t delay_ms = 1000,
        bool rr = false,
        float latencySpikePossibility = 0.0): _nodesLayers(3), 
        _nProxy(nProxy), _nReceiver(nReceiver), _dataRate(dataRate), 
        _delay_us(delay_ms), _rr(rr),_latencySpikePossibility(latencySpikePossibility) {

        NS_LOG_INFO("Creating OnyxTree with " << nProxy << " proxies and " << nReceiver << " receivers");
        if(_nReceiver != _nProxy * NUM_OF_RECEIVERS_PER_PROXY){
            NS_LOG_ERROR("Number of receivers should be multiple of " <<  NUM_OF_RECEIVERS_PER_PROXY << " of proxies");
            exit(1);
        }
    }

    void OnyxTree::SetupTopology() {
        PointToPointHelper NodeToSW;
        PointToPointHelper SWToSW;

        NodeToSW.SetDeviceAttribute ("DataRate", 	StringValue (_dataRate));
        NodeToSW.SetChannelAttribute("Delay", 		TimeValue(MicroSeconds(_delay_us)));
        SWToSW.SetDeviceAttribute 	("DataRate", 	StringValue (_dataRate));
        SWToSW.SetChannelAttribute 	("Delay", 		TimeValue(MicroSeconds(_delay_us)));


        // TrafficControlHelper tchNodes;		// node to swtiche!   --- Not needed rn

        NodeContainer spins, leafs, nodes[LEAF_CNT];
        { // filling the variables!
            spins.Create (SPIN_CNT);
            leafs.Create(LEAF_CNT);
            for(uint8_t i = 0; i < LEAF_CNT; i++)
                nodes[i].Create(NODE_IN_RACK_CNT);
        }

        InternetStackHelper stack;
        {
            stack.Install(spins);
            stack.Install(leafs);
            for(uint8_t i = 0; i < LEAF_CNT; i++)
                stack.Install(nodes[i]);
        }

	//=========================== Creating Topology ==============================//
        NetDeviceContainer node2sw [LEAF_CNT][NODE_IN_RACK_CNT]; 		// [i][j]: rack[i], node[j]
	    NetDeviceContainer sw2sw [SPIN_CNT][LEAF_CNT];					// [i][j]: spin[i], rack[j]

        // Establish p2p connections
        {
            // Point2Point between nodes and leaf switches(rack)
            for(uint8_t i = 0; i < LEAF_CNT; i++)
                for(uint8_t j = 0; j < NODE_IN_RACK_CNT; j++){
                    node2sw[i][j] = NodeToSW.Install(nodes[i].Get(j), leafs.Get(i));
                }
            // Point2Point between leaf switches(rack) and spine switches
            for(uint8_t i = 0; i < SPIN_CNT; i++)
                for(uint8_t j = 0; j < LEAF_CNT; j++) {
                    sw2sw[i][j] = SWToSW.Install(spins.Get(i), leafs.Get(j));
                }
	    }

 	//=========================== Assigning the IP address ==============================//
        Ipv4AddressHelper address;
        {
            // node <-> switch(rack) interfaces
            for(uint8_t i = 0; i < LEAF_CNT; i++)
            {
                Names::Add("rack_switch" + std::to_string(i), leafs.Get(i));
                for(uint8_t j = 0; j < NODE_IN_RACK_CNT; j++)
                {
                    std::string ipBaseStr = "10." + std::to_string(i) + "." + std::to_string(j) + ".0";
                    Ipv4Address ipBase = Ipv4Address(ipBaseStr.c_str());
                    address.SetBase(ipBase ,"255.255.255.0");
                    auto interface = address.Assign (node2sw[i][j]);
                    Names::Add("node" + std::to_string(i) + "_" + std::to_string(j), nodes[i].Get(j));
                    SetNodeNameForIP(interface.GetAddress(0), "node" + std::to_string(i) + "_" + std::to_string(j));
                    SetNodeNameForIP(interface.GetAddress(1), "rack_switch" + std::to_string(i));
                }}

            // switch(rack) <-> switch interfaces
            for(uint8_t i = 0; i < SPIN_CNT; i++)
                {
                Names::Add("spine_switch" + std::to_string(i), spins.Get(i));
                for(uint8_t j = 0; j < LEAF_CNT; j++)
                {
                    std::string ipBaseStr = "11." + std::to_string(i) + "." + std::to_string(j) + ".0";
                    Ipv4Address ipBase = Ipv4Address(ipBaseStr.c_str());
                    address.SetBase(ipBase ,"255.255.255.0");
                    auto interface = address.Assign (sw2sw[i][j]);
                    SetNodeNameForIP(interface.GetAddress(0), "spine_switch" + std::to_string(i));
                    SetNodeNameForIP(interface.GetAddress(1), "rack_switch" + std::to_string(j));
                }}
        }
    
    // Select nodes into an OnyxTree
    // FOR TESTING: we assume all proxy, node, and cli are in the same rack for now.
    // In rack 0, we have 1 client, 3 proxies, and 9 receivers
    // Wrap this into a function later, that reads some spec
        _nodesLayers[0] = NodeContainer(nodes[0].Get(0));
        for(uint32_t i = 1; i <= _nProxy; i++) {
            _nodesLayers[1].Add(nodes[0].Get(i));
            SetNodeNameForIP(GetIPFromNodeName(Names::FindName(nodes[0].Get(i))), "proxy" + std::to_string(i));
        }
        for(uint32_t i = _nProxy + 1; i <= _nProxy + _nReceiver; i++) {
            _nodesLayers[2].Add(nodes[0].Get(i));
            SetNodeNameForIP(GetIPFromNodeName(Names::FindName(nodes[0].Get(i))), "receiver" + std::to_string(i - _nProxy - 1));
        }
        


        // Setup receivers
        std::vector<Ptr<OnyxReceiverApp>> receiverApps;
        std::vector<std::vector<Ptr<Socket>>> proxyId2RcvrSockets(_nProxy);
        for (uint32_t i = 0; i < _nReceiver; i++) {
            Ptr<Socket> socket = Socket::CreateSocket(_nodesLayers[2].Get(i), TypeId::LookupByName("ns3::UdpSocketFactory"));
            socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), PROXY_RECVR_PORT));
            for(uint32_t j = 0; j < _nProxy; j++) {
                Ptr<Socket> sendSocket = Socket::CreateSocket(_nodesLayers[1].Get(j), TypeId::LookupByName("ns3::UdpSocketFactory"));
                sendSocket->Connect(InetSocketAddress(GetIPFromNodeName(Names::FindName(_nodesLayers[2].Get(i))), PROXY_RECVR_PORT));
                proxyId2RcvrSockets[j].emplace_back(sendSocket);
            }
            Ptr<OnyxReceiverApp> receiverApp = CreateObject<OnyxReceiverApp>();
            receiverApp->Setup(socket, "receiver" + std::to_string(i));
            _nodesLayers[2].Get(i)->AddApplication(receiverApp);  
            receiverApps.emplace_back(receiverApp);
        }


        // Setup proxies
        std::vector<Ptr<OnyxProxyApp>> proxyApps;
        std::vector<Ptr<Socket>> proxySockets(_nProxy);
        OnyxLatencyGenerator latGenerator;
        auto cliNode = _nodesLayers[0].Get(0);
        for (uint32_t i = 0; i < _nProxy; i++) {
            Ptr<Socket> recvSocket = Socket::CreateSocket(_nodesLayers[1].Get(i), TypeId::LookupByName("ns3::UdpSocketFactory"));
            recvSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), CLI_PROXY_PORT));
            
            Ptr<OnyxProxyApp> proxyApp = CreateObject<OnyxProxyApp>();
            proxyApp->Setup(recvSocket, proxyId2RcvrSockets[i], i * NUM_OF_RECEIVERS_PER_PROXY, NUM_OF_RECEIVERS_PER_PROXY, "proxy" + std::to_string(i));
            _nodesLayers[1].Get(i)->AddApplication(proxyApp);
            proxyApps.emplace_back(proxyApp);

            // Create the counterpart sockets for the client to connect from
            proxySockets[i] = Socket::CreateSocket(cliNode, TypeId::LookupByName("ns3::UdpSocketFactory"));
            proxySockets[i]->Connect(InetSocketAddress(GetIPFromNodeName(Names::FindName(_nodesLayers[1].Get(i))), CLI_PROXY_PORT ));
            // Since node only connects to switch, so the dstNum will always be 1.
            // If later we want to adopt multi-spne, this can be change for switch(rack) to switch(spine)
            latGenerator.AddNode(_nodesLayers[1].Get(i), 1);
        }

        // Setup clients
        Ptr<OnyxClientApp> clientApp = CreateObject<OnyxClientApp>();
        clientApp->Setup(proxySockets, latGenerator);
        cliNode->AddApplication(clientApp);

        _clientApp = std::move(clientApp);
        _proxyApps = std::move(proxyApps);
        _receiverApps = std::move(receiverApps);

        // Add exact match routes
        // Ipv4StaticRoutingHelper routingHelper;
        // Ptr<Ipv4StaticRouting> routing = routingHelper.GetStaticRouting(cliNode->GetObject<Ipv4>());
        // for(uint32_t i = 0; i < _nProxy; i++) {
        //     // cli -> proxy
        //     routing->AddHostRouteTo(cli2ProxyIFs[i].GetAddress(1), Ipv4Address::GetAny(), i+1);
        //     Ptr<Ipv4StaticRouting> proxyRouting = routingHelper.GetStaticRouting(_nodesLayers[1].Get(i)->GetObject<Ipv4>());
        //     for(uint32_t j = 0; j < _nReceiver; j++) {
        //         // proxy -> rcvr
        //         proxyRouting = routingHelper.GetStaticRouting(_nodesLayers[1].Get(i)->GetObject<Ipv4>());
        //         // j + 2 for interface as 1 is the client
        //         proxyRouting->AddHostRouteTo(proxies2RecvrIFs[i][j].GetAddress(1), Ipv4Address::GetAny(), j + 2);
        //     }
        // }
        Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

        NodeToSW.EnablePcapAll("onyx_pcap/onyx");
        SWToSW.EnablePcapAll("onyx_pcap/onyx");
        // for(uint32_t i = 0; i < _nProxy; i++) {
        //     p2p.EnablePcap("onyx_pcap/onyx_cli", cli2ProxyDevices[i].Get(0), true);
        //     p2p.EnablePcap("onyx_pcap/onyx_proxy", cli2ProxyDevices[i].Get(1), true);
        // }
        // for(uint32_t i = 0; i < _nProxy; i++) {
        //     for(uint32_t j = 0; j < _nReceiver; j++) {
        //         p2p.EnablePcap("onyx_pcap/onyx_proxy_to_rcvr", proxy2RecvrDevices[i][j].Get(0), true);
        //         p2p.EnablePcap("onyx_pcap/onyx_recvr", proxy2RecvrDevices[i][j].Get(1), true);
        //     }
        // }   
    }

    // TODO: make times configurable with cmd args
    void OnyxTree::StartApplications() {
        NS_LOG_INFO("Starting applications");
        _clientApp->SetStartTime(Seconds(0.1));
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
        NS_LOG_INFO(" --------SIMULATION STARTED--------");

        Simulator::Run();
        Simulator::Destroy();
    }


} // namespace Onyx
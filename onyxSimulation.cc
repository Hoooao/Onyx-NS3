#include "lib/onyxComponents.h"


#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("OnyxTopology");

int main(int argc, char* argv[]){
    bool verbose = true;
    uint32_t nProxy = 3;
    uint32_t nReceiver = 9;
    uint32_t dataRate = 20;
    uint32_t delay = 1000;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nProxy", "Number of intermediate proxies", nProxy);
    cmd.AddValue("nReceiver", "Number of intermediate proxies", nReceiver);
    cmd.AddValue("dataRate", "Data rate of the links", dataRate);
    cmd.AddValue("delay", "Delay of the links in ms", delay);
    cmd.AddValue("verbose", "Tell applications to log if true", verbose);
    cmd.Parse(argc, argv);

    if (verbose)
    {
        LogComponentEnable("OnyxClientApp", LOG_LEVEL_INFO);
        LogComponentEnable("OnyxProxyApp", LOG_LEVEL_INFO);
        LogComponentEnable("OnyxReceiverApp", LOG_LEVEL_INFO);
        LogComponentEnable("OnyxTree", LOG_LEVEL_INFO);
        
        //LogComponentEnable("UdpSocket", LOG_LEVEL_ALL);
        //LogComponentEnable("Application", LOG_LEVEL_ALL);
    }

    Onyx::OnyxTree tree(nProxy, nReceiver, std::to_string(dataRate) + "Mbps", delay);

    tree.SetupTopology();
    tree.StartApplications();
}
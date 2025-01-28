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
    bool verbose = false;
    uint32_t nProxy = 3;
    uint32_t nReceiver = 9;
    uint32_t dataRate = BANDWIDTH_MBPS;
    uint32_t delay = LINK_BASE_LATENCY_US;
    uint32_t duration = 10;
    uint32_t clientSendFreq = 1;
    uint32_t numOfReceiversPerProxy = 3;
    uint32_t packetSize = PAYLOAD_SIZE_B;
    float latencySpikePossibility = 10;
    bool rr = false;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nProxy", "Number of intermediate proxies", nProxy);
    cmd.AddValue("nReceiver", "Number of intermediate proxies", nReceiver);
    cmd.AddValue("dataRate", "Data rate of the links/banwidth", dataRate);
    cmd.AddValue("delay", "Delay of the links in us", delay);
    cmd.AddValue("verbose", "Tell applications to log if true", verbose);
    cmd.AddValue("rr", "Round robin forwarding", rr);
    cmd.AddValue("packetSize", "Packet Size", packetSize);
    cmd.AddValue("duration", "Run the simulation for <duration> seconds", duration);
    cmd.AddValue("clientSendFreq", "Client's sending num/s", clientSendFreq);
    cmd.AddValue("numOfReceiversPerProxy", "Number of receivers per proxy", numOfReceiversPerProxy);
    cmd.AddValue("latencySpikePossibility", "Possibility of randomized latency spike in 10000", latencySpikePossibility);
    cmd.Parse(argc, argv);

    Onyx::OnyxConfigs& configs = Onyx::OnyxConfigs::GetInstance();
    configs.nProxy = nProxy;
    configs.nReceiver = nReceiver;
    configs.dataRate = dataRate;
    configs.delay = delay;
    configs.duration = duration;
    configs.clientSendFreq = clientSendFreq;
    configs.latencySpikePossibility = latencySpikePossibility;
    configs.numOfReceiversPerProxy = numOfReceiversPerProxy;
    configs.packetSize = packetSize;
    configs.rr = rr;

    std::ofstream logFile("./onyx_pcap/simulation.log");
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file!" << std::endl;
        return 1;
    }

    LogComponentEnable("OnyxClientApp", LOG_LEVEL_INFO);
    LogComponentEnable("OnyxProxyApp", LOG_LEVEL_INFO);
    LogComponentEnable("OnyxReceiverApp", LOG_LEVEL_INFO);
    LogComponentEnable("OnyxTree", LOG_LEVEL_INFO);
    if (verbose)
    {
        LogComponentEnable("CommonUtils", LOG_LEVEL_DEBUG);
        
        //LogComponentEnable("UdpSocket", LOG_LEVEL_ALL);
        //LogComponentEnable("Application", LOG_LEVEL_ALL);
    }
     std::clog.rdbuf(logFile.rdbuf()); // Redirect log output to the file

    Onyx::OnyxTree tree;

    tree.SetupTopology();
    tree.StartApplications();
}
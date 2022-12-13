// Team N.54

//------------------------------------
#include "ns3/applications-module.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/gnuplot.h"
#include "ns3/log.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-socket-address.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
//------ RNG -------------------------
// #include "rng-seed-manager.h"
// Il parametro RngRun viene passato da cmd ( --RngRun=<..>)
// come indicato da una risposta per email, ad RngRun si deve
// passare la somma delle matricole dei componenti del gruppo;
// per questo gruppo il parametro risulta:
// --RngRun = somma numeri di matricola
//------ WiFi ------------------------
#include "ns3/mobility-module.h"
#include "ns3/mobility-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
// #include "wifi-phy-standard.h"
//------ NetAnim ---------------------
#include "ns3/netanim-module.h"
//------ Trace -----------------------
#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"
//------------------------------------

/*
// ============================================ Network Topology ============================================
//
//                                               __ n0 __
//                                           __ /        \__                 AD HOC MODE                                     
//                                          /               \                                               
//                                       n4                  n1                                              
//                                         \                 /                                              
//                                          \               /                                              
//                                           n3 _________ n2                                                 
//                                                                                                          
//                                           192.168.0.1/24                                                            
//                                                                                                                                                                                                                  
// ==========================================================================================================
*/

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("HW2_Task_1_Team_54");      // AD-HOC MODE

int main(int argc, char* argv[]){

    bool useRtsCts = false;  // Forza l'utilizzo dell'handshake RTS/CTS
    bool verbose = false; // Abilita l'uso dei LOG (SRV e CLT) per UDP application
    bool useNetAnim = false; // Genera file per l'analisi su NetAnim

    CommandLine cmd(__FILE__);
    cmd.AddValue("useRtsCts", "Force the use of Rts and Cts", useRtsCts);    // Scelta di useRtsCts da CMD
    cmd.AddValue("verbose", "Enable the use of Logs on SRV and CLI", verbose);    // Scelta di verbose da CMD
    cmd.AddValue("useNetAnim", "Enable file generation for NetAnim", useNetAnim);    // Scelta di useNetAnim da CMD

    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);      //Riferito a NS_LOG_INFO

    ///////////////////////////////////////////////////////////////////////////////

    UintegerValue ctsThreshold = (useRtsCts ? UintegerValue(100) : UintegerValue(2346));                              
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", ctsThreshold);

    ///////////////////////////////////////////////////////////////////////////////

    NS_LOG_INFO("Creazione dei nodi e dei relativi container");        //STATUS LOG INFO LEVEL

    uint32_t nodesNum = 5;      // Numero di nodi totali

    NodeContainer allWifiAdHocModNodes;     // Contenitore di tutti i nodi della rete WiFi AD-HOC
    allWifiAdHocModNodes.Create(nodesNum);

    YansWifiChannelHelper channelAdHocMod = YansWifiChannelHelper::Default();   // Definizione canale di comunicazione tra i nodi
    YansWifiPhyHelper phyAdHocMod;   // Definizione physical layer tra i nodi
    phyAdHocMod.SetChannel(channelAdHocMod.Create());

    WifiMacHelper macAdHocMod; 
    macAdHocMod.SetType("ns3::AdhocWifiMac");   // Definizione modalità operativa
    
    WifiHelper wifiAdHocMod;    // Helper per creare ed installare WiFi devices
    wifiAdHocMod.SetStandard(WifiStandard(WIFI_STANDARD_80211g)); // Definizione di standard da usare, andrà a sovrascrivere i valori di default impostati precedentemente
    wifiAdHocMod.SetRemoteStationManager("ns3::AarfWifiManager");

    NetDeviceContainer adHocModDevices;      // Contenitore finale con nodi collegati con link
    adHocModDevices = wifiAdHocMod.Install(phyAdHocMod, macAdHocMod, allWifiAdHocModNodes);

    NS_LOG_INFO("Fine creazione topologia di rete");        //STATUS LOG INFO LEVEL

    ///////////////////////////////////////////////////////////////////////////// 

    NS_LOG_INFO("Creazione del mobility model");

    MobilityHelper mobilityAdHocMod;

    mobilityAdHocMod.SetPositionAllocator(
        "ns3::GridPositionAllocator",       // Allocate positions on a rectangular 2d grid
        "MinX", DoubleValue(0.0),           // The x coordinate where the grid starts
        "MinY", DoubleValue(0.0),           // The y coordinate where the grid starts
        "DeltaX", DoubleValue(5.0),         // The x space between objects
        "DeltaY", DoubleValue(10.0),        // The y space between objects
        "GridWidth", UintegerValue(3),      // The number of objects layed out on a line
        "LayoutType", StringValue("RowFirst")   // The type of layout
    );

    mobilityAdHocMod.SetMobilityModel(
        "ns3::RandomWalk2dMobilityModel",
        "Bounds", RectangleValue(Rectangle(-90, 90, -90, 90))   // xMin, xMax, yMin, yMax
    );

    mobilityAdHocMod.Install(allWifiAdHocModNodes); 

    NS_LOG_INFO("Fine creazione del mobility model");

    ///////////////////////////////////////////////////////////////////////////////

    NS_LOG_INFO("Creazione del blocco di indirizzi IP per ogni container definito");

    InternetStackHelper allstack;   //InternetStackHelper su tutti i nodi 
    allstack.Install(allWifiAdHocModNodes);

    //------ IP Address -------------
    
    Ipv4AddressHelper ipAddAdHocModNodes;      //Definisco blocco di indirizzi
    ipAddAdHocModNodes.SetBase("192.168.1.0", "255.255.255.0");    //Struttura del blocco di indirizzi
    Ipv4InterfaceContainer adHocModNodesInterfaces;    //Definisco un container con devices e IP Set
    adHocModNodesInterfaces = ipAddAdHocModNodes.Assign(adHocModDevices);   //Assegno il blocco di indirizzi ai devices

    NS_LOG_INFO("Fine definizione blocco di indirizzi IP");        //STATUS LOG INFO LEVEL

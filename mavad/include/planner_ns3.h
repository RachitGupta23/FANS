/**
 * @brief Planner class for the project 
 */
#pragma once

#include "planner_config.h"
#include "planner_ns3_utils.h"
#include "ns3/core-module.h"
#include <cmath>

#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>

#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/internet-stack-helper.h"

#include "ns3/netanim-module.h"

#include "ns3/ipv4-static-routing-helper.h"

/**
 * @brief Trace to capture the pkt number, pkt receive time and sender node id
 * in pkt_rec_time.txt file
 *
 * @param index Receiver node id
 * @param p received pkt
 * @param a sender's ip address
 */
void TraceSink (std::size_t index, ns3::Ptr<const ns3::Packet> p, const ns3::Address& a);

/**
 * @namespace 
 */
namespace rnl{

    /**
     * @brief Drone Socket common for planning and communication
     */
    struct DroneSoc
    {   
        /**
         * @brief Construct a new Drone Soc object
         * 
         */
        DroneSoc ();

        /**
         * @brief send Packet after every n * pktInterval. \n
         * This registers a callback which is periodically called
         * 
         * @param pktInterval
         * 
         * @param n number of nodes in the swarm 
         */
        void sendPacket (ns3::Time pktInterval, int n);

        /**
         * @brief Sends a braoadcast packet.
         *
         * @param pktInterval Deprecated
         * @param n Deprecated
         */
        void sendBcPacket (ns3::Time pktInterval, int n);
        
        /**
         * @brief Socket receiving callback. \n
         * This function will be called as an interrupt if something is received at the socket end 
         * 
         * @param soc Socket at which the message will be received 
         */
        void receivePacket (ns3::Ptr<ns3::Socket> soc);

        /**
         * @brief Terminate all sockets and send shut down command for this node
         */
        void closeSender ();
        
        /**
         * @brief update send message with the correct parent location to follow
         */
        void updateSendMsg ();
        
        /**
         * @brief Setup the node for broadcasting 
         * 
         * @param node Node to setup broadcasting for
         * @param tid Type id
         */
        void setBcSender (ns3::Ptr<ns3::Node> node, ns3::TypeId tid);
        
        /**
         * @brief Initialize the sender for UDP msgs
         * 
         * @param node node 
         * @param tid type id
         * @param ip IP of the receiver socket
         */
        void setSender (ns3::Ptr<ns3::Node> node, ns3::TypeId tid, const std::string& ip);

        /**
         * @brief Initialize the sender for TCP msgs
         * 
         * @param node node 
         * @param self_ip IP of the sender socket
         * @param remote_ip IP of the receiver/remote socket
         * @param startTime Time when sender application will start sending
         */
        void setSenderTCP (ns3::Ptr<ns3::Node> node, const std::string& self_ip, const std::string& remote_ip, ns3::Time startTime);

        /**
         * @brief Initialize the receiver for UDP msgs
         * 
         * @param node node 
         * @param tid type id
         */
        void setRecv   (ns3::Ptr<ns3::Node> node, ns3::TypeId tid);

        /**
         * @brief Initialize the receiver for TCP msgs
         * 
         * @param node node 
         * @param ip IP of the receiver
         * @param num_nodes Number of nodes
         * @param stopTime Time when receiver application can stop
         */
        void setRecvTCP (ns3::Ptr<ns3::Node> node, const std::string& ip, int num_nodes, ns3::Time stopTime);
        
        ns3::Ptr<ns3::Socket>         source; /**< Socket for sending unicast messages */
        ns3::Ptr<ns3::Socket>         source_bc; /**< Socket for sending broadcast messages */

        ns3::Ptr<ns3::Socket>         recv_sink; /**< Receiver/sink socket */
        int                           id; /**< Id of this drone soc */
        int                           anch_id; /**< Anchoring Id if any */
        int                           circle_dir; /**Circling direction */
        ns3::Vector3D                 anch_pos; /**< Anchoring position */
        rnl::USMsg                    msg_send; /**< message to send */
        rnl::URMsg                    msg_rec; /**< Message received */
        rnl::Nbt                      nbt; /**< Neighbour table */
        std::vector<ns3::Vector3D>    wpts; /**< Waypoints that drone needs to follow */
        ns3::Vector3D                 pos; /**< Current position of the drone */
        int                           lookaheadindex; /**< Look ahead index for the drone */
        int                           toggle_bc; /**< toggle broadcast on */

        ros::Publisher                drone_lk_ahead_pub;
        ros::Subscriber               drone_pos_sub;

        /**
         * @brief Initializes the ros parameters.
         *
         * @param nh node handle
         */
        void initializeRosParams(ros::NodeHandle& nh);

        /**
         * @brief Publishes the look ahead index
         */
        void publishLookAhead ();

        /**
         * @brief Set the (subscribed) position
         *
         * @param _pos Position
         */
        void posSubCb (const geometry_msgs::PoseStamped& _pos);
    };

    /**
     * @class 
     * @brief Wifi properties set in this class and passed to planner
     * 
     */
    class Properties
    {
       public:
            
            /**
             * @brief Construct a new Properties object
             * 
             * @param _phyMode Physical layer mode
             * @param _rss Deprecated 
             * @param _num_nodes Number of nodes in the swarm
             */
            Properties(
                std::string _phyMode, 
                double _rss, 
                int _num_nodes
            );
            
            /*********************************************************/
            /*NS3 Functions*/

            /**
             * @brief For Realtime simulation set realtime and checksum 
             * 
             * @param realtime Simulation will run realtime
             * @param checksum Checksum is required for setting data checks 
             */
            void initialize(bool realtime, bool checksum);
            
            /**
             * @brief Set the Wifi object
             * 
             * @param verb If verbose required
             * @param pcap_enable If pcap is enabled
             */
            void setWifi(bool verb, bool pcap_enable);

            /**
             * @brief Set the Internet
             */
            void setInternet();

            /**
             * @brief Sets the static route.
             *
             * @param n            node
             * @param destination  IP address of destination
             * @param nextHop      IP address of next hop
             * @param interface    Interface
             */
            void SetStaticRoute(ns3::Ptr<ns3::Node> n, const char* destination, const char* nextHop, uint32_t interface);

            ns3::NodeContainer c; /**< Node container containing all the nodes */
            
            /**
             * @brief Get type id value
             * 
             * @return ns3::TypeId 
             */
            ns3::TypeId  tid_val () const;

            /**
             * @brief Set type id value
             * 
             * @return ns3::TypeId& 
             */
            ns3::TypeId& tid_val ();
            
            
        private:
            ns3::WifiHelper wifi; /**< Wifi Helper */

            ns3::YansWifiPhyHelper wifiPhy; /**< YansWifiHelper for ease of use */

            ns3::YansWifiChannelHelper wifiChannel; /**< channel level properties */ 

            ns3::WifiMacHelper wifiMac; /**< Adding mac layer */
            
            ns3::NetDeviceContainer devices; /**< Virtual net device to be used */

            ns3::InternetStackHelper internet; /**< InternetStackHelper */

            ns3::Ipv4StaticRoutingHelper staticRouting; /** Static Routing Helper */

            ns3::Ipv4AddressHelper ipv4; /**< Ipv4AddressHelper used for setting ips to node */
            ns3::Ipv4InterfaceContainer i; /**< This is ips assigned to nodes */ 

            ns3::TypeId tid; /**< Type ID being used */

            std::string phy_mode; /**< Phy Mode */
            double rss;  /**< Rss (in dBm) value Deprecated*/
            int num_nodes; /**< Number of nodes */

            ns3::AsciiTraceHelper ascii;
    };

    /**
     * @brief Initial neighbour table to be set. Since we are initializing the nodes in linear fashion \n 
     * Setting it requires only index of the node. \n
     * Set to id + 1 and id - 1 as one hop neighbours
     * 
     * @param id Index
     * @param n number of nodes
     * @return rnl::Nbt neighbour table
     */
    rnl::Nbt   setinitialNbt (int id, int n);
    
    /**
     * @brief Set the initial message to be sent to successor
     * 
     * @param nbt Initial neighbour table
     * @param id source id
     * @param n number of nodes
     * @return rnl::USMsg 
     */
    rnl::USMsg setinitialSMsg (rnl::Nbt nbt, int id, int n);


    /**
     * @brief Planner class. Flow \n
     * 1. Initialze Dronesoc for every UAV \n
     * 2. Initialize the mobility (Positions) of each UAV \n
     * 3. set the initial exploration path of leader \n
     * 4. Start simulation 
     */
    class Planner{
        public:
            /**
             * @brief Construct a new Planner object
             * 
             * @param p wifi properties
             * @param _no_nodes number of nodes
             * @param _pkt_int packet interval
             * @param pos_int position interval, this is to be used for broadcasting positions once UAV reaches desired location
             * @param _stp Stop time for simulation
             */

            /**
             * @brief      Construct a new Planner object
             *
             * @param      _nh          node handle
             * @param      _nh_private  private node handle
             * @param      p            wifi properties
             * @param      _no_nodes    number of nodes
             * @param      _pkt_int     packet interval
             * @param      pos_int      position interval, this is to be used
             *                          for broadcasting positions once UAV
             *                          reaches desired location
             * @param      _stp         Stop time for simulation
             */
            Planner (ros::NodeHandle& _nh, ros::NodeHandle& _nh_private, rnl::Properties& p, int _no_nodes, float _pkt_int, float pos_int, float _stp);
            
            /**
             * @brief Initialize positions of each UAV 
             */
            void initializeMobility ();

            /**
             * @brief Initialize sockets
             * 
             */
            void initializeSockets ();

            /**
             * @brief Set the Initial Leader Explore Path
             */
            void setLeaderExplorePath ();
            
            /**
             * @brief   Check if the drone reached the site or not
             *
             * @param   node_pos  Current node position
             * @param   ID        ID of the drone
             *
             * @return  true if drone reached else false
             */
            static bool siteReached (ns3::Vector3D node_pos, int ID);

            /**
             * @brief Start simulation
             */
            void startSimul ();

            /**
             * @brief Increment look ahead point. (Assuming no dynamics), spawning nodes at distances
             */
            void incLookAhead ();
            
            /**
             * @brief      Update the state of CENTRE drones and sends control commands to their neighbours
             */
            void updateStateofCentre ();

            /**
             * @brief Update the position of UAVs after one time step
             */
            void updatePosSocs();

            /**
             * @brief update waypoints of the node with index id \n
             * Here we check for the State of the UAV. This function acts \n
             * as a FSM, and decides what next action the UAV must take depending on \n
             * the commands from its successor
             * 
             * @param id index of UAV
             */
            void updateWpts(int id);

            /**
             * @brief Just calls updateWaypoints for every node in the swarm
             */
            void updateSocsfromRec ();

            /**
             * @brief  Start lawn mover scanning.
             *
             * @param  interval  The interval after which the cycle of lawn
             *                   mover scanning is repeated
             * @param  id        ID of the drone
             * @param  pos0      Initial position of the drone
             */
            void doLawnMoverScanning(ns3::Time interval, int id, ns3::Vector3D pos0);

            /**
             * @brief      Update the state of UAVs (Currently only LEFT and RIGHT drones)
             */
            void updateSocs  ();

            /**
             * @brief Advance the position, runs repeatedly after interval time
             * 
             * @param interval time after which this function to repeat. Will determine the speed of simulation
             */
            void advancePos (ns3::Time interval);
            void takeOff (double _t);
            bool withinThreshold (const rnl::DroneSoc* _soc);
            
            static ns3::Vector3D       disas_centre; /**< known centre of the disaster site to monitor*/

        private:
            ros::NodeHandle nh;
            ros::NodeHandle nh_private;
            rnl::Properties            wifi_prop; /**< wifi properties object */
            std::vector<rnl::DroneSoc> nsocs; /**< UAV Drone socs in the simulation, Each DroneSoc represents a UAV */
            int                        num_nodes; /**< number of nodes */

            ns3::MobilityHelper        mobility; /**< Mobility helper to set the initial mobility of the nodes */
            ns3::Time                  pkt_interval; /**< Unicast packet interval */
            ns3::Time                  pos_interval; /**< Interval after which advancePos repeats */
            ns3::Time                  stopTime; /**< Stop time */
            int                        leader_id; /**< Leader index */
            int                        ldirec_flag; /**< Deprecated */
            int                        lchild_id; /**< Child index */
            int                        tail_id; /**< Child index */
    };
};

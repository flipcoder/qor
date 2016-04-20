#ifndef _NET_H_RKPLEW0
#define _NET_H_RKPLEW0
#include <raknet/RakPeer.h>
#include "IRealtime.h"
#include "Session.h"
#include <boost/signals2.hpp>

class Qor;

class Net:
    public Session::IModule
{
    public:
        Net(Qor* engine, bool server, int connections=8);
        virtual ~Net();
        
        void destroy();

        virtual void logic(Freq::Time t) override;


        boost::signals2::signal<void(RakNet::Packet*)> on_data;
        boost::signals2::signal<void(RakNet::Packet*)> on_disconnect;
        boost::signals2::signal<void(RakNet::Packet*)> on_connect;
        boost::signals2::signal<void(RakNet::Packet*)> on_connection_lost;
        boost::signals2::signal<void(RakNet::Packet*)> on_full;
        
    public:
        
        RakNet::RakPeerInterface* m_pSocket = nullptr;
};

#endif


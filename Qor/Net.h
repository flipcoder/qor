#ifndef _NET_H_RKPLEW0
#define _NET_H_RKPLEW0
#ifndef QOR_NO_NET
#include <raknet/RakPeer.h>
#include <raknet/MessageIdentifiers.h>
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

        bool local() const { return not m_pSocket; }
        bool remote() const { return m_pSocket && not m_bServer; }
        bool server() const { return m_bServer; }

        boost::signals2::signal<void(RakNet::Packet*)> on_data;
        boost::signals2::signal<void(RakNet::Packet*)> on_disconnect;
        boost::signals2::signal<void(RakNet::Packet*)> on_connect;
        boost::signals2::signal<void(RakNet::Packet*)> on_connection_lost;
        boost::signals2::signal<void(RakNet::Packet*)> on_full;
        RakNet::RakPeerInterface* socket() { return m_pSocket; }
        
    public:
        
        RakNet::RakPeerInterface* m_pSocket = nullptr;
        bool m_bServer = false;
};

#endif
#endif


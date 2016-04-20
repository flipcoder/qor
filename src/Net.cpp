#include "Net.h"
#include "Qor.h"
#include "raknet/MessageIdentifiers.h"

using namespace RakNet;
using namespace std;

Net :: Net(Qor* engine, bool server, int connections)
{
    try{
        if(not server){
            string ip = engine->args().value("ip").c_str();
            if(not ip.empty()){
                LOGf("Connecting to %s...", ip);
                m_pSocket = RakNet::RakPeerInterface::GetInstance();
                SocketDescriptor sd(0, 0);
                sd.socketFamily=AF_INET;
                m_pSocket->Startup(connections, &sd, 1);
                m_pSocket->Connect(ip.c_str(), 11523, 0, 0);
                m_pSocket->SetOccasionalPing(true);
            }
        } else {
            LOG("Starting server...");
            m_pSocket = RakNet::RakPeerInterface::GetInstance();
            SocketDescriptor sd(11523, 0);
            sd.socketFamily=AF_INET;
            m_pSocket->Startup(connections, &sd, 1);
            m_pSocket->SetMaximumIncomingConnections(connections);
            m_pSocket->SetOccasionalPing(true);
            m_pSocket->SetUnreliableTimeout(1000);
        }
    }catch(...){
        destroy();
        throw;
    }
}

void Net :: destroy()
{
    if(m_pSocket){
        m_pSocket->Shutdown(300);
        RakPeerInterface::DestroyInstance(m_pSocket);
    }
}

Net :: ~Net()
{
    destroy();
}

void Net :: logic(Freq::Time t)
{
    if(not m_pSocket)
        return;
    
    RakNet::Packet *packet;
    while(true)
    {
        packet = m_pSocket->Receive();
        if(not packet)
            break;
        
        auto guid = RakNetGUID::ToUint32(packet->guid);
        try{
            switch(packet->data[0])
            {
                case ID_CONNECTION_REQUEST_ACCEPTED:
                    LOG("Connected.");
                    on_connect(packet);
                    break;
                case ID_NO_FREE_INCOMING_CONNECTIONS:
                    LOG("No free incoming connections.");
                    on_full(packet);
                    break;
                case ID_DISCONNECTION_NOTIFICATION:
                    LOG("Disconnected.");
                    on_disconnect(packet);
                    break;
                case ID_CONNECTION_LOST:
                    LOG("Connection lost.");
                    on_connection_lost(packet);
                    break;
                case ID_CONNECTION_ATTEMPT_FAILED:
                    LOG("Connection attempt failed.");
                    break;
                case ID_SND_RECEIPT_ACKED:
                    LOG("Send receipt acked.");
                    break;
                case ID_SND_RECEIPT_LOSS:
                    LOG("Send receipt loss.");
                    break;

                // server only?
                case ID_NEW_INCOMING_CONNECTION:
                    LOGf("Client %s (%s) connected.", guid % packet->systemAddress.ToString(true));
                    on_connect(packet);
                    break;
                case ID_REMOTE_NEW_INCOMING_CONNECTION:
                    LOGf("%s connected.", guid);
                    break;
                case ID_INCOMPATIBLE_PROTOCOL_VERSION:
                    LOG("Incompatible protocol version.");
                    break;
                case ID_CONNECTED_PING:
                case ID_UNCONNECTED_PING:
                    LOG("Ping");
                    break;
                default:
                    LOG("Packet Data");
                    on_data(packet);
                    break;
            }
            m_pSocket->DeallocatePacket(packet);
        }catch(...){
            m_pSocket->DeallocatePacket(packet);
            throw;
        }
    }
}


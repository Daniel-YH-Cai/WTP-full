#include "UDPSocket.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

class BatchReceiver
{

public:
    UDPSocket *s;
    // windows base: the first chunk in the windows;
    // the next packet to be send and to wait for ack
    string file;
    ofstream logfile;
    int window_size;
    Packet **window;
    int fileCount;
    // the number of packets already sent and are waiting for ack
    BatchReceiver(int ws, const char *filename, const char *log)
    {
        s = nullptr;
        window_size = ws;
        window = (Packet **)malloc(ws * sizeof(Packet *));
        for (int i = 0; i < ws; i++)
        {
            window[i] = nullptr;
        }
        logfile = ofstream(log);
        file = filename;

        fileCount = 0;
    }

    void set_UDPSocket(UDPSocket *sock)
    {
        cout << "Socket set!\n";
        s = sock;
    }

    void writeFile(ofstream &file)
    {
        file.write(window[0]->data, window[0]->get_length());
        for (int i = 0; i < window_size - 1; i++)
        {
            window[i] = window[i + 1];
        }
        window[window_size - 1] = nullptr;
    }
};

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        cout << "Usage: ./wSender\n";
        return 0;
    }
    int port = atoi(argv[1]);
    int window_size = atoi(argv[2]);
    UDPSocket udp;
    if (!udp.bind_me(port))
    {
        return 0;
    }
    // receiver.bind_me(8888);
    cout << "Start receicing\n";
    BatchReceiver receiver(atoi(argv[2]), argv[3], argv[4]);
    receiver.set_UDPSocket(&udp);

    Packet p;
    receiver.s->receivePacket(&p);
    receiver.logfile << p.get_type() << " " << p.get_seqNum()
                     << " " << p.get_length() << " " << p.get_checksum() << "\n";
    if (p.get_type() == 0)
    {
        // ack for start
        Packet startP(p.get_seqNum());
        receiver.s->sendPacket(startP);
        receiver.logfile << startP.get_type() << " " << startP.get_seqNum()
                         << " " << startP.get_length() << " " << startP.get_checksum() << "\n";
        ofstream file = ofstream(receiver.file + "/FILE-" + to_string(receiver.fileCount++) + ".out");
        int num = 0;
        int max = 0;
        cout << "This is new receiver\n";
        while (true)
        {
            Packet *pData = new Packet(0);
            if(!receiver.s->receivePacket(pData)){
                cout<<"lost";
                continue;
            }
            receiver.logfile << pData->get_type() << " " << pData->get_seqNum()
                             << " " << pData->get_length() << " " << pData->get_checksum() << "\n";
            if (pData->get_type() == 0)
            {
                receiver.s->sendPacket(startP);
                receiver.logfile << startP.get_type() << " " << startP.get_seqNum()
                                 << " " << startP.get_length() << " " << startP.get_checksum() << "\n";
                continue;
            }

            // For each packet received
            if (pData->get_type() == 1 && pData->get_seqNum() == p.get_seqNum())
            {
                // ack for end
                Packet endP(pData->get_seqNum());
                receiver.s->sendPacket(endP);
                receiver.logfile << endP.get_type() << " " << endP.get_seqNum()
                                 << " " << endP.get_length() << " " << endP.get_checksum() << "\n";
                break;
            }
            if (pData->checkSum())
            {
                if (pData->get_seqNum() == num)
                {
                    // if (pData.get_seqNum() > max)
                    // {
                    //     max = pData.get_seqNum();
                    // }
                    receiver.window[0] = pData;
                    Packet newP(num);
                    while (receiver.window[0] != nullptr)
                    {
                        receiver.writeFile(file);
                        num += 1;
                    }
                    
                    cout << "I will send a ack packet with seq " << newP.get_seqNum() << "\n";
                    receiver.s->sendPacket(newP);
                    //receiver.logfile << "ACK Sent\n";
                    receiver.logfile << newP.get_type() << " " << newP.get_seqNum()
                                     << " " << newP.get_length() << " " << newP.get_checksum() << "\n";
                }
                else
                {
                    if (pData->get_seqNum() > num && pData->get_seqNum() <= num + receiver.window_size)
                    {
                        receiver.window[pData->get_seqNum() - num] = pData;
                    }
                    Packet newP (pData->get_seqNum());
                    cout<<"I will send a ack packet with seq "<<newP.get_seqNum()<<"\n";

                    receiver.s->sendPacket(newP);
                    //receiver.logfile << "ACK Send\n";
                    receiver.logfile << newP.get_type() << " " << newP.get_seqNum()
                                     << " " << newP.get_length() << " " << newP.get_checksum() << "\n";
                }
            }
        }
    }

    return 0;
}

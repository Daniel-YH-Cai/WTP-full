#include "UDPSocket.h"
#include <iostream>
#include <fstream>
#define MAX_START_ACK 2048
using namespace std;

// TODO: avoid hardcoding size
class BatchSender
{
    UDPSocket *s;
    // windows base: the first chunk in the windows;
    // the next packet to be send and to wait for ack
    int window_base;
    // the number of packets already sent and are waiting for ack
    int waiting_ack;
    int length;
    ifstream file;
    ofstream logfile;
    int window_size;

public:
    static const int chunk_size = DATA_SIZE;
    BatchSender(const char *filename, const char *log, int ws)
    {
        window_size = ws;
        waiting_ack = 0;
        window_base = 0;
        s = nullptr;
        logfile = ofstream(log);
        file = ifstream(filename, ios::binary);
        file.seekg(0, ios::end);
        length = file.tellg();
        file.seekg(0, ios::beg);
    };
    // send the index th chunck of the file
    void set_UDPSocket(UDPSocket *sock)
    {
        cout << "Socket set!\n";
        s = sock;
    }
    // send a chunk (1024 bytes) of the file and increase waiting_ack by 1
    // since data packet initial seq_num is zero, index=seq_num
    void send_chunk(int index)
    {
        if (chunk_size * index > length)
        {
            cout << "Invalid index!\n";
        }
        else
        {
            char buffer[1024] = {0};
            cout << "Send chunk with index: " << index << "\n";
            file.seekg(index * chunk_size, ios::beg);
            int packet_data_size=0;
            if(index==length/chunk_size){
                packet_data_size=length%chunk_size;
                file.read(buffer, packet_data_size);
                file.clear();
            }
            else{
                packet_data_size=chunk_size;
                file.read(buffer, packet_data_size);
            }
            Packet p(buffer, index,packet_data_size);
            cout<<"The content before packeting: "<<buffer<<"\n";
            s->sendPacket(p);
            logfile << p.get_type() << " " << p.get_seqNum()
                    << " " << p.get_length() << " " << p.get_checksum() << "\n";
            waiting_ack++;

        }
    }

    // return the new sequence number after sending the packets;
    // can send less then window_size packets (no longer used)
    int send_window(int seqNumber, int index, int window_size)
    {
        if (chunk_size * index > length)
        {
            cout << "Invalid index!\n";
            return seqNumber;
        }
        file.seekg(index * chunk_size, ios::beg);
        char buffer[1024] = {0};
        for (int i = 0; i < window_size; i++)
        {
            if ((index + i) * chunk_size <= length)
            {
                file.read(buffer, chunk_size);
                Packet p(buffer, seqNumber,chunk_size);
                s->sendPacket(p);
                seqNumber++;
                logfile << p.get_type() << " " << p.get_seqNum()
                        << " " << p.get_length() << " " << p.get_checksum() << "\n";
            }
        }
        return seqNumber;
    }
    // Perform one cycle of sending packet and waiting ack;
    // Take in a bool representing whether ack is received in the previous round;
    // return whether a proper ack is received
    void cycle()
    {
        // send next packet
        int to_send = window_size - waiting_ack;
        for (int i = 0; i < to_send; i++)
        {
            send_chunk(window_base + i);
        }
        Packet response;
        if (s->receivePacketTimeout(&response))
        {
            logfile << response.get_type() << " " << response.get_seqNum()
                    << " " << response.get_length() << " " << response.get_checksum() << "\n";
            if (response.isValidACK() && response.get_seqNum() > window_base)
            {
                waiting_ack = waiting_ack - (response.get_seqNum() - window_base);
                window_base = response.get_seqNum();
                cout << "Receive ack: " << response.get_seqNum() << " New base: " << window_base << "\n";
                cout << "Now only " << waiting_ack << " packets "
                     << "wait for ack";
            }
            else
            {
                cout << "Receive low ack: " << response.get_seqNum() << " while " << window_base
                     << " is expected\n";
                waiting_ack = 0;
            }
        }
        else
        {
            cout << "Waiting for ack of :" << window_base << " time out\n";
            waiting_ack = 0;
        }
    }
    bool finished()
    {
        return window_base == length / chunk_size + 1;
    }
    ~BatchSender()
    {
        file.close();
    }
};

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        cout << "Usage: ./wSender\n";
        return 0;
    }
    int port = atoi(argv[2]);
    int window_size = atoi(argv[3]);
    UDPSocket udp;
    udp.set_other(port, argv[1]);
    // send start packet
    int initial_seq = random() % MAX_START_ACK;
    Packet start = Packet::StartPacket(initial_seq);
    Packet response;
    cout<<"Request start\n";
    while (true)
    {
        udp.sendPacket(start);
        if (udp.receivePacketTimeout(&response))
        {
            if (response.isValidACK() && response.get_seqNum() == initial_seq)
            {
                break;
            }
        }
        response.reset();
    }
    cout<<"Start!\n";
    BatchSender bsender(argv[4], argv[5], window_size);
    bsender.set_UDPSocket(&udp);
    while (!bsender.finished())
    {
        // firstly send five packets, then wait for one ack
        // if not acked, then resent five of them
        // if acked, then send one more packet and move the windows forward
        bsender.cycle();
    }
    Packet end=Packet::EndPacket(initial_seq);
    cout<<"Request end!\n";
    while (true)
    {
        udp.sendPacket(end);
        if (udp.receivePacketTimeout(&response))
        {
            if (response.isValidACK() && response.get_seqNum() == initial_seq)
            {
                break;
            }
        }
        response.reset();
    }
    cout<<"End!\n";
    return 0;
}

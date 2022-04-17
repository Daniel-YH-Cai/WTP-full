#include "UDPSocket.h"
#include <iostream>
#include <fstream>
#include <sys/time.h>
#define MAX_START_ACK 2048
using namespace std;
#define SENT_AWAIT_ACK 1
#define NOT_SENT 2
#define SENT_ACKED 3


//time utils
double msElapsed(struct timeval* start,struct timeval* end){
    long sec=end->tv_sec-start->tv_sec;
    long millisec=end->tv_usec-start->tv_usec;
    double ans=1000.0*sec+millisec/1000.0;
    if(ans<0){
        cout<<"Invalid time!\n";
    }
    return ans;
}
// TODO: avoid hardcoding size
class AdvancedBatchSender
{

    UDPSocket *s;
    // windows base: the first chunk in the windows;
    // the next packet to be send and to wait for ack
    int window_base;
    // the number of packets already sent and are waiting for ack
    int waiting_ack;
    int length;
    ifstream file;
    int num_packet;
    int* status;
    ofstream logfile;
    int window_size;
    int start_seq;

public:
    static const int chunk_size = DATA_SIZE;
    AdvancedBatchSender(const char *filename, const char *log, int ws,int first_seq)
    {
        window_size = ws;
        waiting_ack = 0;
        window_base = 0;
        start_seq=first_seq;
        s = nullptr;
        logfile = ofstream(log);
        file = ifstream(filename, ios::binary);
        file.seekg(0, ios::end);
        length = file.tellg();
        file.seekg(0, ios::beg);
        num_packet=length/chunk_size+1;
        status=new int [num_packet];
        for(int i=0;i<num_packet;i++){
            status[i]=NOT_SENT;
        }
        cout<<"Length of file: "<<num_packet<<" packets; "<<"Window size: "<<window_size<<"\n";
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
            //cout<<"The content before packeting: "<<buffer<<"\n";
            s->sendPacket(p);
            logfile << p.get_type() << " " << p.get_seqNum()
                    << " " << p.get_length() << " " << p.get_checksum() << "\n";
            waiting_ack++;

        }
    }

    // return the new sequence number after sending the packets;
    // can send less then window_size packets (no longer used)
//    int send_window(int seqNumber, int index, int window_size)
//    {
//        if (chunk_size * index > length)
//        {
//            cout << "Invalid index!\n";
//            return seqNumber;
//        }
//        file.seekg(index * chunk_size, ios::beg);
//        char buffer[1024] = {0};
//        for (int i = 0; i < window_size; i++)
//        {
//            if ((index + i) * chunk_size <= length)
//            {
//                file.read(buffer, chunk_size);
//                Packet p(buffer, seqNumber,chunk_size);
//                s->sendPacket(p);
//                seqNumber++;
//                logfile << p.get_type() << " " << p.get_seqNum()
//                        << " " << p.get_length() << " " << p.get_checksum() << "\n";
//            }
//        }
//        return seqNumber;
//    }
    // Perform one cycle of sending packet and waiting ack;
    // Take in a bool representing whether ack is received in the previous round;
    // return whether a proper ack is received
    void cycle(){
        for(int i=window_base;i<window_base+window_size;i++){
            if(i<num_packet){
                if(status[i]==NOT_SENT){
                    cout<<"Packet "<<i<<" sent\n";
                    send_chunk(i);
                    status[i]=SENT_AWAIT_ACK;
                }
            }
            else break;
        }
        Packet buffer;
        timeval start;memset(&start,0,sizeof (timeval));
        timeval end;memset(&end,0,sizeof (timeval));
        gettimeofday(&start, nullptr);
        gettimeofday(&end, nullptr);
        while(msElapsed(&start,&end)<500){
            if(s->nonBlockReceive(&buffer)){
                if(buffer.isValidACK()){
                    if(buffer.get_seqNum()<window_base){
                        cout<<"ACK value low\n";
                    }
                    else if(buffer.get_seqNum()==window_base){
                        cout<<"Window base packet "<<buffer.get_seqNum()<<" received\n";
                        status[buffer.get_seqNum()]=SENT_ACKED;
                        //window_base++;
                        //increase the window base for all acked
                        for(int i=window_base;i<window_base+window_size;i++){
                            if(i<num_packet){
                                if(status[i]==SENT_ACKED){
                                    window_base++;
                                }
                                else{
                                    break;
                                }
                            }
                        }
                    }
                    else if(buffer.get_seqNum()>window_base&&buffer.get_seqNum()<window_base+window_size){
                        if(status[buffer.get_seqNum()]==SENT_AWAIT_ACK){
                            cout<<"Packet "<<buffer.get_seqNum()<<" buffered\n";
                            status[buffer.get_seqNum()]=SENT_ACKED;
                        }
                        else{
                            cout<<"Error: ACKed a packet not yet send!\n";
                        }
                    }
                    else{
                        cout<<"ACK value too high!\n";
                    }
                }
                else{

                }
            }
            else{
                usleep(2500);
            }
            gettimeofday(&end, nullptr);
            buffer.reset();
        }
        for(int i=window_base;i<window_base+window_size;i++){
            if(i<num_packet){
                if(status[i]==NOT_SENT){
                    break;
                }
                else if(status[i]==SENT_AWAIT_ACK){
                    status[i]=NOT_SENT;
                }
            }
            else break;
        }
    }
    bool finished()
    {
        for(int i=0;i<num_packet;i++){
            if(status[i]!=SENT_ACKED){
                cout<<"Packet "<<i<<" is not yet sent!\n";
                return false;
            }
        }
        return true;
    }
    ~AdvancedBatchSender()
    {
        file.close();
        delete[] status;
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
    AdvancedBatchSender bsender(argv[4], argv[5], window_size,initial_seq);
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

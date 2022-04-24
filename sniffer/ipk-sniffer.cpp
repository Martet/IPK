#include "pcap.h"
#include <iostream>
#include <getopt.h>
#include <string>
#include <string.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <net/if_arp.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

void error(std::string message){
    std::cerr << message << "\n";
    exit(1);
}

void handler(u_char *args, const struct pcap_pkthdr *pkthdr, const u_char *packet){
    char buf[100];
    strftime(buf, 99, "%FT%T%z", localtime(&pkthdr->ts.tv_sec));
    std::cout << "timestamp: " << buf << "\n";
    (void)args;

    struct ether_header *header = (struct ether_header *) packet;
    std::cout << "src MAC: " << ether_ntoa((const struct ether_addr *) header->ether_shost) << "\n";
    std::cout << "dst MAC: " << ether_ntoa((const struct ether_addr *) header->ether_dhost) << "\n";
    std::cout << "frame length: " << pkthdr->len << " bytes\n";

    u_short type = ntohs(header->ether_type);
    if(type == ETHERTYPE_IP){
        const struct ip *ip = (const struct ip *) (packet + sizeof(struct ether_header));
        std::cout << "src IP: " << inet_ntoa(ip->ip_src) << "\n";
        std::cout << "dst IP: " << inet_ntoa(ip->ip_dst) << "\n";

        int offset = sizeof(struct ether_header);
        if(ip->ip_p == IPPROTO_TCP){
            struct tcphdr *tcp = (struct tcphdr *)(packet + sizeof(struct ether_header) + (ip->ip_hl & 0x0F) * 4);
            std::cout << "src port: " << tcp->th_sport << "\n";
            std::cout << "dst port: " << tcp->th_dport << "\n";
            offset += (ip->ip_hl & 0x0F) * 4 + ((tcp->th_off & 0xF0) >> 4);
        }
        else if(ip->ip_p == IPPROTO_UDP){
            struct udphdr *udp = (struct udphdr *)(packet + sizeof(struct ether_header) + (ip->ip_hl & 0x0F) * 4);
            std::cout << "src port: " << udp->uh_sport << "\n";
            std::cout << "dst port: " << udp->uh_dport << "\n";
            offset += (ip->ip_hl & 0x0F) * 4 + sizeof(struct udphdr);
        }
        else
            return;
        
        const u_char *payload = packet + offset;
        for(unsigned int i = 0; i < pkthdr->len; i += 16){
            printf("0x%04hhx: ", i);
            for(unsigned int j = 0; j < 16; j++){
                printf("%02hhx ", payload[i + j]);
            }
            for(unsigned int j = 0; j < 16; j++){
                printf("%c ", payload[i + j] >= 32 && payload[i + j] <= 127 ? payload[i + j] : '.');
            }
            printf("\n");
        }
    }
}

int main(int argc, char *argv[]){
    int flag, filter = 0;
    struct option options[] = {
        {"interface", required_argument, NULL, 'i'}, 
        {"tcp", no_argument, &flag, 't'},
        {"udp", no_argument, &flag, 'u'},
        {"arp", no_argument, &flag, 'a'},
        {"icmp", no_argument, &flag, 'c'},
        {0, 0, 0, 0}};
    int c, port = 0, n = 1;
    std::string interface;
    while((c = getopt_long(argc, argv, "i:tuacp:n:", options, NULL)) != -1){
        switch (c){
            case 'i':
                interface = std::string(optarg);
                break;
            case 0:
                if(filter && filter != flag)
                    error("Argument parsing error");
                filter = flag;
                break;
            case 't': case 'u':
                if(filter && filter != c)
                    error("Argument parsing error");
                filter = c;
                break;
            case 'n': case 'p':
                try{
                    if(c == 'n')
                        n = std::stoi(optarg, nullptr, 10);
                    else
                        port = std::stoi(optarg, nullptr, 10);
                } catch(...){
                    error("Argument parsing error");
                }
                break;
            case '?': default:
                error("Argument parsing error");
        }
    }

    if(interface == ""){
        pcap_if_t *devs;
        if(pcap_findalldevs(&devs, NULL) == PCAP_ERROR)
            error("Error getting interfaces");
        pcap_if_t *dev = devs;
        while(dev){
            std::cout << dev->name << "\n";
            dev = dev->next;
        }
        pcap_freealldevs(devs);
        return 0;
    }

    char errbuf[PCAP_ERRBUF_SIZE];
    bpf_u_int32 ip, mask;
    if(pcap_lookupnet(interface.c_str(), &ip, &mask, errbuf) == PCAP_ERROR)
        error("Error looking up interface: " + std::string(errbuf));

    pcap_t *pcap;
    if((pcap = pcap_open_live(interface.c_str(), BUFSIZ, 1, -1, errbuf)) == NULL)
        error("Error opening interface for sniffing: " + std::string(errbuf));

    std::string filter_str;
    if(filter)
        switch(filter){
            case 't':
                filter_str = "tcp ";
                break;
            case 'u':
                filter_str = "udp ";
                break;
            case 'a':
                filter_str = "arp ";
                break;
            case 'c':
                filter_str = "icmp ";
                break;
        }
    if(port)
        filter_str += "port " + std::to_string(port);

    struct bpf_program program;
    if(pcap_compile(pcap, &program, filter_str.c_str(), 0, ip) == PCAP_ERROR)
        error("Error compiling filters");
    if(pcap_setfilter(pcap, &program) == PCAP_ERROR)
        error("Error applying filters");
    
    return pcap_loop(pcap, n, handler, NULL);
}


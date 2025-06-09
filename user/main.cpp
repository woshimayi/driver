#include <iostream>
#include <pcap.h>
#include <string>
#include <cstring>

#pragma comment(lib, "wpcap.lib")
#pragma comment(lib, "Packet.lib")

void print_devices() {
    pcap_if_t* alldevs;
    char errbuf[PCAP_ERRBUF_SIZE];
    
    if (pcap_findalldevs(&alldevs, errbuf) == -1) {
        std::cerr << "Error in pcap_findalldevs: " << errbuf << std::endl;
        return;
    }

    std::cout << "Available network interfaces:" << std::endl;
    for (pcap_if_t* d = alldevs; d; d = d->next) {
        std::cout << d->name;
        if (d->description)
            std::cout << " (" << d->description << ")";
        std::cout << std::endl;
    }

    pcap_freealldevs(alldevs);
}

bool send_packet(const char* device_name, const u_char* packet_data, int packet_len) {
    pcap_t* handle;
    char errbuf[PCAP_ERRBUF_SIZE];

    // Open the device for sending packets
    handle = pcap_open_live(device_name, BUFSIZ, 1, 1000, errbuf);
    if (!handle) {
        std::cerr << "Couldn't open device " << device_name << ": " << errbuf << std::endl;
        return false;
    }

    // Send the packet
    if (pcap_sendpacket(handle, packet_data, packet_len) != 0) {
        std::cerr << "Error sending packet: " << pcap_geterr(handle) << std::endl;
        pcap_close(handle);
        return false;
    }

    std::cout << "Packet sent successfully!" << std::endl;
    pcap_close(handle);
    return true;
}

int main() {
    // Print available network interfaces
    print_devices();

    // Example: Create a simple ICMP echo request packet
    u_char packet[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Destination MAC
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Source MAC
        0x08, 0x00,                          // EtherType (IPv4)
        0x45, 0x00, 0x00, 0x1C,              // IP Header
        0x00, 0x00, 0x40, 0x00,
        0x40, 0x01, 0x00, 0x00,
        0x7F, 0x00, 0x00, 0x01,              // Source IP
        0x7F, 0x00, 0x00, 0x01,              // Destination IP
        0x08, 0x00,                          // ICMP Type (Echo Request)
        0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    // Get device name from user
    std::string device_name;
    std::cout << "Enter the device name to send packet: ";
    std::getline(std::cin, device_name);

    // Send the packet
    if (send_packet(device_name.c_str(), packet, sizeof(packet))) {
        std::cout << "Packet sent successfully!" << std::endl;
    } else {
        std::cerr << "Failed to send packet." << std::endl;
    }

    return 0;
} 
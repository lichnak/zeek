#warning "This file will be removed in v4.1. The code here is unused by Zeek. Notify a Zeek team member if your plugins are using this code."

// See the file "COPYING" in the main distribution directory for copyright.

#pragma once

#include <pcap.h>

#include <sys/types.h> // for u_char

class PacketDumper {
public:
	explicit PacketDumper(pcap_dumper_t* pkt_dump);

	void DumpPacket(const struct pcap_pkthdr* hdr,
			const u_char* pkt, int len);

protected:
	pcap_dumper_t* pkt_dump;
	struct timeval last_timestamp;

	void SortTimeStamp(struct timeval* timestamp);
};

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <nids.h>
#define ERRMSG(fmt, args...) do{fprintf(stderr, "[%s %d]" fmt "\n", __FUNCTION__, __LINE__, ##args); fflush(stderr);}while(0)
#define INFO(fmt, args...) do{fprintf(stderr, "INFO[%s %d]" fmt "\n", __FUNCTION__, __LINE__, ##args);fflush(stderr);}while(0)
struct Args
{
	char input[1024];
}g_args;
#define int_ntoa(x) inet_ntoa(*((struct in_addr *)&x))

// struct tuple4 contains addresses and port numbers of the TCP connections
// the following auxiliary function produces a string looking like
// 10.0.0.1,1024,10.0.0.2,23
char* adres (struct tuple4* addr)
{
	static char buf[256];
	strcpy (buf, int_ntoa (addr->saddr));
	sprintf (buf + strlen (buf), ":%i to ", addr->source);
	strcat (buf, int_ntoa (addr->daddr));
	sprintf (buf + strlen (buf), ":%i", addr->dest);
	return buf;
}
void print_data(struct half_stream*  stream)
{
	if(stream->count)
	{
		INFO("count:%u data:%s\n", stream->count, stream->data);
	}
}
void udp_callback(struct tuple4 * addr, char * buf, int len, struct ip * iph)
{
	char ipinfo[1024];
	char from[20], to[20];
	strcpy(from, int_ntoa(iph->ip_src.s_addr));
	strcpy(to, int_ntoa(iph->ip_dst.s_addr));
	strcpy(ipinfo, adres(addr));
	INFO("UDP %s\n", ipinfo);
	INFO("from:%s to:%s len:%d buf content:%s\n", from, to, len, buf);
}
void tcp_callback (struct tcp_stream *a_tcp, void ** this_time_not_needed)
{
	char buf[1024];
	strcpy (buf, adres (&a_tcp->addr)); // we put conn params into buf
	switch(a_tcp->nids_state)
	{
		case NIDS_JUST_EST:
			a_tcp->client.collect++; // we want data received by a client
			a_tcp->server.collect++; // and by a server, too
			//a_tcp->server.collect_urg++;
			//a_tcp->client.collect_urg++;
			INFO("%s established\n", buf);
			break;
		case NIDS_RESET:
			INFO("%s reset\n", buf);
		case NIDS_CLOSE:
			INFO("%s closing\n", buf);
			INFO("client\n");
			print_data(&a_tcp->client);
			INFO("server\n");
			print_data(&a_tcp->server);
			INFO("%s closing(print over)\n", buf);
			break;
		case NIDS_DATA:
			if(a_tcp->client.count_new)
			{
				INFO("%s client data recv. count_new:%u string:%s\n", buf, a_tcp->client.count_new, a_tcp->client.data + a_tcp->client.count - a_tcp->client.count_new);
			}
			else if(a_tcp->server.count_new)
			{
				INFO("%s server data send. count_new:%u string:%s\n", buf, a_tcp->server.count_new, a_tcp->server.data +  a_tcp->server.count - a_tcp->server.count_new);
			}
			else
			{
				INFO("%s other data send/recv. continue get..\n", buf);
			}
			nids_discard(a_tcp, 0);
			break;
		default:
			INFO("%s not any enter:%d\n", buf, a_tcp->nids_state);
	}
}
int main(int argc, char** argv)
{
	snprintf(g_args.input, 1024, "-");
	int _opt;
	while((_opt = getopt(argc, argv, "hf:")) != -1) 
	{   
		INFO("enter getopt");
		switch(_opt)
		{   
			case 'f':
				INFO("tcpdump file:%s", optarg);
				strncpy(g_args.input, optarg, sizeof(g_args.input));
				break;
			case '?':
				INFO("undefine cmd:%c", _opt);
			case 'h':
				INFO("help:");
				fprintf(stderr, "%s [-f tcpdumpfile]\n"
						"\t-f,\t\tif tcpdumpfile is -, read data from standard input\n", argv[0]);
				exit(1);
				break;
		}   
	} 
	nids_params.filename = g_args.input;
	nids_params.sk_buff_size = 65535;
	if (!nids_init()) 
	{
		fprintf(stderr,"%s\n",nids_errbuf);
		exit(1);
	}
	INFO("Enter!");
	nids_register_tcp((void*)tcp_callback);
	nids_register_udp((void*)udp_callback);
	nids_run(); 
	return 0;
}
//gcc -o parse_dump parse_dump.c /usr/local/lib/libnids.a /usr/lib/libnet.a -lpcap 

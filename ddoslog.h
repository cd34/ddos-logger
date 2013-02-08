struct ddos_log {
    time_t time;
    //struct sockaddr_storage	source_ip;
    u_int		source_ip;
    unsigned short	source_port;
    unsigned char 	country[3];
    //struct sockaddr_storage	dest_ip;
    u_int		dest_ip;
    unsigned short	dest_port;
    unsigned int payload_len;
};

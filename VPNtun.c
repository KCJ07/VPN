// Kevin Johanson
// creates a virutal TUN interface to be able to capture packets
// only handles IP packets
//


#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <unistd.h>
#include <arpa/inet.h>

// creates a virtual network/ TUN device 
// returns TUN file descripter
int createTUN(char *devName) {
    int fd; 
    struct ifreq ifr; // struct used by icotl() system

    // opens TUN driver with reading and writing priveledges 
    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        perror("opening TUN file descriptor error");
        return -1;
    }

    // configure ifreq flags (not rlly sure if this is right)
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    strncpy(ifr.ifr_name, devName, IFNAMSIZ);

    // actually creates the TUN device with given parameters
    if (ioctl(fd, TUNSETIFF, (void *)&ifr) < 0) {
        perror("ioctl error");
        close(fd);  
        return -1;
    }

    strncpy(ifr.ifr_name, devName, IFNAMSIZ);

    return fd;
}

// configures all the settings for our TUN device
// IP address, Netmask, MTU
int TUNconfig( char *TUNname, char *ip, char *netmask, int MTU) {

    // make UDP socket for icotl()
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct ifreq ifr;          
    struct sockaddr_in addr;   // struct for IP and Netmask

    strncpy(ifr.ifr_name, TUNname, IFNAMSIZ);

    // set ip address to ipv4 config 
    addr.sin_family = AF_INET; 


    inet_pton(AF_INET, ip, &addr.sin_addr);

    // sets socket address in ifr
    ifr.ifr_addr = *(struct sockaddr *)&addr;

    // sets ip address of the TUN interface
    if (ioctl(sock, SIOCSIFADDR, &ifr) < 0) {
        perror("SIOCSIFADDR");
        close(sock);
        return -1;
    }
        
    // MTU config
    ifr.ifr_mtu = MTU;
    ioctl(sock, SIOCSIFMTU, &ifr); 

    // checks to see if flags are running?
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
        perror("SIOCGIFFLAGS");
        close(sock);
        return -1;
    }    

    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);

    // set new flags after modifying them
    if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0) {
        perror("SIOCSIFFLAGS");
        close(sock);
        return -1;
    }
    
    close(sock);
    return 0;
}

// Reads packets from our Virtual TUN interface 
int readTUN(int TUNfd, unsigned char *buffer, int size) {
    return(read(TUNfd, buffer, size));
 
}

// re-writes our network packet from our TUN interface into our network packet stack
int writeTUN(int TUNfd, unsigned char *buffer, int size) {
    return(write(TUNfd, buffer, size));
}
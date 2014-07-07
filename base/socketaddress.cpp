#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <net/if.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "base/socketaddress.h"

namespace wi {
extern void strncpyz(char *pszDst, const char *pszSrc, int cb);
}

namespace base {

SocketAddress::SocketAddress() {
    hostname_ = NULL;
    Clear();
}

SocketAddress::~SocketAddress() {
    Clear();
}

SocketAddress::SocketAddress(const char *hostname, int port, bool use_dns) {
    hostname_ = NULL;
    Clear();
    SetIP(hostname, use_dns);
    SetPort(port);
}

SocketAddress::SocketAddress(const char *hostport, bool use_dns) {
    hostname_ = NULL;
    Clear();

    // Parse hostname:port format
    const char *colon = NULL;
    for (const char *s = hostport; *s != 0; s++) {
        if (*s == ':') {
            colon = s;
        }
    }

    if (colon == NULL) {
        // Just hostname
        SetIP(hostport, use_dns);
    } else {
        // Hostname:port
        char hostnameT[256];
        wi::strncpyz(hostnameT, hostport, colon - hostport + 1);
        long port = strtol(colon + 1, NULL, 10);
        SetIP(hostnameT, use_dns);
        SetPort(port);
    }
}

SocketAddress::SocketAddress(dword ip, int port) {
    hostname_ = NULL;
    Clear();
    SetIP(ip);
    SetPort(port);
}

SocketAddress::SocketAddress(const SocketAddress& addr) {
    hostname_ = NULL;
    Clear();
    this->operator=(addr);
}

void SocketAddress::Clear() {
    delete[] hostname_;
    hostname_ = NULL;
    ip_ = 0;
    port_ = 0;
}

SocketAddress& SocketAddress::operator=(const SocketAddress& addr) {
    SetHostname(addr.hostname_);
    ip_ = addr.ip_;
    port_ = addr.port_;
    return *this;
}

void SocketAddress::SetHostname(const char *hostname) {
    if (hostname == hostname_)
        return;
    delete[] hostname_;
    hostname_ = NULL;
    if (hostname != NULL) {
        int cb = strlen(hostname) + 1;
        hostname_ = new char[cb];
        wi::strncpyz(hostname_, hostname, cb);
    }
}

bool SocketAddress::IsHostnameEmpty() const {
    return (hostname_ == NULL || hostname_[0] == 0);
}

void SocketAddress::SetIP(dword ip) {
    SetHostname(NULL);
    ip_ = ip;
}

bool SocketAddress::SetIP(const char *hostname, bool use_dns) {
    SetHostname(hostname);
    ip_ = 0;
    return Resolve(true, use_dns);
}

void SocketAddress::SetResolvedIP(dword ip) {
    ip_ = ip;
}

void SocketAddress::SetPort(int port) {
    port_ = port;
}

dword SocketAddress::ip() const {
    return ip_;
}

word SocketAddress::port() const {
    return port_;
}

void SocketAddress::IPAsString(char *psz, int cb) const {
    if (!IsHostnameEmpty()) {
        wi::strncpyz(psz, hostname_, cb);
        return;
    }
    IPToString(ip_, psz, cb);
}

void SocketAddress::PortAsString(char *psz, int cb) const {
    char szT[32];
    sprintf(szT, "%d", port_);
    wi::strncpyz(psz, szT, cb);
}

void SocketAddress::ToString(char *psz, int cb) const {
    IPAsString(psz, cb);
    int cbT = strlen(psz);
    cb -= cbT + 1;
    psz = &psz[cbT];
    if (cb <= 0)
        return;
    *psz++ = ':';
    cb--;
    PortAsString(psz, cb);
}   

const char *SocketAddress::ToString() const {
    static char s_szT[128];
    ToString(s_szT, sizeof(s_szT));
    return s_szT;
}

bool SocketAddress::IsAny() const {
    return (ip_ == 0);
}

bool SocketAddress::IsLocalIP() const {
    return (ip_ >> 24) == 127;
}

bool SocketAddress::IsPrivateIP() const {
    return ((ip_ >> 24) == 127) ||
                 ((ip_ >> 24) == 10) ||
                 ((ip_ >> 20) == ((172 << 4) | 1)) ||
                 ((ip_ >> 16) == ((192 << 8) | 168));
}

bool SocketAddress::IsUnresolved() const {
    return IsAny() && !IsHostnameEmpty();
}

bool SocketAddress::Resolve(bool force, bool use_dns) {
    if (IsHostnameEmpty()) {
        // nothing to resolve
    } else if (!force && !IsAny()) {
        // already resolved
    } else if (dword ip = StringToIP(hostname_, use_dns)) {
        ip_ = ip;
    } else {
        return false;
    }
    return true;
}

bool SocketAddress::operator ==(const SocketAddress& addr) const {
    return EqualIPs(addr) && EqualPorts(addr);
}

bool SocketAddress::EqualIPs(const SocketAddress& addr) const {
    
    if (ip_ != addr.ip_) {
        return false;
    }
    if (ip_ != 0) {
        return true;
    }
    if (IsHostnameEmpty() != addr.IsHostnameEmpty()) {
        return false;
    }
    if (hostname_ == NULL) {
        return true;
    }
    return strcmp(hostname_, addr.hostname_) == 0;
}

bool SocketAddress::EqualPorts(const SocketAddress& addr) const {
    return (port_ == addr.port_);
}

dword SocketAddress::Hash() const {
    dword h = 0;
    h ^= ip_;
    h ^= port_ | (port_ << 16);
    return h;
}

void SocketAddress::ToSockAddr(sockaddr_in* saddr) const {
    memset(saddr, 0, sizeof(*saddr));
    saddr->sin_family = AF_INET;
    saddr->sin_port = htons(port_);
    if (0 == ip_) {
        saddr->sin_addr.s_addr = INADDR_ANY;
    } else {
        saddr->sin_addr.s_addr = htonl(ip_);
    }
}

void SocketAddress::FromSockAddr(const sockaddr_in& saddr) {
    SetIP(ntohl(saddr.sin_addr.s_addr));
    SetPort(ntohs(saddr.sin_port));
}

void SocketAddress::IPToString(dword ip, char *psz, int cb) const {
    char szT[64];
    sprintf(szT, "%d.%d.%d.%d", (int)((ip >> 24) & 0xff),
            (int)((ip >> 16) & 0xff), (int)((ip >> 8) & 0xff),
            (int)((ip >> 0) & 0xff));
    wi::strncpyz(psz, szT, cb);
}

dword SocketAddress::StringToIP(const char *hostname, bool use_dns) {
    in_addr addr;
    if (inet_aton(hostname, &addr) != 0) {
        return ntohl(addr.s_addr);
    } else if (use_dns) {
        hostent *phost = gethostbyname(hostname);
        if (phost == NULL) {
            return 0;
        }
        return ntohl(*reinterpret_cast<dword *>(phost->h_addr_list[0]));
    }
    return 0;
}

void SocketAddress::GetHostname(char *psz, int cb) {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        wi::strncpyz(psz, hostname, cb);
    } else {
        if (cb > 0) {
            *psz = 0;
        }
    }
}

bool SocketAddress::GetNetworkInfo(int n, char *pszName, int cb, dword *pip) {
    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        return false;
    }
    
    struct ifconf ifc;
    ifc.ifc_len = 64 * sizeof(struct ifreq);
    ifc.ifc_buf = new char[ifc.ifc_len];
    
    if (ioctl(fd, SIOCGIFCONF, &ifc) < 0) {
        delete [] ifc.ifc_buf;
        close(fd);
        return false;
    }
    
    int nT = 0;
    struct ifreq* ptr = reinterpret_cast<struct ifreq*>(ifc.ifc_buf);
    struct ifreq* end = reinterpret_cast<struct ifreq*>(ifc.ifc_buf + ifc.ifc_len);
    while (ptr < end) {
        struct sockaddr_in* inaddr = reinterpret_cast<struct sockaddr_in*>(&ptr->ifr_ifru.ifru_addr);
        if (inaddr->sin_family == AF_INET) {
            if (nT == n) {
                wi::strncpyz(pszName, ptr->ifr_name, cb);
                *pip = ntohl(inaddr->sin_addr.s_addr);
                delete [] ifc.ifc_buf;
                close(fd);
                return true;
            }
            nT++;
        }
        
#ifdef _SIZEOF_ADDR_IFREQ
        ptr = reinterpret_cast<struct ifreq*>(
                reinterpret_cast<char*>(ptr) + _SIZEOF_ADDR_IFREQ(*ptr));
#else
        ptr++;
#endif
    }
    
    delete [] ifc.ifc_buf;
    close(fd);
    
    return false;
}

} // namespace base

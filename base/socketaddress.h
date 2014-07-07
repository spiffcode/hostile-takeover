#ifndef __SOCKETADDRESS_H__
#define __SOCKETADDRESS_H__

#include <netinet/in.h>
#include "inc/basictypes.h"

#undef SetPort

namespace base {

// Records an IP address and port, which are 32 and 16 bit integers,
// respectively, both in <b>host byte-order</b>.
class SocketAddress {
public:
    // Creates a missing / unknown address.
    SocketAddress();
    ~SocketAddress();

    // Creates the address with the given host and port. If use_dns is true,
    // the hostname will be immediately resolved to an IP (which may block for
    // several seconds if DNS is not available). Alternately, set use_dns to
    // false, and then call Resolve() to complete resolution later, or use
    // SetResolvedIP to set the IP explictly.
    SocketAddress(const char *hostname, int port, bool use_dns = true);

    // Creates address with host:port format string
    SocketAddress(const char *hostport, bool use_dns = true);

    // Creates the address with the given IP and port.
    SocketAddress(dword ip, int port);

    // Creates a copy of the given address.
    SocketAddress(const SocketAddress& addr);

    // Resets to missing / unknown address.
    void Clear();

    // Replaces our address with the given one.
    SocketAddress& operator =(const SocketAddress& addr);

    // Changes the IP of this address to the given one, and clears the hostname.
    void SetIP(dword ip);

    // Changes the hostname of this address to the given one.
    // Calls Resolve and returns the result.
    bool SetIP(const char *hostname, bool use_dns = true);

    // Sets the IP address while retaining the hostname. Useful for bypassing
    // DNS for a pre-resolved IP.
    void SetResolvedIP(dword ip);

    // Changes the port of this address to the given one.
    void SetPort(int port);

    // Returns the IP address.
    dword ip() const;

    // Returns the port part of this address.
    word port() const;

    // Returns the hostname
    void hostname(char *psz, int cb);

    // Returns the IP address in dotted form.
    void IPAsString(char *psz, int cb) const;

    // Returns the port as a string
    void PortAsString(char *psz, int cb) const;

    // Returns a display version of the IP/port.
    void ToString(char *psz, int cb) const;
    const char *ToString() const;

    // Determines whether this represents a missing / any address.
    bool IsAny() const;

    // Synomym for missing / any.
    bool IsNil() const { return IsAny(); }

    // Determines whether the IP address refers to the local host, i.e. within
    // the range 127.0.0.0/8.
    bool IsLocalIP() const;

    // Determines whether the IP address is in one of the private ranges:
    // 127.0.0.0/8 10.0.0.0/8 192.168.0.0/16 172.16.0.0/12.
    bool IsPrivateIP() const;

    // Determines whether the hostname has been resolved to an IP
    bool IsUnresolved() const;

    // Attempt to resolve a hostname to IP address.
    // Returns false if resolution is required but failed.
    // 'force' will cause re-resolution of hostname.
    // 
    bool Resolve(bool force = false, bool use_dns = true);

    // Determines whether this address is identical to the given one.
    bool operator ==(const SocketAddress& addr) const;

    inline bool operator !=(const SocketAddress& addr) const {
        return !this->operator ==(addr);
    }

    // Determines whether this address has the same IP as the one given.
    bool EqualIPs(const SocketAddress& addr) const;

    // Deteremines whether this address has the same port as the one given.
    bool EqualPorts(const SocketAddress& addr) const;

    // Hashes this address into a small number.
    dword Hash() const;

    // Convert to and from sockaddr_in
    void ToSockAddr(sockaddr_in* saddr) const;
    void FromSockAddr(const sockaddr_in& saddr);

    // Converts the IP address given in compact form into dotted form.
    void IPToString(dword ip, char *psz, int cb) const;

    // Converts the IP address given in dotted form into compact form.
    // Without 'use_dns', only dotted names (A.B.C.D) are resolved.
    static dword StringToIP(const char *psz, bool use_dns = true);

    // Get local machine's hostname
    static void GetHostname(char *psz, int cb);

    // Get information about the ip networks this machine is connected to
    static bool GetNetworkInfo(int n, char *pszName, int cb, dword *pip);

private:
    void SetHostname(const char *psz);
    bool IsHostnameEmpty() const;

    char *hostname_;
    dword ip_;
    word port_;
};

} // namespace base

#endif // __SOCKET_H__

/*---------------------------------------------------------------------------*
  Project:  NitroWiFi - include
  File:     so2soc.h

  Copyright 2005,2006 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef NITROWIFI_SOC2SO_H_
#define NITROWIFI_SOC2SO_H_

#ifdef __cplusplus

extern "C" {
#endif

//---------------------------------------------------------------------------
// defines
//---------------------------------------------------------------------------
#define SOC_VENDOR_NINTENDO  SO_VENDOR_NINTENDO
#define SOC_VERSION          SO_VERSION

#define SOC_PF_UNSPEC        SO_PF_UNSPEC
#define SOC_PF_INET          SO_PF_INET

#define SOC_AF_UNSPEC        SO_AF_UNSPEC
#define SOC_AF_INET          SO_AF_INET

#define SOC_INET_ADDRSTRLEN  SO_INET_ADDRSTRLEN

#define SOC_SOCK_STREAM      SO_SOCK_STREAM
#define SOC_SOCK_DGRAM       SO_SOCK_DGRAM

#define SOC_MSG_OOB          SO_MSG_OOB
#define SOC_MSG_PEEK         SO_MSG_PEEK
#define SOC_MSG_DONTWAIT     SO_MSG_DONTWAIT

// Socket option levels
#define SOC_SOL_SOCKET               SO_SOL_SOCKET
#define SOC_SOL_IP                   SO_SOL_IP
#define SOC_SOL_ICMP                 SO_SOL_ICMP
#define SOC_SOL_TCP                  SO_SOL_TCP
#define SOC_SOL_UDP                  SO_SOL_UDP

#define SOC_IPPROTO_ICMP             SO_IPPROTO_ICMP
#define SOC_IPPROTO_IGMP             SO_IPPROTO_IGMP
#define SOC_IPPROTO_TCP              SO_IPPROTO_TCP
#define SOC_IPPROTO_UDP              SO_IPPROTO_UDP

#define SOC_INADDR_ANY               SO_INADDR_ANY
#define SOC_INADDR_BROADCAST         SO_INADDR_BROADCAST
#define SOC_INADDR_LOOPBACK          SO_INADDR_LOOPBACK
#define SOC_INADDR_UNSPEC_GROUP      SO_INADDR_UNSPEC_GROUP
#define SOC_INADDR_ALLHOSTS_GROUP    SO_INADDR_ALLHOSTS_GROUP
#define SOC_INADDR_MAX_LOCAL_GROUP   SO_INADDR_MAX_LOCAL_GROUP

// SOC_GetSockOpt() / SOC_SetSockOpt()
#define SOC_IP_TOS               SO_IP_TOS
#define SOC_IP_TTL               SO_IP_TTL
#define SOC_IP_MULTICAST_LOOP    SO_IP_MULTICAST_LOOP
#define SOC_IP_MULTICAST_TTL     SO_IP_MULTICAST_TTL
#define SOC_IP_ADD_MEMBERSHIP    SO_IP_ADD_MEMBERSHIP
#define SOC_IP_DROP_MEMBERSHIP   SO_IP_DROP_MEMBERSHIP
#define SOC_SOC_REUSEADDR         SO_SOC_REUSEADDR
#define SOC_SOC_LINGER            SO_SOC_LINGER
#define SOC_SOC_OOBINLINE         SO_SOC_OOBINLINE
#define SOC_SOC_SNDBUF            SO_SOC_SNDBUF
#define SOC_SOC_RCVBUF            SO_SOC_RCVBUF
#define SOC_SOC_SNDLOWAT          SO_SOC_SNDLOWAT
#define SOC_SOC_RCVLOWAT          SO_SOC_RCVLOWAT
#define SOC_SOC_TYPE              SO_SOC_TYPE
#define SOC_TCP_NODELAY          SO_TCP_NODELAY
#define SOC_TCP_MAXSEG           SO_TCP_MAXSEG

// SOC_Shutdown()
#define SOC_SHUT_RD      SO_SHUT_RD
#define SOC_SHUT_WR      SO_SHUT_WR
#define SOC_SHUT_RDWR    SO_SHUT_RDWR

// SOC_Fcntl()
#define SOC_F_GETFL      SO_F_GETFL
#define SOC_F_SETFL      SO_F_SETFL
#define SOC_O_NONBLOCK   SO_O_NONBLOCK

// SOC_Poll()
#define SOC_POLLRDNORM       SO_POLLRDNORM
#define SOC_POLLRDBAND       SO_POLLRDBAND
#define SOC_POLLPRI          SO_POLLPRI
#define SOC_POLLWRNORM       SO_POLLWRNORM
#define SOC_POLLWRBAND       SO_POLLWRBAND
#define SOC_POLLERR          SO_POLLERR
#define SOC_POLLHUP          SO_POLLHUP
#define SOC_POLLNVAL         SO_POLLNVAL
#define SOC_POLLIN           SO_POLLIN
#define SOC_POLLOUT          SO_POLLOUT

#define SOC_INFTIM           SO_INFTIM

#define SOC_E2BIG            SO_E2BIG
#define SOC_EACCES           SO_EACCES
#define SOC_EADDRINUSE       SO_EADDRINUSE
#define SOC_EADDRNOTAVAIL    SO_EADDRNOTAVAIL
#define SOC_EAFNOSUPPORT     SO_EAFNOSUPPORT
#define SOC_EAGAIN           SO_EAGAIN
#define SOC_EALREADY         SO_EALREADY
#define SOC_EBADF            SO_EBADF
#define SOC_EBADMSG          SO_EBADMSG
#define SOC_EBUSY            SO_EBUSY
#define SOC_ECANCELED        SO_ECANCELED
#define SOC_ECHILD           SO_ECHILD
#define SOC_ECONNABORTED     SO_ECONNABORTED
#define SOC_ECONNREFUSED     SO_ECONNREFUSED
#define SOC_ECONNRESET       SO_ECONNRESET
#define SOC_EDEADLK          SO_EDEADLK
#define SOC_EDESTADDRREQ     SO_EDESTADDRREQ
#define SOC_EDOM             SO_EDOM
#define SOC_EDQUOT           SO_EDQUOT
#define SOC_EEXIST           SO_EEXIST
#define SOC_EFAULT           SO_EFAULT
#define SOC_EFBIG            SO_EFBIG
#define SOC_EHOSTUNREACH     SO_EHOSTUNREACH
#define SOC_EIDRM            SO_EIDRM
#define SOC_EILSEQ           SO_EILSEQ
#define SOC_EINPROGRESS      SO_EINPROGRESS
#define SOC_EINTR            SO_EINTR
#define SOC_EINVAL           SO_EINVAL
#define SOC_EIO              SO_EIO
#define SOC_EISCONN          SO_EISCONN
#define SOC_EISDIR           SO_EISDIR
#define SOC_ELOOP            SO_ELOOP
#define SOC_EMFILE           SO_EMFILE
#define SOC_EMLINK           SO_EMLINK
#define SOC_EMSGSIZE         SO_EMSGSIZE
#define SOC_EMULTIHOP        SO_EMULTIHOP
#define SOC_ENAMETOOLONG     SO_ENAMETOOLONG
#define SOC_ENETDOWN         SO_ENETDOWN
#define SOC_ENETRESET        SO_ENETRESET
#define SOC_ENETUNREACH      SO_ENETUNREACH
#define SOC_ENFILE           SO_ENFILE
#define SOC_ENOBUFS          SO_ENOBUFS
#define SOC_ENODATA          SO_ENODATA
#define SOC_ENODEV           SO_ENODEV
#define SOC_ENOENT           SO_ENOENT
#define SOC_ENOEXEC          SO_ENOEXEC
#define SOC_ENOLCK           SO_ENOLCK
#define SOC_ENOLINK          SO_ENOLINK
#define SOC_ENOMEM           SO_ENOMEM
#define SOC_ENOMSG           SO_ENOMSG
#define SOC_ENOPROTOOPT      SO_ENOPROTOOPT
#define SOC_ENOSPC           SO_ENOSPC
#define SOC_ENOSR            SO_ENOSR
#define SOC_ENOSTR           SO_ENOSTR
#define SOC_ENOSYS           SO_ENOSYS
#define SOC_ENOTCONN         SO_ENOTCONN
#define SOC_ENOTDIR          SO_ENOTDIR
#define SOC_ENOTEMPTY        SO_ENOTEMPTY
#define SOC_ENOTSOCK         SO_ENOTSOCK
#define SOC_ENOTSUP          SO_ENOTSUP
#define SOC_ENOTTY           SO_ENOTTY
#define SOC_ENXIO            SO_ENXIO
#define SOC_EOPNOTSUPP       SO_EOPNOTSUPP
#define SOC_EOVERFLOW        SO_EOVERFLOW
#define SOC_EPERM            SO_EPERM
#define SOC_EPIPE            SO_EPIPE
#define SOC_EPROTO           SO_EPROTO
#define SOC_EPROTONOSUPPORT  SO_EPROTONOSUPPORT
#define SOC_EPROTOTYPE       SO_EPROTOTYPE
#define SOC_ERANGE           SO_ERANGE
#define SOC_EROFS            SO_EROFS
#define SOC_ESPIPE           SO_ESPIPE
#define SOC_ESRCH            SO_ESRCH
#define SOC_ESTALE           SO_ESTALE
#define SOC_ETIME            SO_ETIME
#define SOC_ETIMEDOUT        SO_ETIMEDOUT
#define SOC_ETXTBSY          SO_ETXTBSY
#define SOC_EWOULDBLOCK      SO_EWOULDBLOCK
#define SOC_EXDEV            SO_EXDEV

#define SOC_MAXDNAME         SO_MAXDNAME

// Name for SOConfig.alloc() and SOConfig.free()
#define SOC_MEM_TCP_INFO                SO_MEM_TCP_INFO         
#define SOC_MEM_TCP_SENDBUF             SO_MEM_TCP_SENDBUF      
#define SOC_MEM_TCP_RECVBUF             SO_MEM_TCP_RECVBUF      
#define SOC_MEM_UDP_INFO                SO_MEM_UDP_INFO         
#define SOC_MEM_UDP_SENDBUF             SO_MEM_UDP_SENDBUF      
#define SOC_MEM_UDP_RECVBUF             SO_MEM_UDP_RECVBUF      
#define SOC_MEM_TIMEWAITBUF             SO_MEM_TIMEWAITBUF      
#define SOC_MEM_REASSEMBLYBUF           SO_MEM_REASSEMBLYBUF    
#define SOC_MEM_SOCKADDR                SO_MEM_SOCKADDR         
#define SOC_MEM_CANONNAME               SO_MEM_CANONNAME        
#define SOC_MEM_ADDRINFO                SO_MEM_ADDRINFO         

// Flags for SOConfig{}.flag
#define SOC_FLAG_DHCP                   SO_FLAG_DHCP        
#define SOC_FLAG_PPPoE                  SO_FLAG_PPPoE       
#define SOC_FLAG_ZEROCONF               SO_FLAG_ZEROCONF    

//
#define SOC_IP_ERR_NONE                 IP_ERR_NONE                 
#define SOC_IP_ERR_BUSY                 IP_ERR_BUSY                 
#define SOC_IP_ERR_UNREACHABLE          IP_ERR_UNREACHABLE          
#define SOC_IP_ERR_RESET                IP_ERR_RESET                
#define SOC_IP_ERR_NOT_EXIST            IP_ERR_NOT_EXIST            
#define SOC_IP_ERR_EXIST                IP_ERR_EXIST                
#define SOC_IP_ERR_SOCKET_UNSPECIFIED   IP_ERR_SOCKET_UNSPECIFIED   
#define SOC_IP_ERR_INS_RESOURCES        IP_ERR_INS_RESOURCES        
#define SOC_IP_ERR_CLOSING              IP_ERR_CLOSING              
#define SOC_IP_ERR_AGAIN                IP_ERR_AGAIN                
#define SOC_IP_ERR_TIMEOUT              IP_ERR_TIMEOUT              
#define SOC_IP_ERR_REFUSED              IP_ERR_REFUSED              
#define SOC_IP_ERR_INVALID              IP_ERR_INVALID              
#define SOC_IP_ERR_INV_SOCKET           IP_ERR_INV_SOCKET           
#define SOC_IP_ERR_INV_OPTION           IP_ERR_INV_OPTION           

//
#define SOC_IP_ERR_CANCELED             IP_ERR_CANCELED
#define SOC_IP_ERR_DATASIZE             IP_ERR_DATASIZE
#define SOC_IP_ERR_SOURCE_QUENCH        IP_ERR_SOURCE_QUENCH
#define SOC_IP_ERR_NETDOWN              IP_ERR_NETDOWN

// IPGetConfigError()
#define SOC_IP_ERR_DHCP_TIMEOUT         IP_ERR_DHCP_TIMEOUT         
#define SOC_IP_ERR_DHCP_EXPIRED         IP_ERR_DHCP_EXPIRED         
#define SOC_IP_ERR_DHCP_NAK             IP_ERR_DHCP_NAK             
#define SOC_IP_ERR_PPPoE_TIMEOUT        IP_ERR_PPPoE_TIMEOUT        
#define SOC_IP_ERR_PPPoE_SERVICE_NAME   IP_ERR_PPPoE_SERVICE_NAME   
#define SOC_IP_ERR_PPPoE_AC_SYSTEM      IP_ERR_PPPoE_AC_SYSTEM      
#define SOC_IP_ERR_PPPoE_GENERIC        IP_ERR_PPPoE_GENERIC        
#define SOC_IP_ERR_LCP                  IP_ERR_LCP                  
#define SOC_IP_ERR_AUTH                 IP_ERR_AUTH                 
#define SOC_IP_ERR_IPCP                 IP_ERR_IPCP                 
#define SOC_IP_ERR_PPP_TERMINATED       IP_ERR_PPP_TERMINATED       
#define SOC_IP_ERR_ADDR_COLLISION       IP_ERR_ADDR_COLLISION       
#define SOC_IP_ERR_LINK_DOWN            IP_ERR_LINK_DOWN            

// DNS Error
#define SOC_IP_ERR_DNS_MAX      IP_ERR_DNS_MAX
#define SOC_IP_ERR_DNS_MIN      IP_ERR_DNS_MIN

#define SOC_DNS_ERROR_MASK      DNS_ERROR_MASK
#define SOC_DNS_ERROR_NONE      DNS_ERROR_NONE
#define SOC_DNS_ERROR_FORMAT    DNS_ERROR_FORMAT
#define SOC_DNS_ERROR_SERVER    DNS_ERROR_SERVER
#define SOC_DNS_ERROR_NAME      DNS_ERROR_NAME
#define SOC_DNS_ERROR_NOTIMP    DNS_ERROR_NOTIMP
#define SOC_DNS_ERROR_REFUSED   DNS_ERROR_REFUSED

#define SOC_IP_ERR_DNS_FORMAT   IP_ERR_DNS_FORMAT
#define SOC_IP_ERR_DNS_SERVER   IP_ERR_DNS_SERVER
#define SOC_IP_ERR_DNS_NAME     IP_ERR_DNS_NAME
#define SOC_IP_ERR_DNS_NOTIMP   IP_ERR_DNS_NOTIMP
#define SOC_IP_ERR_DNS_REFUSED  IP_ERR_DNS_REFUSED

//---------------------------------------------------------------------------
// typedefs
//---------------------------------------------------------------------------
#define SOCSockAddr         SOSockAddr
#define SOCInAddr           SOInAddr
#define SOCSockAddrIn       SOSockAddrIn
#define SOCLinger           SOLinger
#define SOCIpMreq           SOIpMreq
#define SOCPollFD           SOPollFD
#define SOCHostEnt          SOHostEnt
#define SOCResolver         SOResolver
#define SOCConfig           SOConfig
#define SOCIPSocket         IPSocket
#define SOC_ALEN            IP_ALEN
#define SOCTime             IPTime
#define SOC_MAC_ALEN        MAC_ALEN

//---------------------------------------------------------------------------
// consts
//---------------------------------------------------------------------------
#define SOCAddrAny      IPAddrAny       // 0.  0.  0.  0
#define SOCAddrLoopback IPAddrLoopback  // 127.0.  0.  1
#define SOCAddrLimited  IPAddrLimited   // 255.255.255.255

//---------------------------------------------------------------------------
// functions
//---------------------------------------------------------------------------
#define SOC_NtoHl               SO_NtoHl
#define SOC_NtoHs               SO_NtoHs
#define SOC_HtoNl               SO_HtoNl
#define SOC_HtoNs               SO_HtoNs

#define SOC_InetAtoN            SO_InetAtoN
#define SOC_InetNtoA            SO_InetNtoA

#define SOC_InetPtoN            SO_InetPtoN
#define SOC_InetNtoP            SO_InetNtoP

#define SOC_Socket              SO_Socket
#define SOC_Close               SO_Close
#define SOC_Shutdown            SO_Shutdown

#define SOC_Bind                SO_Bind
#define SOC_Connect             SO_Connect
#define SOC_GetPeerName         SO_GetPeerName
#define SOC_GetSockName         SO_GetSockName

#define SOC_Read                SO_Read
#define SOC_Recv                SO_Recv
#define SOC_RecvFrom            SO_RecvFrom

#define SOC_Write               SO_Write
#define SOC_Send                SO_Send
#define SOC_SendTo              SO_SendTo

#define SOC_Accept              SO_Accept
#define SOC_Listen              SO_Listen

#define SOC_SockAtMark          SO_SockAtMark

#define SOC_GetSockOpt          SO_GetSockOpt
#define SOC_SetSockOpt          SO_SetSockOpt

#define SOC_Fcntl               SO_Fcntl

#define SOC_Poll                SO_Poll

#define SOC_GetHostByAddr       SO_GetHostByAddr
#define SOC_GetHostByName       SO_GetHostByName
#define SOC_FreeAddrInfo        SO_FreeAddrInfo
#define SOC_GetNameInfo         SO_GetNameInfo

#define SOC_GetAddrInfo         SO_GetAddrInfo
#define SOC_GetAddrInfoAsync    SO_GetAddrInfoAsync

#define SOC_GetHostID           SO_GetHostID

#define SOC_Init                SO_Init
#define SOC_Reset               SO_Reset
#define SOC_Startup             SO_Startup
#define SOC_Cleanup             SO_Cleanup

#define SOC_SetResolver         SO_SetResolver
#define SOC_GetResolver         SO_GetResolver

#define SOCInterface            IPInterface
#define SOC_GetMacAddr          IP_GetMacAddr
#define SOC_GetAddr             IP_GetAddr
#define SOC_GetAlias            IP_GetAlias
#define SOC_GetGateway          IP_GetGateway
#define SOC_GetNetmask          IP_GetNetmask
#define SOC_GetBroadcastAddr    IP_GetBroadcastAddr
#define SOC_GetLinkState        IP_GetLinkState
#define SOC_GetConfigError      IP_GetConfigError
#define SOC_ClearConfigError    IP_ClearConfigError

#define SOC_DHCPGetStatus           DHCP_GetStatus
#define SOCDHCPInfo;                DHCPInfo
#define SOC_DHCP_STATE_INIT         DHCP_STATE_INIT
#define SOC_DHCP_STATE_SELECTING    DHCP_STATE_SELECTING
#define SOC_DHCP_STATE_REQUESTING   DHCP_STATE_REQUESTING
#define SOC_DHCP_STATE_BOUND        DHCP_STATE_BOUND
#define SOC_DHCP_STATE_RENEWING     DHCP_STATE_RENEWING
#define SOC_DHCP_STATE_REBINDING    DHCP_STATE_REBINDING
#define SOC_DHCP_STATE_INIT_REBOOT  DHCP_STATE_INIT_REBOOT
#define SOC_DHCP_STATE_REBOOTING    DHCP_STATE_REBOOTING

#ifdef __cplusplus

}
#endif

#endif // NITROWIFI_SOC2SO_H_

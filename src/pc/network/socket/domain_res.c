#include <stdio.h>
#include "socket.h"
#include "pc/configfile.h"
#include "pc/debuglog.h"
#include "pc/djui/djui.h"
#ifdef WINSOCK
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#endif


void domain_resolution(void) {
  struct in_addr addr;
  struct hostent *remoteHost;
  char* domainname = "";
  if (configJoinIp == NULL) {
	  return;
  }
  remoteHost = gethostbyname(configJoinIp);
  if (remoteHost->h_addrtype == AF_INET) {

    while (remoteHost->h_addr_list[i] != 0) {
          addr.s_addr = *(u_long *) remoteHost->h_addr_list[i++];
          domainname = inet_ntoa(addr);
          snprintf(configJoinIp, MAX_CONFIG_STRING, "%s", domainname);
    }
  }
}

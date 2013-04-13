#ifndef _NET_H_
#define _NET_H_

#include <string>
#include <netinet/in.h> /* For struct sockaddr_in */

namespace Net { 
  const int ERR = -1; 
  typedef void( *NewTcpClientFunc )(int, struct sockaddr_in*, void*);  
  typedef void( *NewUdpClientFunc )(int, void*);  
  typedef void( *ServiceFunc)(int, void*); 
  typedef void( *InitFunc)(int, void*);

  /////////////////////////////////////////////////////////
  // Establish a serv4er clients can connect to,
  // NewClientFunc gets call when clients connect
  void runTcpServer( int port, Net::NewTcpClientFunc, void* = 0 );
  void runUdpServer( int port, Net::NewUdpClientFunc, void* = 0 );

  /////////////////////////////////////////////////////////
  // Connect to a server on 'host'
  // - InitFunc called just after connect [for handshaking]
  // - ServiceFunc called when a read is possible
  void runTcpClient(std::string host, int port,
		    Net::InitFunc,Net::ServiceFunc,void* =0);
  void runUdpClient(std::string host, int port,
		    Net::InitFunc,Net::ServiceFunc,void* =0);

  /////////////////////////////////////////////////////////
  // stop all server/client threads
  bool isDone(void);
  void setDone(void);
  void waitForThreads(void);
  void allowBindToFloat(void);

  /////////////////////////////////////////////////////////
  // manually create a [default=tcp] socket
  // UDP: type=SOCK_DGRAM
  int create(int domain=AF_INET, int type=SOCK_STREAM, int protocol=0);

  /////////////////////////////////////////////////////////
  // server: manually set up a socket 
  //       : setsockopt(SO_REUSEADDR),bind,getsockname
  //    TCP: follow this with a listen 
  //    UDP: y'er good to go
  int initSock( int sock );

  void toAddr(in_addr_t, unsigned char[4]);

};

#endif

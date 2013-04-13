#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>  // for socket(), AF_INET
#include <netinet/in.h>  // for INADDR_ANY
#include <netinet/tcp.h> // for TCP_NODELAY
#include <netdb.h>       // for gethostbyname()       
#include <sys/select.h>  // POSIX 1003.1-2001 standard
#include <unistd.h>      // for getpagesize
#include <pthread.h>
#include <assert.h>
// #include "Port.h"
#include "Net.h"

//#define DEBUG 1
#include "debug.h"

using namespace std;

namespace Net {

  class TcpServerCBInfo {
  public:
    TcpServerCBInfo(int p, Net::NewTcpClientFunc f, void* u) : 
      port(p),func(f),user(u)
    {
    }
    int port;
    Net::NewTcpClientFunc func;
    void* user;
  };

  class UdpServerCBInfo {
  public:
    UdpServerCBInfo(int p, Net::NewUdpClientFunc f, void* u) : 
      port(p),func(f),user(u)
    {
    }
    int port;
    Net::NewUdpClientFunc func;
    void* user;
  };

  class ClientCBInfo {
  public:
    ClientCBInfo(string h, int p,
		 Net::InitFunc init, 
		 Net::ServiceFunc srvc, 
		 void* u) :
      host(h),initFunc(init),serviceFunc(srvc),port(p),user(u)
    {
    }
    string host;
    Net::InitFunc initFunc;
    Net::ServiceFunc serviceFunc;
    int port;
    void* user;
  };

  vector<int> clients;
  vector<pthread_t> threads;

  pthread_mutex_t doneLock = PTHREAD_MUTEX_INITIALIZER;
  bool doneFlag=false;
  bool bindFailDie=true; // set false to allow floating bind

  ///// Private Methods /////
  bool isDone(void);
  void *tcpServer(void * arg);
  void *tcpClient(void * user);

  void *udpServer(void * arg);
  void *udpClient(void * user);

  //////////////////////////////////////////////////////////////////////////
  ///////////////////////////// Public Methods /////////////////////////////
  //////////////////////////////////////////////////////////////////////////
 
  void allowBindToFloat(void)
  {
    bindFailDie=false;
  }

  void runTcpServer(int port, Net::NewTcpClientFunc func, void* user)
  {
    FANCYMESG("runTcpServer");
    pthread_t thread;
    TcpServerCBInfo* cbinfo = new TcpServerCBInfo(port,func,user);
    int res = pthread_create(&thread,0,Net::tcpServer,cbinfo);
    if( res != 0 ){
      cerr << "pthread_create() failed ["<<res<<"]!" << endl;
      return;
    }
    threads.push_back(thread);
  }

  void runTcpClient(string host, int port,
		    Net::InitFunc init, Net::ServiceFunc srvc, void* user)
  {
    FANCYMESG("runTcpClient");
    pthread_t thread;
    ClientCBInfo* userCallbackInfo = 
      new ClientCBInfo(host,port,init,srvc,user);
    int res = pthread_create(&thread,0,Net::tcpClient,userCallbackInfo);
    if( res != 0 ){
      cerr << "pthread_create() failed ["<<res<<"]!" << endl;
      return;
    }
    threads.push_back(thread);
  }

  void runUdpServer(int port, Net::NewUdpClientFunc func, void* user)
  {
    FANCYMESG("runUdpServer");
    pthread_t thread;
    UdpServerCBInfo* cbinfo = new UdpServerCBInfo(port,func,user);
    int res = pthread_create(&thread,0,Net::udpServer,cbinfo);
    if( res != 0 ){
      cerr << "pthread_create() failed ["<<res<<"]!" << endl;
      return;
    }
    threads.push_back(thread);
  }

  void runUdpClient( string host, int port,
		     Net::InitFunc init, Net::ServiceFunc srvc, void* user)
  {
    FANCYMESG("runUdpClient");
    pthread_t thread;
    ClientCBInfo* userCallbackInfo = 
      new ClientCBInfo(host,port,init,srvc,user);
    int res = pthread_create(&thread,0,Net::udpClient,userCallbackInfo);
    if( res != 0 ){
      cerr << "pthread_create() failed ["<<res<<"]!" << endl;
      return;
    }
    threads.push_back(thread);
  }

  void setDone(void)
  {
    FANCYMESG("setDone");
    pthread_mutex_lock(&doneLock);
    doneFlag = true;
    pthread_mutex_unlock(&doneLock);
    waitForThreads();
  }
  
  void waitForThreads(void)
  {
    vector<pthread_t>::iterator tIter = threads.begin();
    for( ; tIter != threads.end() ; ++tIter ){
      void* threadRes;
      pthread_join(*tIter,&threadRes);
    }
  }

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////// Private Methods /////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  int create(int domain, int type, int protocol){
    return socket(domain, type, protocol);
  }

  int initSock( int sock, int port )
  {
    FANCYMESG("Net::Server: initSock");
    int val = 1;
    socklen_t length;
    struct sockaddr_in name;
    bool done=false;
    int curport=port;

    while( ! done ){
      name.sin_family       = AF_INET;    /* internet domain */
      name.sin_addr.s_addr  = INADDR_ANY; /* wildcard - allow any */
      name.sin_port         = htons(curport); /* user-specified */
      length                = sizeof(name);
      
      /* allow the socket to be rebound next time! */
      if( setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(int)) == ERR){
	perror("setsockopt");
	return(ERR);
      }
      if( setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,&val,sizeof(int)) == ERR ){
	perror("setsockopt");
	return(ERR);
      }
      socklen_t rres;
      int ssize = getpagesize()*16;
      
      if( setsockopt(sock,SOL_SOCKET,SO_SNDBUF,&ssize,sizeof(int))){
	perror("setsockopt(SO_SNDBUF)");
	return(ERR); 
      }
      if( setsockopt(sock,SOL_SOCKET,SO_RCVBUF,&ssize,sizeof(int))){
	perror("setsockopt(SO_RCVBUF)");
	return(ERR);      
      }
      
      if(getsockopt(sock,SOL_SOCKET, SO_SNDBUF, &ssize,&rres)){
	perror("getsockopt");
	return(ERR);      
      }
      //_MESGVAR("SO_SNDBUF",ssize);
      if(getsockopt(sock,SOL_SOCKET, SO_RCVBUF, &ssize,&rres)){
	perror("getsockopt");
	return(ERR);      
      }
      //_MESGVAR("SO_RCVBUF",ssize);
      
      /* let bind assign us a port number */      
      if( bind(sock, (const struct sockaddr*)&name, length) == Net::ERR)  
      {
	if( bindFailDie ){
	  perror("bind");
	  return(ERR);
	} else {
	  MESGVAR("Net::Server: initSock: failed to bind to",curport);
	  perror("bind");
	  ++curport;
	}
      }
      else /* get the portnumber... */
      {
	done=true;
	if (getsockname(sock, (struct sockaddr*)&name, &length) == Net::ERR)  
	{
	  perror("getsockname");
	  return(ERR);
	}
	cout<<"Net::Server: listening on ["<<ntohs(name.sin_port)<<"]\n";
      }
    }

    return(name.sin_port);
  }

  bool isDone(void)
  {
    bool res=false;
    pthread_mutex_lock(&doneLock);
    res = doneFlag;
    pthread_mutex_unlock(&doneLock);
    return res;
  }
  
  void *
  tcpServer(void * user)
  {
    TcpServerCBInfo* cbInfo = static_cast<TcpServerCBInfo*>(user);
    
    FANCYMESG("tcpServer starting");

    fd_set readfds;
    int selected;
    struct timeval timeout = {2,0};

    int mainSocket = Net::create();
    assert( mainSocket>0 );
    Net::initSock(mainSocket,cbInfo->port);

    /* establish "... a willingness to accept connections ..." */
    if (::listen(mainSocket, 50) == ERR)  {
      perror("listen");
      return(0);
    }

    while( ! isDone() ) {

      // check main socket for connection requests      
      memset( &readfds, 0, sizeof(fd_set) );    
      timeout.tv_sec=2;
      timeout.tv_usec=0;
      
      FD_SET( mainSocket, &readfds );
      //MESG("calling select");
      selected = select(FD_SETSIZE,&readfds,0,0,&timeout);
      //VAR(selected);
      if( selected ){
	MESG("request for connection");
	struct sockaddr_in cName;
	socklen_t cLen = sizeof(struct sockaddr_in);
	int client = accept(mainSocket, (struct sockaddr*)&cName, &cLen);
	if( client==Net::ERR ){
	  perror("accept");
	  setDone();
	}

	MESG("connection accepted");

	// call user func
	MESG("calling user func");
	cbInfo->func(client,&cName,cbInfo->user);
	continue;
      }
      
    } // while ! done

    ::close(mainSocket);
    FANCYMESG("tcpServer exiting");
    return 0;
  } // tcpServer
  
  void *
  udpServer(void * user)
  {
    UdpServerCBInfo* cbInfo = static_cast<UdpServerCBInfo*>(user);
    
    FANCYMESG("udpServer starting");

    fd_set readfds;
    int selected;
    struct timeval timeout = {2,0};

    int mainSocket = Net::create(AF_INET,SOCK_DGRAM);
    assert( mainSocket>0 );
    Net::initSock(mainSocket,cbInfo->port);

    while( ! isDone() ) {
      
      // check main socket for incoming datagrams
      memset( &readfds, 0, sizeof(fd_set) );
      FD_SET( mainSocket, &readfds );

      // reset timeout
      timeout.tv_sec=2;
      timeout.tv_usec=0;

      // select w/timeout
      selected = select(FD_SETSIZE,&readfds,0,0,&timeout);
      if( selected )
	cbInfo->func(mainSocket,cbInfo->user);
    }

    ::close(mainSocket);
    FANCYMESG("udpServer exiting");
    return 0;
  }

  void *
  tcpClient(void * user)
  {
    FANCYMESG("tcpClient starting");
    ClientCBInfo* cbinfo = static_cast<ClientCBInfo*>(user);
    fd_set readfds;
    int selected;
    struct timeval timeout = {2,0};
    struct sockaddr_in name;

    int sock = Net::create();
    assert( sock>0 );
    
    struct hostent *hp = gethostbyname(cbinfo->host.c_str());
    if( hp==0 ){
      perror("gethostbyname");
      cerr << "Net::TcpClient: Unknown host: " << cbinfo->host << endl;
      ::close(sock);
      FANCYMESG("client exiting");
      return 0;
    }

    name.sin_family = AF_INET;
    memcpy(&(name.sin_addr.s_addr), hp->h_addr, hp->h_length);
    name.sin_port = htons(cbinfo->port);
    
    if (connect(sock,(const sockaddr*)&name,sizeof(name)) == Net::ERR){
      perror("connect");
      cerr << "Net::TcpClient: connect failed" << endl;
      ::close(sock);
      FANCYMESG("client exiting");
      return 0;
    }
    
    cbinfo->initFunc(sock,cbinfo->user);

    while(! isDone() ){
      memset( &readfds, 0, sizeof(fd_set) );
      FD_SET(sock, &readfds);
      timeout.tv_sec=2;
      timeout.tv_usec=0;
      selected = select(FD_SETSIZE,&readfds,0,0,&timeout);
      if( selected )			
	cbinfo->serviceFunc(sock,cbinfo->user);	
    }

    ::close(sock);
    FANCYMESG("tcpClient exiting");
    return 0;

  } // tcpClient

  void *
  udpClient(void * user)
  {
    _FANCYMESG("udpClient starting");
    ClientCBInfo* cbinfo = static_cast<ClientCBInfo*>(user);
    fd_set readfds;
    int selected;
    struct timeval timeout = {2,0};

    int sock = Net::create(AF_INET,SOCK_DGRAM);
    assert( sock>0 );

    cbinfo->initFunc(sock,cbinfo->user);

    while(! isDone() ){
      memset( &readfds, 0, sizeof(fd_set) );
      FD_SET(sock, &readfds);
      timeout.tv_sec=2;
      timeout.tv_usec=0;
      selected = select(FD_SETSIZE,&readfds,0,0,&timeout);
      if( selected )			
	cbinfo->serviceFunc(sock,cbinfo->user);	
    }

    ::close(sock);
    _FANCYMESG("udpClient exiting");
    return 0;
  }
  
  void toAddr(in_addr_t src, unsigned char dst[4])
  {
    dst[3] = (src & 0xff000000U) >> 24;
    dst[2] = (src & 0x00ff0000U) >> 16;
    dst[1] = (src & 0x0000ff00U) >> 8;
    dst[0] =  src & 0x000000ffU;
  }

} // Net

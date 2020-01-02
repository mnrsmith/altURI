
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32")

class UDPSocket
{
        private:
                int SOCKET;
                int PORT;
        public:
                ~UDPSocket()
                {
                        closesocket(SOCKET);
                }
                UDPSocket()
                {
                        SOCKET=0;
                }
                bool Start(int PORT_)
                {
                        PORT=PORT_;
                       
                        SOCKET=socket(AF_INET,SOCK_DGRAM,0);
                       
                        sockaddr_in service;
                        service.sin_family = AF_INET;
                        service.sin_addr.s_addr = INADDR_ANY;
                        service.sin_port = htons(PORT);
                       
                        return bind(SOCKET,(const sockaddr*)&service,sizeof(service))>=0;
                }
                UDPSocket(int PORT_)
                {
                        Start(PORT_);
                }
                bool Write(unsigned int IP,int PORT_,char* DATA,int SIZE)
                {
                        if (SOCKET==0)
                                return false;
                               
                        sockaddr_in service;
                        service.sin_family = AF_INET;
                        service.sin_addr.s_addr = IP;
                        service.sin_port = htons(PORT_);
                        return sendto(SOCKET,DATA,SIZE,0,(const sockaddr*)&service,sizeof(service))==SIZE;
                }
                bool Write(const char* IP,int PORT_,char* DATA,int SIZE)
                {
                        return Write(inet_addr(IP),PORT_,DATA,SIZE);
                }
                bool Read(char* DATA,int SIZE,unsigned int *IP,int *PORT_)
                {
                        if (SOCKET==0)
                                return false;
                       
                        sockaddr_in service;
                        service.sin_family = AF_INET;
                        service.sin_addr.s_addr = INADDR_ANY;
                        service.sin_port = 0;
                       
                        socklen_t service_len=sizeof(service);
                       
                        bool res=recvfrom(SOCKET,DATA,SIZE,0,(sockaddr*)&service,&service_len)>0;
                       
                        if (res)
                        {
                                *PORT_=ntohs(service.sin_port);
                                *IP=service.sin_addr.s_addr;
                        }
                       
                        return res;
                }
};
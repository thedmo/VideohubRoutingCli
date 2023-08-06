#ifndef TELNETCLIENT
#define TELNETCLIENT

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "WS2_32.lib")

#include <ws2tcpip.h>
#include <string>
#include <vector>

#include <ErrorTrace.hpp>

class TelnetClient {
public:
    TelnetClient() = delete;
    TelnetClient(std::string ip, int port, std::string &, int &result);
    ~TelnetClient();

    int SendMsgToServer(std::string msg);

    int ReceiveMsgFromServer(std::string &response);

    int ChangeIpAddress(std::string newAddress, std::string &init_response);

    std::string GetLastDataDump();
    std::string GetIp();

    const static int TELNET_OK = 0;

private:

    std::string m_ip_address;
    int m_port;

    char m_buf[4096];
    SOCKET m_sock;

    std::string m_last_data_dump;

    int OpenConnection();
    int CloseConnection();
};

#endif // TELNETCLIENT

#ifndef TELNETCLIENT
#define TELNETCLIENT

#include <ws2tcpip.h>
#include <string>
#include <vector>



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

    std::vector<std::string> GetErrorMessages();
    const static int TELNET_OK = 0;

private:
    std::vector<std::string> m_err_msgs;
    std::string m_ip_address;
    int m_port;

    char m_buf[4096];
    SOCKET m_sock;

    std::string m_last_data_dump;

    int OpenConnection();
    int CloseConnection();
};

#endif // TELNETCLIENT

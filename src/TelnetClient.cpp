#include "TelnetClient.hpp"

TelnetClient::TelnetClient(std::string ip, int port, std::string &init_response, int &result)
    : m_ip_address(ip), m_port(port)
{

    result = OpenConnection();
    if (result != TELNET_OK)
    {
        m_err_msgs.push_back("could not open connection.");
        return;
    }
}

TelnetClient::~TelnetClient() { CloseConnection(); }

int TelnetClient::SendMsgToServer(std::string msg) {

    int sendResult = send(m_sock, msg.c_str(), msg.size(), 0);

    ReceiveMsgFromServer(m_last_data_dump);

    return 0;
}

int TelnetClient::ReceiveMsgFromServer(std::string &response)
{
    Sleep(100);
    ZeroMemory(m_buf, 4096);
    int bytesReceived = recv(m_sock, m_buf, 4096, 0);
    std::string s_result = std::string(m_buf, 0, bytesReceived);

    if (s_result.size() != std::string::npos)
    {
        response = s_result;
    }
    else
    {
        response.clear();
    }

    return 0;
}

int TelnetClient::ChangeIpAddress(std::string newAddress, std::string &init_response)
{

    if (newAddress.empty())
    {
        m_err_msgs.push_back("ip adress empty..");
        return 1;
    }

    sockaddr_in newIp;
    newIp.sin_family = AF_INET;
    int _result = inet_pton(AF_INET, newAddress.c_str(), &newIp.sin_addr);

    if (_result != 1)
    {
        m_err_msgs.push_back("not a valid IPv4 Adress");
        return 1;
    }

    m_ip_address = newAddress;

    _result = OpenConnection();
    if (_result != TELNET_OK)
    {
        m_err_msgs.push_back("could not open connection.");
        return 1;
    }

    _result = ReceiveMsgFromServer(init_response);
    if (_result != TELNET_OK)
    {
        m_err_msgs.push_back("could not receive message from server");
        return 1;
    }

    return TELNET_OK;
}

std::string TelnetClient::GetIp() { return m_ip_address; }

std::string TelnetClient::GetLastDataDump() { return m_last_data_dump; }

int TelnetClient::OpenConnection()
{
    WSAData data;

    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &data);
    if (wsResult != 0)
    {
        m_err_msgs.push_back("Can't start Winsock, err #" + WSAGetLastError());
        return 1;
    }

    m_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sock == INVALID_SOCKET)
    {
        WSACleanup();
        m_err_msgs.push_back("Can't Create socket, ERR #" + WSAGetLastError());
        return 1;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(m_port);
    int ipresult = inet_pton(AF_INET, m_ip_address.c_str(), &hint.sin_addr);
    if (ipresult == 0)
    {
        m_err_msgs.push_back("Format of Ip Wrong! Err #: " + WSAGetLastError());
        return 1;
    }

    int connResult = connect(m_sock, (sockaddr *)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR)
    {
        closesocket(m_sock);
        WSACleanup();
        m_err_msgs.push_back("Can't connect to server. Wrong IP? Err #" + WSAGetLastError());
        return 1;
    }

    return TELNET_OK;
}

int TelnetClient::CloseConnection()
{
    closesocket(m_sock);
    WSACleanup();
    return 0;
}

std::vector<std::string> TelnetClient::GetErrorMessages() {
    return m_err_msgs;
}

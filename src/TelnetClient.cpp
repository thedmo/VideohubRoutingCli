#include "TelnetClient.hpp"


// TODO extend library with Curl for crosscompatibility
TelnetClient::TelnetClient(std::string ip, int port, std::string &init_response, int &result)
    : m_ip_address(ip), m_port(port)
{

    result = OpenConnection();
    if (result != TELNET_OK)
    {
        AddToTrace("could not open connection.");
        return;
    }

    result = ReceiveMsgFromServer(init_response);
    if (result != TELNET_OK)
    {
        AddToTrace("Got no message from videohub");
        return;
    }

}

TelnetClient::~TelnetClient() { CloseConnection(); }

int TelnetClient::SendMsgToServer(std::string msg) {

    send(m_sock, msg.c_str(), msg.size(), 0);
    if (WSAGetLastError())
    {
        AddToTrace("could not send message: " + std::to_string(WSAGetLastError()));
        return 1;
    }

    ReceiveMsgFromServer(m_last_data_dump);
    if (WSAGetLastError())
    {
        AddToTrace("could not receive any data: " + std::to_string(WSAGetLastError()));
        return 1;
    }

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
        AddToTrace("ip adress empty..");
        return 1;
    }

    sockaddr_in newIp;
    newIp.sin_family = AF_INET;
    int _result = inet_pton(AF_INET, newAddress.c_str(), &newIp.sin_addr);

    if (_result != 1)
    {
        AddToTrace("not a valid IPv4 Adress");
        return 1;
    }

    m_ip_address = newAddress;

    _result = OpenConnection();
    if (_result != TELNET_OK)
    {
        AddToTrace("could not open connection.");
        return 1;
    }

    _result = ReceiveMsgFromServer(init_response);
    if (_result != TELNET_OK)
    {
        AddToTrace("could not receive message from server");
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
        AddToTrace("Can't start Winsock, err #" + std::to_string(WSAGetLastError()));
        return 1;
    }

    m_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sock == INVALID_SOCKET)
    {
        WSACleanup();
        AddToTrace("Can't Create socket, ERR #" + std::to_string(WSAGetLastError()));
        return 1;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(m_port);
    int ipresult = inet_pton(AF_INET, m_ip_address.c_str(), &hint.sin_addr);
    if (ipresult == 0)
    {
        AddToTrace("Format of Ip Wrong! Err #: " + std::to_string(WSAGetLastError()));
        return 1;
    }

    int connResult = connect(m_sock, (sockaddr *)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR)
    {
        closesocket(m_sock);
        WSACleanup();
        AddToTrace("Can't connect to server. Wrong IP? Err #" + std::to_string(WSAGetLastError()));
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

void TelnetClient::AddToTrace(std::string s) {
    m_err_msgs.push_back("TELNET_CLIENT: " + s);
}

std::vector<std::string> TelnetClient::GetErrorMessages() {
    return m_err_msgs;
}

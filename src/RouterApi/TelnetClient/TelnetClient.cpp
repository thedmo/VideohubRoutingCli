#include "TelnetClient.hpp"


// TODO extend library with Curl for crosscompatibility

/// <summary>
/// Creates a new Connection to a Server and connects to it.
/// </summary>
/// <param name="ip">IPv4 Address as string</param>
/// <param name="port">portnumber as int</param>
/// <param name="init_response">string to store initial response from server</param>
/// <param name="result">int for errorcode</param>
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

/// <summary>
/// Send Message to Server and stores response in member variable accessible via GetLastDataDump()
/// </summary>
/// <param name="msg">message to be sent</param>
/// <returns>result as int</returns>
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

/// <summary>
/// Awaits response from server and stores it in response
/// </summary>
/// <param name="response">handle to store response in</param>
/// <returns>result as int</returns>
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

/// <summary>
/// Changes IP Address of the Client and reconnects to the server
/// </summary>
/// <param name="newAddress">IPv4 address as string</param>
/// <param name="init_response">handle to store the initial response of the server</param>
/// <returns>result as int</returns>
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

/// <summary>
/// Returns the current IP Address of the Client
/// </summary>
/// <returns>ip address as std::string</returns>
std::string TelnetClient::GetIp() { return m_ip_address; }

/// <summary>
/// Returns the last response from the server
/// </summary>
/// <returns>response as std::string</returns>
std::string TelnetClient::GetLastDataDump() { return m_last_data_dump; }

/// <summary>
/// Connects to the server
/// </summary>
/// <returns>result as int</returns>
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

/// <summary>
/// Closes the connection to the server and cleanup
/// </summary>
/// <returns></returns>
int TelnetClient::CloseConnection()
{
    closesocket(m_sock);
    WSACleanup();
    return 0;
}

/// <summary>
/// Adds a message to the error message vector when stuff gets wrong
/// </summary>
/// <param name="s">message to be added</param>
void TelnetClient::AddToTrace(std::string s) {
    m_err_msgs.push_back("TELNET_CLIENT: " + s);
}

/// <summary>
/// Returns vector with all error messages
/// </summary>
/// <returns>Collected messages as std::vector<std::string></returns>
std::vector<std::string> TelnetClient::GetErrorMessages() {
    std::vector<std::string> temp = m_err_msgs;
    m_err_msgs.clear();
    return temp;
}

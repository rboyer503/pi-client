#include "SocketMgr.h"

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#define MAX_BUFFER_SIZE 230454  // Large enough for .bmp formatted 320x240x3 image.

using namespace std;
using namespace cv;


SocketMgr::SocketMgr() :
    m_monfd(-1), m_cmdfd(-1), m_connected(false), m_exited(false)
{
}

SocketMgr::~SocketMgr()
{
    Disconnect();
}

bool SocketMgr::Connect(const char * hostname)
{
    if (!HostnameToIP(hostname))
    {
        cerr << "Error: Could not find server '" << hostname << "'." << endl;
        return false;
    }

    if ( (m_monfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 )
    {
        cerr << "Error: Could not create socket." << endl;
        return false;
    }

    if ( (m_cmdfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 )
    {
        cerr << "Error: Could not create socket." << endl;
        close(m_monfd);
        return false;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(m_serverIP);
    serverAddr.sin_port = htons(SM_MONITOR_PORT);

    if ( connect(m_monfd, (const sockaddr *)&serverAddr, sizeof(serverAddr)) < 0 )
    {
        cerr << "Error: Could not connect to monitor port." << endl;
        close(m_monfd);
        close(m_cmdfd);
        return false;
    }

    serverAddr.sin_port = htons(SM_COMMAND_PORT);
    if ( connect(m_cmdfd, (const sockaddr *)&serverAddr, sizeof(serverAddr)) < 0 )
    {
        cerr << "Error: Could not connect to command port." << endl;
        close(m_monfd);
        close(m_cmdfd);
        return false;
    }

    m_threadMon = boost::thread(&SocketMgr::ManageMonitorStream, this);

    m_connected = true;
    return true;
}

void SocketMgr::Disconnect()
{
    if (m_connected)
    {
        if (m_threadMon.joinable())
        {
            m_threadMon.interrupt();
            m_threadMon.join();
        }

        close(m_monfd);
        close(m_cmdfd);
        m_connected = false;
    }
}

unique_ptr<Mat> SocketMgr::GetCurrentFrame()
{
    boost::mutex::scoped_lock lock(m_frameMutex);
    return move(m_currFrame);
}

void SocketMgr::SendCommand(const char * command)
{
    boost::mutex::scoped_lock lock(m_sendMutex);
    int ret;
    if ((ret = send(m_cmdfd, command, strlen(command), 0)) < 0)
    {
        cerr << "Error: send() returned " << ret << endl;
    }
}

bool SocketMgr::HostnameToIP(const char * hostname)
{
    struct addrinfo hints, *addrs;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status;
    if ( (status = getaddrinfo(hostname, nullptr, &hints, &addrs)) != 0 )
    {
        cerr << "Error: getaddrinfo() returned " << gai_strerror(status) << endl;
        return false;
    }

    struct sockaddr_in * addr_in = (struct sockaddr_in *)addrs->ai_addr;
    inet_ntop(addrs->ai_family, &(addr_in->sin_addr), m_serverIP, sizeof(m_serverIP));

    freeaddrinfo(addrs);
    return true;
}

void SocketMgr::ManageMonitorStream()
{
    // Enter monitor loop.
    char * pBuffer = new char[MAX_BUFFER_SIZE];
    int size = 0;
    int frame = 0;
    while (1)
    {
        // Get a frame from the server.
        if (!RecvFrame(pBuffer, size))
            break;

        // Decode into cv:Mat.
        vector<char> buf(pBuffer, pBuffer + size);
        {
            boost::mutex::scoped_lock lock(m_frameMutex);
            m_currFrame = make_unique<Mat>(imdecode(buf, 1));
        }

        // Give main thread an opportunity to shut this thread down.
        try
        {
            boost::this_thread::interruption_point();
        }
        catch (boost::thread_interrupted &)
        {
            break;
        }
    }

    // Clean up.
    delete [] pBuffer;
    m_exited = true;
}

bool SocketMgr::RecvFrame(char * pRawData, int & size)
{
    if ( !RecvDataField(reinterpret_cast<char *>(&size), sizeof(size)) )
    {
        cerr << "Error: Receive frame size failed." << endl;
        return false;
    }

    if ( (size > MAX_BUFFER_SIZE) || (size <= 0) )
    {
        cerr << "Error: Invalid frame size " << size << "." << endl;
        return false;
    }

    if ( !RecvDataField(pRawData, size) )
    {
        cerr << "Error: Receive frame failed." << endl;
        return false;
    }

    return true;
}

bool SocketMgr::RecvDataField(char * pData, int size)
{
    while (size > 0)
    {
        int bytesRecv = recv(m_monfd, pData, size, 0);
        if (bytesRecv > 0)
        {
            pData += bytesRecv;
            size -= bytesRecv;
            continue;
        }
        else if (bytesRecv == -1)
        {
            if ( (errno == EAGAIN) || (errno == EWOULDBLOCK) )
                continue;
        }

        // Something bad happened.
        break;
    }

    return (size == 0);
}

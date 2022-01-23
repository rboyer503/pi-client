#ifndef SOCKETMGR_H_
#define SOCKETMGR_H_

#include <arpa/inet.h>
#include <memory>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <opencv2/opencv.hpp>


class SocketMgr
{
    static constexpr int c_monitorPort = 34601;
    static constexpr int c_commandPort = 34602;
    static constexpr int c_numRxSegments = 4;
    static constexpr int c_maxBufferSize = 1048576; // Enough room for worst-case compression of 640x480x3 images.

    char m_serverIP[INET6_ADDRSTRLEN] = {};
    int m_monfd = -1;
    int m_cmdfd = -1;
    bool m_connected = false;
    boost::thread m_threadMon;
    volatile bool m_exited = false;
    std::unique_ptr<cv::Mat> m_currFrame;
    boost::mutex m_frameMutex;
    boost::mutex m_sendMutex;

public:
    SocketMgr() = default;
    ~SocketMgr();

    bool IsConnected() const { return m_connected; }
    bool HasExited() const { return m_exited; }

    bool Connect(const char * hostname, const char * token);
    void Disconnect();

    std::unique_ptr<cv::Mat> GetCurrentFrame();
    void SendCommand(const char * command);

private:
    bool HostnameToIP(const char * hostname);
    void ManageMonitorStream();
    bool RecvFrame(char * pRawData, int & size);
    bool RecvDataField(char * pData, int size);
};

#endif /* SOCKETMGR_H_ */


#ifndef SOCKETMGR_H_
#define SOCKETMGR_H_

#include <arpa/inet.h>
#include <memory>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <opencv2/opencv.hpp>

#define SM_MONITOR_PORT 5000
#define SM_COMMAND_PORT 5001


class SocketMgr
{
    char m_serverIP[INET6_ADDRSTRLEN] = {};
    int m_monfd;
    int m_cmdfd;
    bool m_connected;
    boost::thread m_threadMon;
    volatile bool m_exited;
    std::unique_ptr<cv::Mat> m_currFrame;
    boost::mutex m_frameMutex;
    boost::mutex m_sendMutex;

public:
    SocketMgr();
    ~SocketMgr();

    bool IsConnected() const { return m_connected; }
    bool HasExited() const { return m_exited; }

    bool Connect(const char * hostname);
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


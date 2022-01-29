#include "SocketMgr.h"
#include "Profiling.h"

#include <iostream>
#include <opencv2/opencv.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


using namespace std;
using namespace cv;


constexpr int c_windowWidth = 640;
constexpr int c_windowHeight = 480;


struct Statistics
{
    int numFrames;
    int currUs;
    int maxUs;
};


void usage()
{
    cout << "Usage: ./pi-client <server hostname> <token>" << endl;
}

int main(int argc, char * argv[])
{
    if (argc != 3)
    {
        usage();
        return 1;
    }

    SocketMgr socketMgr;
    if (!socketMgr.Connect(argv[1], argv[2]))
        return 1;

    const string windowName = "Pi Client - "s + argv[1];
    namedWindow(windowName, WINDOW_NORMAL);
    resizeWindow(windowName, c_windowWidth, c_windowHeight);

    Statistics stats = {};
    boost::posix_time::ptime startTime = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::ptime lastTime = boost::posix_time::microsec_clock::local_time();
    bool clientDebugMode = false;

    while (!socketMgr.HasExited())
    {
        char c = waitKey(5);
        if (c == 'q')
            break;
        else if (c == ' ')
        {
            clientDebugMode = !clientDebugMode;
        }
        else
        {
            if (!clientDebugMode)
            {
                if (c == 's')
                    socketMgr.SendCommand("status");
                else if (c == 'c')
                    socketMgr.SendCommand("config");
                else if (c == 'm')
                    socketMgr.SendCommand("mode");
                else if (c == 'p')
                    socketMgr.SendCommand("page");
                else if (c == '[')
                    socketMgr.SendCommand("param1 down");
                else if (c == ']')
                    socketMgr.SendCommand("param1 up");
                else if (c == '{')
                    socketMgr.SendCommand("param2 down");
                else if (c == '}')
                    socketMgr.SendCommand("param2 up");
                else if (c == '*')
                    socketMgr.SendCommand("debugmode");
            }
            else
            {
                if (c == 's')
                {
                    cout << endl << "Statistics" << endl;
                    cout << "\tTotal frames:\t" << stats.numFrames << endl;

                    if (stats.numFrames)
                    {
                        auto diffTime = boost::posix_time::microsec_clock::local_time() - startTime;
                        cout << "\tAverage FPS:\t" << (stats.numFrames / diffTime.total_seconds()) << endl;
                        cout << "\tCurrent us:\t" << stats.currUs << endl;
                        cout << "\tMax us:\t" << stats.maxUs << endl;
                    }
                }
            }
        }

        auto currFrame = socketMgr.GetCurrentFrame();
        if (currFrame)
        {
            if ( (currFrame->size().width > 0) && (currFrame->size().height > 0) )
            {
                PROFILE_START;
                imshow(windowName, *currFrame);
                PROFILE_LOG(SHOW);

                auto currTime = boost::posix_time::microsec_clock::local_time();
                ++stats.numFrames;
                stats.currUs = (currTime - lastTime).total_microseconds();
                lastTime = currTime;
                if (stats.maxUs < stats.currUs)
                    stats.maxUs = stats.currUs;
            }
            else
                cerr << "DEBUG: Missed frame" << endl;
        }
    }

    return 0;
}

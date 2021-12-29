#include <iostream>
#include <opencv2/opencv.hpp>
#include "SocketMgr.h"


using namespace std;
using namespace cv;


constexpr int c_windowWidth = 640;
constexpr int c_windowHeight = 480;


void usage()
{
    cout << "Usage: ./pi-client <server hostname>" << endl;
}

int main(int argc, char * argv[])
{
    if (argc != 2)
    {
        usage();
        return 1;
    }

    SocketMgr socketMgr;
    if (!socketMgr.Connect(argv[1]))
        return 1;

    const std::string windowName = "Pi Client - "s + argv[1];
    namedWindow(windowName, WINDOW_NORMAL);
    resizeWindow(windowName, c_windowWidth, c_windowHeight);

    bool debugMode = false;
    while (!socketMgr.HasExited())
    {
        char c = waitKey(50);
        if (c == 'q')
            break;
        else if (c == '*')
        {
            socketMgr.SendCommand("debugmode");
            debugMode = !debugMode;
        }
        else
        {
            if (debugMode)
            {
                if (c == 's')
                    socketMgr.SendCommand("status");
                else if (c == 'c')
                    socketMgr.SendCommand("config");
                else if (c == 'm')
                    socketMgr.SendCommand("mode");
                else if (c == 'p')
                    socketMgr.SendCommand("page");
                else if (c == 'd')
                    socketMgr.SendCommand("debug");
                else if (c == '[')
                    socketMgr.SendCommand("param1 down");
                else if (c == ']')
                    socketMgr.SendCommand("param1 up");
                else if (c == '{')
                    socketMgr.SendCommand("param2 down");
                else if (c == '}')
                    socketMgr.SendCommand("param2 up");
            }
        }

        auto currFrame = socketMgr.GetCurrentFrame();
        if (currFrame)
        {
            if ( (currFrame->size().width > 0) && (currFrame->size().height > 0) )
            {
                imshow(windowName, *currFrame);
            }
            else
                cerr << "DEBUG: Missed frame" << endl;
        }
    }

    return 0;
}

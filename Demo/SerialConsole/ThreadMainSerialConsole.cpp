#include "ThreadMainSerialConsole.h"

#include "ThreadSerialPort.h"

ThreadMainSerialConsole::ThreadMainSerialConsole()
    : ThreadMainDaemon("Thread.Main",
                       "SerialConsole",
                       "Serial Console",
                       nullptr)
    , _threadSerialPort(nullptr)
{
}

void ThreadMainSerialConsole::startChildThreads()
{
    _threadSerialPort = registerAndStartChildThread(new ThreadSerialPort(this, "/dev/ttyACM0"));
}

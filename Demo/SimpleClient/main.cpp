#include "ThreadMainSimpleClient.h"

#include "../../Threads/DaemonApplication.h"

int main(int argc, char *argv[])
{
    return AppMain<ThreadMainSimpleClient>(argc, argv, "Simple Client");
}

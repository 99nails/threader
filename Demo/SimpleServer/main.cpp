#include "ThreadMainSimpleServer.h"

#include "DaemonApplication.h"

int main(int argc, char *argv[])
{
    return AppMain<ThreadMainSimpleServer>(argc, argv, "Simple_service");
}


#include <VideoMonitor.h>
#include <iostream>

#include "NDIReceiver.h"

int main(int argc, char *argv[])
{
	VideoMonitor Monitor;
	return Monitor.Run();
}

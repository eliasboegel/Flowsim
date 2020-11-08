#include "SimApp.h"


int main(int argc, char* args[])
{
	SimApp* app = new SimApp();
	app->mainLoop();
	delete app;

	return 0;
}
#pragma once
#include "Window.h"
#include "Timer.h"


class App
{
public:
	App();
	~App();
	int GO();

private:
	void DoFrame();

	Window wnd;
	Timer timer;
};
#include "pch.h"
#include "App.h"

App::App()
	: wnd(800, 600, L"DirectX11")
{
	
}

App::~App()
{

}

int App::GO()
{
	
	while (true)
	{
		if (const auto ecode = Window::ProcessMessages())
		{
			return *ecode;
		}
		DoFrame();
	}

}

void App::DoFrame()
{
	const float t = timer.Peek();
	wnd.GetGraphics().ClearBuffer(0.0f,0.0f,0.5f);
	wnd.GetGraphics().EndFrame();
}

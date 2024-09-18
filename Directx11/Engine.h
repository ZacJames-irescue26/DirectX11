#pragma once
#include "WindowContainer.h"
#include "Timer.h"
namespace Engine
{
class EngineInit : public WindowContainer
{
public:
	bool Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	bool ProcessMessages();
	void Update();
	void RenderFrame();

	Timer timer;

};
}
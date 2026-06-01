#pragma once
#include "RenderWindow.h"
#include "Keyboard/KeyboardClass.h"
#include "Mouse/MouseClass.h"
#include "Controller/ControllerClass.h"
#include "Graphics/Graphics.h"
namespace Engine
{
class WindowContainer
{
public:
	WindowContainer();
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
public:
	RenderWindow render_window;
	KeyboardClass keyboard;
	MouseClass mouse;
	ControllerClass controller;
	Graphics gfx;
protected:
private:
};
}
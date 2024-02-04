#include "pch.h"
#include "WindowsMessageMap.h"
#include "Window.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		if (wParam == (WPARAM)L"F")
		{
		}
		break;
	case  WM_CHAR:
		break;
	case WM_LBUTTONDOWN:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	Window wnd(800,500, L"DirectX11");
	
	MSG msg;
	BOOL gResult;
	while (gResult = GetMessage(& msg, nullptr, 0, 0) > 0 )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (gResult == -1)
	{
		return -1;
	}
	return msg.wParam;
}

#include "pch.h"
#include "WindowsMessageMap.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static WindowsMessageMap mm;
	//OutputDebugString((LPCWSTR)mm(msg,lParam,wParam).c_str());
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
	// register window class
	const LPCWSTR pClassName = L"DirectX11";



	//create window instance
	HWND hWnd = CreateWindowEx(
		0, (LPCWSTR)pClassName, (LPCWSTR)pClassName,
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		200,200,640,480,nullptr,nullptr,hInstance,nullptr);
	ShowWindow(hWnd, SW_SHOW);
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
	else
	{
		return msg.wParam;
	}
}

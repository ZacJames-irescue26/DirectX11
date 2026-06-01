#pragma once
#include <string>
#include <fstream>

#include <iostream>
#include <sstream>

#include <filesystem>

#include <windows.h>
#include <commdlg.h>
#include "pch.h"
namespace Utils
{

	inline std::string OpenFile(const char* filter, HWND hwnd)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		CHAR currentDir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hwnd;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (GetCurrentDirectoryA(256, currentDir))
			ofn.lpstrInitialDir = currentDir;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return std::string();

	}

	inline std::string SaveFile(const char* filter, HWND hwnd)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		CHAR currentDir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hwnd;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (GetCurrentDirectoryA(256, currentDir))
			ofn.lpstrInitialDir = currentDir;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

		// Sets the default extension by extracting it from the filter
		ofn.lpstrDefExt = strchr(filter, '\0') + 1;

		if (GetSaveFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return std::string();
	}

	inline DirectX::XMFLOAT3 QuaternionToEuler(const DirectX::XMFLOAT4& q)
	{
		float x = q.x;
		float y = q.y;
		float z = q.z;
		float w = q.w;

		DirectX::XMFLOAT3 euler;

		// Pitch / X rotation
		float sinp = 2.0f * (w * x + y * z);
		float cosp = 1.0f - 2.0f * (x * x + y * y);
		euler.x = std::atan2(sinp, cosp);

		// Yaw / Y rotation
		float siny = 2.0f * (w * y - z * x);
		if (std::abs(siny) >= 1.0f)
			euler.y = std::copysign(DirectX::XM_PIDIV2, siny);
		else
			euler.y = std::asin(siny);

		// Roll / Z rotation
		float sinr = 2.0f * (w * z + x * y);
		float cosr = 1.0f - 2.0f * (y * y + z * z);
		euler.z = std::atan2(sinr, cosr);

		return euler;
	}
	
}


#pragma once
#include "StringConverter.h"
#include <string>
#define JPH_ENABLE_ASSERTS

namespace Engine
{
class COMException;
class ErrorLogger
{
public:
	static void Log(std::string message);
	static void Log(HRESULT hr, std::string message);
	static void Log(HRESULT hr, std::wstring message);
	static void Log(COMException& exception);
	// Callback for asserts, connect this to your own assert handler if you have one
	
#ifdef JPH_ENABLE_ASSERTS
	static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, UINT inLine)
	{
		// Print to the TTY
		Log(std::string(inFile) + ":" + std::to_string(inLine) + ": (" + std::string(inExpression) + ") " + (inMessage != nullptr ? std::string(inMessage) : ""));

		// Breakpoint
		return true;
	};

#endif // JPH_ENABLE_ASSERTS
};
#define COM_ERROR_IF_FAILED( hr, msg ) if( FAILED( hr ) ) throw COMException( hr, msg, __FILE__, __FUNCTION__, __LINE__ )

class COMException
{
public:
	COMException(HRESULT hr, const std::string& msg, const std::string& file, const std::string& function, int line)
	{
		_com_error error(hr);
		whatmsg = L"Msg: " + StringHelper::StringToWide(std::string(msg)) + L"\n";
		whatmsg += error.ErrorMessage();
		whatmsg += L"\nFile: " + StringHelper::StringToWide(file);
		whatmsg += L"\nFunction: " + StringHelper::StringToWide(function);
		whatmsg += L"\nLine: " + StringHelper::StringToWide(std::to_string(line));
	}

	const wchar_t* what() const
	{
		return whatmsg.c_str();
	}
private:
	std::wstring whatmsg;
};
}
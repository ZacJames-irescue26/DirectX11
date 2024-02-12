#include "pch.h"
#include "Exception.h"

DOEException::DOEException(int line, const char* file) noexcept
	:line(line), file(file)
{

}

const char* DOEException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
	<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* DOEException::GetType() const noexcept
{
	return "DOE Exception";
}

int DOEException::GetLine() const noexcept
{
	return line;
}

const std::string& DOEException::GetFile() const noexcept
{
	return file;
}

std::string DOEException::GetOriginString() const noexcept
{
	std::ostringstream oss;
	oss << "[File] " << file << std::endl
		<< "[Line] " << line;
	return oss.str();
}

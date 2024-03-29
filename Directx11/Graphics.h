#pragma once
#include "Exception.h"

class Graphics
{
public:
	class Exception : public DOEException
	{
		using DOEException::DOEException;
	};
	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {}) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
		std::string GetErrorDescription() const noexcept;
		std::string GetErrorInfo() const noexcept;
	private:
		HRESULT hr;
		std::string info;
	};
	class InfoException : public Exception
	{
	public:
		InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		std::string GetErrorInfo() const noexcept;
	private:
		std::string info;
	};
	class DeviceRemovedException : public HrException
	{
		using HrException::HrException;
	public:
		const char* GetType() const noexcept override;
	private:
		std::string reason;
	};




	Graphics(HWND hwnd, int width, int height);
	//shouldnt be copied
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;

	~Graphics();
	void EndFrame();
	void ClearBuffer(float r, float g, float b) noexcept
	{
		const float color[] = {r,g,b,1.0f};
		pContext->ClearRenderTargetView(pTarget.Get(), color);
		pContext->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
	
	}

	void DrawTestTriangle();

private:
	Microsoft::WRL::ComPtr < ID3D11Device> pDevice = nullptr;
	Microsoft::WRL::ComPtr < IDXGISwapChain> pSwap = nullptr;
	Microsoft::WRL::ComPtr < ID3D11DeviceContext> pContext = nullptr;
	Microsoft::WRL::ComPtr < ID3D11RenderTargetView> pTarget = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDSV;

};
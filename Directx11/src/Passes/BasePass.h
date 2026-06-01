#pragma once
#include <d3d11.h>
#include "src/Graphics/Graphics.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include "src/Scene/Project.h"
namespace Engine
{

	const UINT offset = 0;

	class BasePass
	{
		virtual bool Initialize(ID3D11Device* device) = 0;
		virtual void Draw(Graphics* gfx) = 0;
		
	public:
		virtual void ImGuiPass() = 0;
		virtual std::vector<ID3D11ShaderResourceView*> GetSRVRenderTarget() = 0;
	};
}
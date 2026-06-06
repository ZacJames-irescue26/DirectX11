#pragma once
#include "BasePass.h"
#include "illumination\Probe.h"
#include "src/Scene/Scene.h"
#include "Acceleration\Octree\Octree.h"
#include "illumination\SurfelGenerator.h"
namespace Engine
{
	
	class ProbeCubemapCreationPass : public BasePass
	{

	public:
		void ImGuiPass() override;


		std::vector<ID3D11ShaderResourceView*> GetSRVRenderTarget() override;
		
		bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, uint32_t faceSize, const std::vector<Probe>& probes);
		Camera MakeCubemapFaceCamera(const XMFLOAT3& probePosition, int face, float nearPlane, float farPlane);
		bool CreateStagingTexture(ID3D11Device* device, ID3D11Texture2D* source, Microsoft::WRL::ComPtr<ID3D11Texture2D>& staging);
		void BeginFace(ID3D11DeviceContext* context, int face);
		void Draw(Graphics* gfx, const std::vector<Probe>& probes, Scene& scene);
	
		void GenerateAndMergeSurfelsFromCubemapCPU(ID3D11DeviceContext* context, const Probe& probe, uint32_t probeIndex, const XMMATRIX invViewProjCPU[6], Engine::SurfelBuilder& builder, int pixelStep, bool reversedZ, bool albedoIsSRGB);
		bool UploadMergedSurfelsToGPU(ID3D11Device* device, const std::vector<Surfel>& mergedSurfels);
		void LightSurfels(Graphics* gfx, ID3D11ShaderResourceView* ShadowSRV, XMMATRIX Lightspacematrix, XMFLOAT3 LightDir, XMFLOAT3 LightColor);
		void NormalizeAndAccumilate(Graphics* gfx);

		inline ID3D11ShaderResourceView* GetProbesSRV()
		{
			return ProbeSRV.Get();
		}
		

	private:
		bool Initialize(ID3D11Device* device) override;
		VertexShader m_ProbeGBuffer;
		PixelShader m_ProbeGBufferpixelShader;
		ComputeShader m_ProbeCS;
		ComputeShader m_SurfelLightingCS;
		ComputeShader m_AccumilateProbesCS;
		ConstantBuffer<Engine::SurfelCreation> m_ConstantBufferSurfelCreation;
		ConstantBuffer<Engine::ComputeLightCB> m_ConstantBufferSurfelLighting;
		ConstantBuffer<Engine::ComputeNormalizeCB> m_ConstantBufferProbeAccum;
		void Draw(Graphics* gfx) override;

		Microsoft::WRL::ComPtr < ID3D11Texture2D> NormalTexture;
		Microsoft::WRL::ComPtr < ID3D11RenderTargetView> NormalRTV[6];
		Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> NormalSRV;
		Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> ComputeNormalSRV;

		Microsoft::WRL::ComPtr < ID3D11Texture2D> AlbedoTexture;
		Microsoft::WRL::ComPtr < ID3D11RenderTargetView> AlbedoRTV[6];
		Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> AlbedoSRV;
		Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> ComputeAlbedoSRV;


		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthDSV[6];
		Microsoft::WRL::ComPtr<ID3D11Texture2D> DepthTexture;
		Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> DepthSRV;
		Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> ComputeDepthSRV;

		Microsoft::WRL::ComPtr < ID3D11DepthStencilState> DSS;

		Microsoft::WRL::ComPtr<ID3D11Buffer> GlobalSurfelBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GlobalSurfelSRV;

		uint32_t GlobalSurfelCount = 0;
		Microsoft::WRL::ComPtr<ID3D11Buffer> ProbePositionBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ProbePositionSRV;

		XMMATRIX InverseViewProjection[6];
		int FaceSize = 64;
		uint32_t probeCount = 0;
		float RadianceScale = 100.0f;
		Microsoft::WRL::ComPtr<ID3D11Buffer> ProbeBuffer;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> ProbeUAV;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ProbeSRV;  


		Microsoft::WRL::ComPtr<ID3D11Buffer> ProbeAccumBuffer;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> ProbeAccumUAV;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ProbeAccumSRV;

		
		Microsoft::WRL::ComPtr<ID3D11Texture2D> AlbedoStaging;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> NormalStaging;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> DepthStaging;

		Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	};




}
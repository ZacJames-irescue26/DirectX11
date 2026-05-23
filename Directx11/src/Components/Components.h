#pragma once

#include "pch.h"
#include "Graphics/ModelSimple.h"
#include <string>
#include "src/Graphics/AnimatedModel.h"

namespace Engine
{
	enum ComponentEnum
	{
		Transform,
		StaticMesh,
		AnimatedMeshenum,
		PointLight,
		DirectionalLight,
		SpotLight,
	};

	struct Component
	{
	public:
		virtual ~Component() = default;
		virtual ComponentEnum GetType() const = 0;
	};
	struct TransformComponent : public Component
	{

		TransformComponent(){}

		TransformComponent(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale)
		:Position(pos), Rotation(rot), Scale(scale)
		{
			CalculateModelMatrix();
		}
		static ComponentEnum StaticType()
		{
			return ComponentEnum::Transform;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}
		XMFLOAT3 Position = XMFLOAT3(0.0,0.0,0.0);
		XMFLOAT3 Rotation = XMFLOAT3(0.0,0.0,0.0);
		XMFLOAT3 Scale = XMFLOAT3(1.0,1.0,1.0);
		XMMATRIX ModelMatrix;
	
		XMMATRIX CalculateModelMatrix()
		{

			this->ModelMatrix = XMMatrixScaling(Scale.x, Scale.y, Scale.z) * XMMatrixRotationRollPitchYaw(this->Rotation.x, this->Rotation.y, this->Rotation.z) * XMMatrixTranslation(this->Position.x, this->Position.y, this->Position.z);
			return ModelMatrix;
		}


	};

	struct StaticMeshComponent : public Component
	{
		StaticMeshComponent()
		{ }
		StaticMeshComponent(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexShader>& cb_vs_vertexshader)
		: m_device(device), m_deviceContext(deviceContext), m_cb_vs_vertexshader(&cb_vs_vertexshader)	
		{
			m_filepath = filePath;
			m_Model.Initialize(filePath, device, deviceContext, cb_vs_vertexshader);
			Initialized = true;
		}
		static ComponentEnum StaticType()
		{
			return ComponentEnum::StaticMesh;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}
		void Draw(const XMMATRIX& worldMatrix, const XMMATRIX& viewProjectionMatrix)
		{
			if (Initialized)
			{
				m_Model.Draw(worldMatrix, viewProjectionMatrix);
			}
		}
		void DrawWithoutCBuffer()
		{
			if (Initialized)
			{
				m_Model.Draw();
			}
		}
		void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexShader>* cb_vs_vertexshader)
		{
			Initialized = true;
			m_Model.Initialize(m_filepath, device, deviceContext, *cb_vs_vertexshader);
		}
		Model m_Model;
		std::string m_filepath;
		bool Initialized = false;
	private:
		ID3D11Device* m_device;
		ID3D11DeviceContext* m_deviceContext;
		ConstantBuffer<CB_VS_vertexShader>* m_cb_vs_vertexshader;
	};
	struct AnimatedMeshComponent : public Component 
	{
		AnimatedMeshComponent(){}
		AnimatedMeshComponent(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_Anim_VS_vertexShader>& cb_vs_vertexshader)
			: m_device(device), m_deviceContext(deviceContext), m_cb_vs_vertexshader(&cb_vs_vertexshader)
		{
			m_filepath = filePath;
			Initialized = m_Model.Initialize(filePath, device, deviceContext, cb_vs_vertexshader);
		}

		static ComponentEnum StaticType()
		{
			return ComponentEnum::AnimatedMeshenum;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}
		void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_Anim_VS_vertexShader>* cb_vs_vertexshader)
		{
			Initialized = m_Model.Initialize(m_filepath, device, deviceContext, *cb_vs_vertexshader);
		}
		void Draw(const XMMATRIX& worldMatrix, const XMMATRIX& viewProjectionMatrix)
		{
			if (Initialized)
			{
				m_Model.Draw(worldMatrix, viewProjectionMatrix);
			}
		}
		void AddAnimation(const std::string& path)
		{
			m_Model.LoadAnimationOnly(path);
		}
		void RemoveAnimation(uint32_t index)
		{
			m_Model.RemoveAnimation(index);
		}
		void RemoveAnimationByName(const std::string& name)
		{

		}
		AnimatedModel m_Model;
		std::string m_filepath;
		std::string m_AnimPath;
		bool Initialized = false;
		bool m_PlayAnimation;
	private:
		ID3D11Device* m_device;
		ID3D11DeviceContext* m_deviceContext;
		ConstantBuffer<CB_Anim_VS_vertexShader>* m_cb_vs_vertexshader;

	};

	enum class LightType
	{
		None = 0, Directional = 1, Point = 2, Spot = 3
	};

	struct DirectionalLightComponent : public Component
	{
		DirectionalLightComponent(){}
		static ComponentEnum StaticType()
		{
			return ComponentEnum::DirectionalLight;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}
		XMFLOAT3 Radiance = { 1.0f, 1.0f, 1.0f };
		float Intensity = 1.0f;
		bool CastShadows = true;
		bool SoftShadows = true;
		float LightSize = 0.5f; // For PCSS
		float ShadowAmount = 1.0f;
	};

	struct PointLightComponent : public Component
	{
		PointLightComponent(){}
		static ComponentEnum StaticType()
		{
			return ComponentEnum::PointLight;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}
		XMFLOAT3 Radiance = { 1.0f, 1.0f, 1.0f };
		float Intensity = 1.0f;
		float LightSize = 0.5f; // For PCSS
		float MinRadius = 1.f;
		float Radius = 10.f;
		bool CastsShadows = true;
		bool SoftShadows = true;
		float Falloff = 1.f;
	};

	struct SpotLightComponent : public Component
	{
		SpotLightComponent(){}
		static ComponentEnum StaticType()
		{
			return ComponentEnum::SpotLight;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}
		XMFLOAT3 Radiance = { 1.0f,1.0,1.0 };
		float Intensity = 1.0f;
		float Range = 10.0f;
		float Angle = 60.0f;
		float AngleAttenuation = 5.0f;
		bool CastsShadows = false;
		bool SoftShadows = false;
		float Falloff = 1.0f;
	};

}
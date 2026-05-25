#pragma once
#include "Scene\Scene.h"
#include "EngineInclude.h"
namespace Editor
{

	class SceneHierarchyPanel
	{

	public:
		SceneHierarchyPanel() = default;

		SceneHierarchyPanel(Engine::Scene* scene, ID3D11Device* device, ID3D11DeviceContext* deviceContext, Engine::ConstantBuffer<Engine::CB_VS_vertexShader>* cb_vs_vertexshader, Engine::ConstantBuffer<Engine::CB_Anim_VS_vertexShader>* cb_anim_vs_vertexshader);
		void SetContext(Engine::Scene* context);

		void OnImGuiRender();

		void HandleDragDrop(Engine::Entity* targetEntity);
		void SetSelectedObject(std::shared_ptr < Engine::Entity> ent);
		Engine::Entity* GetSelectedObject() const { return m_SelectionContext; }
	private:
		void DrawEntity(Engine::Entity* entity);
		void DrawEntityNode(Engine::Entity* entity);
		
		
		void DrawComponents(Engine::Entity* entity);
		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName) {
			if (!m_SelectionContext->HasComponent<T>())
			{
				if (ImGui::MenuItem(entryName.c_str()))
				{
					m_SelectionContext->AddComponent<T>(std::make_unique<T>());
					ImGui::CloseCurrentPopup();
				}
			}
		}
		
		
	private:
		Engine::Scene* m_Context;
		Engine::Entity* m_SelectionContext = nullptr;
		std::string m_MeshPath;
		std::string m_skeletonPath;
		std::string m_animationPath;
		std::string m_ModelPath;
		Engine::UUID IDToDelete;
		bool entityDeleted = false;
		float PlabackSpeed = 1.0f;
		ID3D11Device* m_device; 
		ID3D11DeviceContext* m_deviceContext;
		Engine::ConstantBuffer<Engine::CB_VS_vertexShader>* m_cb_vs_vertexshader;
		Engine::ConstantBuffer<Engine::CB_Anim_VS_vertexShader>* m_cb_anim_vs_vertexshader;
	};
}
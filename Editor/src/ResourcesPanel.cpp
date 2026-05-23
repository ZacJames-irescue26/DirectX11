#include "Gamepch.h"
#include "ResourcesPanel.h"
#include "ImGui/imgui_internal.h"

namespace Editor
{
	SceneHierarchyPanel::SceneHierarchyPanel(
		Engine::Scene* scene,
		ID3D11Device* device,
		ID3D11DeviceContext* deviceContext,
		Engine::ConstantBuffer<Engine::CB_VS_vertexShader>* cb_vs_vertexshader,
		Engine::ConstantBuffer<Engine::CB_Anim_VS_vertexShader>* cb_anim_vs_vertexshader)
		: m_device(device),
		m_deviceContext(deviceContext),
		m_cb_vs_vertexshader(cb_vs_vertexshader),
		m_cb_anim_vs_vertexshader(cb_anim_vs_vertexshader)
	{
		SetContext(scene);
	}

	void SceneHierarchyPanel::SetContext(Engine::Scene* context)
	{
		m_Context = context;
		m_SelectionContext = nullptr;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		if (m_Context)
		{
			for (auto& value : m_Context->GetEntities())
			{
				DrawEntityNode(value.get());
			}

			if (ImGui::IsWindowHovered() &&
				ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
				!ImGui::IsAnyItemHovered())
			{
				m_SelectionContext = nullptr;
			}

			if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
			{
				if (ImGui::MenuItem("Create Blank Entity"))
				{
					m_Context->AddEntity("Blank Entity");
				}

				if (ImGui::BeginMenu("Lighting"))
				{
					if (ImGui::MenuItem("Create Directional Light"))
					{
						auto entity = m_Context->AddEntity("Directional Light");
						
						entity->AddComponent<Engine::DirectionalLightComponent>(std::make_unique<Engine::DirectionalLightComponent>());
					}

					if (ImGui::MenuItem("Create Point Light"))
					{
						auto entity = m_Context->AddEntity("Point Light");
						
						entity->AddComponent<Engine::PointLightComponent>(std::make_unique<Engine::PointLightComponent>());
					}

					if (ImGui::MenuItem("Create Spot Light"))
					{
						auto entity = m_Context->AddEntity("Spot Light");
						
						entity->AddComponent<Engine::SpotLightComponent>(std::make_unique<Engine::SpotLightComponent>());
					}

					ImGui::EndMenu();
				}

				ImGui::EndPopup();
			}
		}

		ImGui::End();

		if (m_SelectionContext)
		{
			DrawComponents(m_SelectionContext);
		}
	}

	void SceneHierarchyPanel::DrawEntityNode(Engine::Entity* entity)
	{
		if (!entity)
			return;

		auto& tag = entity->GetName();

		ImGuiTreeNodeFlags flags =
			((m_SelectionContext && m_SelectionContext->GetUUID() == entity->GetUUID())
				? ImGuiTreeNodeFlags_Selected
				: 0)
			| ImGuiTreeNodeFlags_OpenOnArrow
			| ImGuiTreeNodeFlags_SpanAvailWidth;

		bool opened = ImGui::TreeNodeEx(
			reinterpret_cast<void*>(static_cast<uint64_t>(entity->GetUUID())),
			flags,
			"%s",
			tag.c_str()
		);

		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}

		bool entityDeleted = false;

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
			{
				entityDeleted = true;
			}

			ImGui::EndPopup();
		}

		if (opened)
		{
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			Engine::UUID deletedId = entity->GetUUID();

			if (m_SelectionContext && m_SelectionContext->GetUUID() == deletedId)
			{
				m_SelectionContext = nullptr;
			}

			m_Context->DestroyEntity(deletedId);
		}
	}

	static void DrawVec3Control(
		const std::string& label,
		XMFLOAT3& values,
		float resetValue = 0.0f,
		float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImFont* boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::TextUnformatted(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		ImGui::Columns(1);
		ImGui::PopID();
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(
		const std::string& name,
		Engine::Entity* entity,
		UIFunction uiFunction)
	{
		if (!entity || !entity->HasComponent<T>())
			return;

		T* component = entity->GetComponent<T>();
		if (!component)
			return;

		const ImGuiTreeNodeFlags treeNodeFlags =
			ImGuiTreeNodeFlags_DefaultOpen |
			ImGuiTreeNodeFlags_Framed |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			ImGuiTreeNodeFlags_AllowItemOverlap |
			ImGuiTreeNodeFlags_FramePadding;

		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });

		float lineHeight =
			GImGui->Font->FontSize +
			GImGui->Style.FramePadding.y * 2.0f;

		ImGui::Separator();

		bool open = ImGui::TreeNodeEx(
			reinterpret_cast<void*>(typeid(T).hash_code()),
			treeNodeFlags,
			"%s",
			name.c_str()
		);

		ImGui::PopStyleVar();

		ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);

		if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
		{
			ImGui::OpenPopup("ComponentSettings");
		}

		bool removeComponent = false;

		if (ImGui::BeginPopup("ComponentSettings"))
		{
			if (ImGui::MenuItem("Remove component"))
				removeComponent = true;

			ImGui::EndPopup();
		}

		if (open)
		{
			uiFunction(*component);
			ImGui::TreePop();
		}

		if (removeComponent)
		{
			entity->RemoveComponent<T>();
		}
	}

	void SceneHierarchyPanel::DrawComponents(Engine::Entity* entity)
	{
	ImGui::Begin("Components");
		if (!entity)
			return;

		char buffer[256] = {};
		strncpy_s(buffer, sizeof(buffer), entity->GetName().c_str(), _TRUNCATE);

		if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
		{
			entity->GetName() = std::string(buffer);
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("AddComponent");
		}

		if (ImGui::BeginPopup("AddComponent"))
		{
			DisplayAddComponentEntry<Engine::TransformComponent>("Transform");
			DisplayAddComponentEntry<Engine::StaticMeshComponent>("StaticMesh");
			DisplayAddComponentEntry<Engine::DirectionalLightComponent>("DirectionalLight");
			DisplayAddComponentEntry<Engine::SpotLightComponent>("SpotLight");
			DisplayAddComponentEntry<Engine::PointLightComponent>("PointLight");

			ImGui::EndPopup();
		}

		DrawComponent<Engine::TransformComponent>("Transform", entity, [](auto& component)
			{
				DrawVec3Control("Translation", component.Position);

				XMFLOAT3 rotationDegrees =
				{
					XMConvertToDegrees(component.Rotation.x),
					XMConvertToDegrees(component.Rotation.y),
					XMConvertToDegrees(component.Rotation.z)
				};

				DrawVec3Control("Rotation", rotationDegrees);

				component.Rotation =
				{
					XMConvertToRadians(rotationDegrees.x),
					XMConvertToRadians(rotationDegrees.y),
					XMConvertToRadians(rotationDegrees.z)
				};

				DrawVec3Control("Scale", component.Scale, 1.0f);

				component.CalculateModelMatrix();
			});

		DrawComponent<Engine::StaticMeshComponent>("StaticMesh", entity, [&](auto& component)
			{
				char pathBuffer[512] = {};
				strncpy_s(pathBuffer, sizeof(pathBuffer), component.m_filepath.c_str(), _TRUNCATE);

				if (ImGui::InputText("File Path", pathBuffer, sizeof(pathBuffer)))
				{
					component.m_filepath = std::string(pathBuffer);
				}

				if (ImGui::Button("Reload Mesh"))
				{
					component.Initialize(m_device, m_deviceContext, m_cb_vs_vertexshader);
				}
			});
		DrawComponent<Engine::StaticMeshComponent>("StaticMesh", entity, [&](auto& component)
			{
				char pathBuffer[512] = {};
				strncpy_s(pathBuffer, sizeof(pathBuffer), component.m_filepath.c_str(), _TRUNCATE);

				if (ImGui::InputText("File Path", pathBuffer, sizeof(pathBuffer)))
				{
					component.m_filepath = std::string(pathBuffer);
				}

				if (ImGui::Button("Reload Mesh"))
				{
					component.Initialize(m_device, m_deviceContext, m_cb_anim_vs_vertexshader);
				}



			});


		DrawComponent<Engine::AnimatedMeshComponent>("AnimMesh", entity, [&](auto& component)
			{
				char pathBuffer[512] = {};
				strncpy_s(pathBuffer, sizeof(pathBuffer), component.m_filepath.c_str(), _TRUNCATE);

				if (ImGui::InputText("File Path", pathBuffer, sizeof(pathBuffer)))
				{
					component.m_filepath = std::string(pathBuffer);
				}

				char animpathBuffer[512] = {};
				strncpy_s(animpathBuffer, sizeof(animpathBuffer), component.m_AnimPath.c_str(), _TRUNCATE);

				if (ImGui::InputText("Anim File Path", animpathBuffer, sizeof(animpathBuffer)))
				{
					component.m_AnimPath = std::string(animpathBuffer);
				}

				if (ImGui::Button("Reload Mesh"))
				{
					component.Initialize(m_device, m_deviceContext, m_cb_vs_vertexshader);
				}

				if (ImGui::Button("Load animation from filepath"))
				{
					component.AddAnimation(component.m_AnimPath);
				}

				const auto& animations = component.m_Model.GetLoadedAnimations();

				if (animations.empty())
				{
					ImGui::TextUnformatted("No animations loaded");
				}
				else
				{
					int currentIndex = component.m_Model.GetCurrentAnimationIndex();

					const char* previewName = "None";

					if (currentIndex >= 0 && currentIndex < static_cast<int>(animations.size()))
					{
						previewName = animations[currentIndex].Name.c_str();
					}

					if (ImGui::BeginCombo("Animations", previewName))
					{
						for (int i = 0; i < static_cast<int>(animations.size()); ++i)
						{
							const bool selected = (currentIndex == i);

							std::string label = animations[i].Name.empty()
								? "Animation " + std::to_string(i)
								: animations[i].Name;

							if (ImGui::Selectable(label.c_str(), selected))
							{
								component.m_Model.SetCurrentAnimationIndex(i);
							}

							if (selected)
							{
								ImGui::SetItemDefaultFocus();
							}
						}

						ImGui::EndCombo();
					}
				}
				ImGui::Checkbox("Play Animation", m_PlayAnimation);

			});

		DrawComponent<Engine::DirectionalLightComponent>("DirectionalLight", entity, [](auto& component)
			{
				DrawVec3Control("Radiance", component.Radiance);
				ImGui::DragFloat("Intensity", &component.Intensity, 0.1f);
				ImGui::Checkbox("Cast Shadows", &component.CastShadows);
				ImGui::Checkbox("Soft Shadows", &component.SoftShadows);
				ImGui::DragFloat("Light Size", &component.LightSize, 0.1f);
				ImGui::DragFloat("Shadow Amount", &component.ShadowAmount, 0.1f, 0.0f, 1.0f);
			});

		DrawComponent<Engine::PointLightComponent>("PointLight", entity, [](auto& component)
			{
				DrawVec3Control("Radiance", component.Radiance);
				ImGui::DragFloat("Intensity", &component.Intensity, 0.1f);
				ImGui::DragFloat("Min Radius", &component.MinRadius, 0.1f);
				ImGui::DragFloat("Radius", &component.Radius, 0.1f);
				ImGui::DragFloat("Falloff", &component.Falloff, 0.1f);
				ImGui::DragFloat("Light Size", &component.LightSize, 0.1f);
				ImGui::Checkbox("Cast Shadows", &component.CastsShadows);
				ImGui::Checkbox("Soft Shadows", &component.SoftShadows);
			});

		DrawComponent<Engine::SpotLightComponent>("SpotLight", entity, [](auto& component)
			{
				DrawVec3Control("Radiance", component.Radiance);
				ImGui::DragFloat("Intensity", &component.Intensity, 0.1f);
				ImGui::DragFloat("Range", &component.Range, 0.1f);
				ImGui::DragFloat("Angle", &component.Angle, 0.1f);
				ImGui::DragFloat("Angle Attenuation", &component.AngleAttenuation, 0.1f);
				ImGui::DragFloat("Falloff", &component.Falloff, 0.1f);
				ImGui::Checkbox("Cast Shadows", &component.CastsShadows);
				ImGui::Checkbox("Soft Shadows", &component.SoftShadows);
			});

		ImGui::PopItemWidth();
		ImGui::End();
	}
}
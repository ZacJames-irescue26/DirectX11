#include "pch.h"
#include "Scene.h"
#include "nlohmann\json.hpp"
#include "Components\Components.h"
#include "..\Graphics.h"
#include "src\Utils.h"
#include "Math\AABB.h"
#include "src/Pathfinding/NavMeshSystem.h"
#include "Scripting\ScriptEngine.h"
#include "Project.h"
namespace Engine
{

	void Scene::Stop()
	{
		for (auto& ent : GetEntities())
		{
			if (ent->HasComponent<LuaScriptComponent>())
			{
				auto scriptcomp = ent->GetComponent<LuaScriptComponent>();
				scriptcomp->ResetRuntime();
			}
		}
		ResetPhysics();
		//m_PhysEngine.reset();
		m_IsPlaying = false;
	}

	void Scene::Play()
	{
		if (m_IsPlaying)
			return;

		m_IsPlaying = true;
		auto enginecontext = EngineContext(this, m_PhysEngine.get(), m_SoundSystem.get());
		if (!m_ScriptEngine)
		{
			m_ScriptEngine = std::make_unique<ScriptEngine>(enginecontext);
		}
		if (!m_ScriptEngine->Initialize(enginecontext))
		{
			m_IsPlaying = false;
			return;
		}
		if (!m_SoundSystem)
		{
			m_SoundSystem = std::make_unique<SoundSystem>();
			m_SoundSystem->Initialize();

		}
		for (auto& ent : GetEntities())
		{
			if (!ent)
				continue;

			auto* Audiocomp = ent->GetComponent<AudioComponent>();

			if (!Audiocomp)
				continue;
			m_SoundSystem->CreateSound(Audiocomp->AudioPath, Audiocomp->GetSoundAddress());
			Audiocomp->SetLoopMode();
			Audiocomp->SetMinMaxDistance();
		}
		for (auto& ent : GetEntities())
		{
			if (!ent)
				continue;

			auto* scriptcomp = ent->GetComponent<LuaScriptComponent>();

			if (!scriptcomp)
				continue;

			scriptcomp->ResetRuntime();

			if (m_ScriptEngine->LoadScript(ent.get(), *scriptcomp))
			{
				m_ScriptEngine->CallOnCreate(ent.get(),*scriptcomp);
			}
		}

		CreatePhysics();
		 enginecontext = EngineContext(this, m_PhysEngine.get(), m_SoundSystem.get());
		m_ScriptEngine->SetContext(enginecontext);
	}
	void Scene::PlayAudio(Entity* ent)
	{
		if (!ent)
			return;

		auto* audiocomp = ent->GetComponent<AudioComponent>();

		if (!audiocomp)
			return;

		m_SoundSystem->PlaySound(audiocomp->GetSound(), false, audiocomp->m_Channel);
	}
	void Scene::PlayUpdate(float dt)
	{
		for (auto& ent : GetEntities())
		{
			if (ent->HasComponent<LuaScriptComponent>())
			{
				auto scriptcomp = ent->GetComponent<LuaScriptComponent>();
				m_ScriptEngine->CallOnUpdate(ent.get(), *scriptcomp, dt);
			}
		}
		m_PhysEngine->Update();
		m_SoundSystem->Update();
	}
	void Scene::ResetPhysics()
	{
		m_PhysEngine->UnRegister();
	}
	void Scene::ReloadScript()
	{
		for (auto& ent : GetEntities())
		{
			if (ent->HasComponent<LuaScriptComponent>())
			{
				auto scriptcomp = ent->GetComponent<LuaScriptComponent>();
				m_ScriptEngine->LoadScript(ent.get(), *scriptcomp);
				m_ScriptEngine->CallOnCreate(ent.get(), *scriptcomp);
			}
		}
	}
	std::shared_ptr<Entity> Scene::AddEntity(std::string name)
	{
		UUID id{};
		std::shared_ptr<Entity> entity = std::make_shared<Entity>(name, id, this);
		entity->AddComponent(std::make_unique<TransformComponent>());
		m_Entities.insert({ id, entity });
		m_NameToUUID.insert({ name, id });
		return entity;
	}


	void Scene::DestroyEntity(UUID id)
	{
		auto entity = GetEntityByID(id);
		if (!entity)
			return;
		m_PhysEngine->UnregisterEntity(entity.get());


		m_NameToUUID.erase(m_Entities.find(id)->second->GetName());
		m_Entities.erase(id);

	}

	std::shared_ptr<Entity> Scene::GetEntityByID(UUID id)
	{
		if (m_Entities.find(id) != m_Entities.end())
		{
			return m_Entities.find(id)->second;
		}

		return nullptr;
	}
	std::shared_ptr<Entity> Scene::GetEntityByName(std::string name)
	{
		return m_Entities.find(m_NameToUUID.find(name)->second)->second;
	}
	std::vector<std::shared_ptr<Entity>> Scene::GetEntities()
	{
		std::vector<std::shared_ptr<Entity>> outvec;
		for (const auto& [id, entity] : m_Entities)
		{
			outvec.push_back(entity);
		}
		return outvec;
	}
	void Scene::UpdateChildren()
	{
		for (auto& [id, entity] : m_Entities)
		{
			auto* trans = entity->GetComponent<TransformComponent>();

			if (!trans)
				continue;

			XMMATRIX world = GetWorldMatrix(entity.get());
			
			XMVECTOR outscale, outrotquat, outpos;
			XMMatrixDecompose(&outscale, &outrotquat, &outpos, world);
			
			XMStoreFloat3(&trans->Position, outpos);
			XMStoreFloat3(&trans->Scale, outscale);
			XMFLOAT4 outrot;
			XMStoreFloat4(&outrot, outrotquat);
			trans->ModelMatrix = world;
			trans->Rotation = Utils::QuaternionToEuler(outrot);
			trans->RotationQuat = outrot;
		}
	}
	void Scene::UpdateWorldTransforms()
	{
		for (auto& [id, entity] : m_Entities)
		{
			if (!entity)
				continue;

			auto* transform = entity->GetComponent<TransformComponent>();

			if (!transform)
				continue;

			transform->ModelMatrix = GetWorldMatrix(entity.get());
		}
	}

	void Scene::DrawStaticScene(const XMMATRIX& viewProjectionMatrix)
	{
		for (auto& [id, entity] : m_Entities)
		{
			auto* mesh = entity->GetComponent<StaticMeshComponent>();

			if (!mesh)
				continue;

			XMMATRIX world = GetWorldMatrix(entity.get());

			mesh->Draw(world, viewProjectionMatrix);
		}
	}
	void Scene::DrawAnimatedScene(const XMMATRIX& viewProjectionMatrix)
	{
		for (auto& [id, entity] : m_Entities)
		{
			auto* animMesh = entity->GetComponent<AnimatedMeshComponent>();

			if (!animMesh)
				continue;

			XMMATRIX world = GetWorldMatrix(entity.get());

			animMesh->Draw(world, viewProjectionMatrix);
		}

	}

	void Scene::DrawWithoutCBuffer(ConstantBuffer<ModelOnly>* constantbuffer)
	{
		for (auto& [id, entity] : m_Entities)
		{
			if (!entity->HasComponent<TransformComponent>())
				continue;
			XMMATRIX world = GetWorldMatrix(entity.get());
			constantbuffer->data.Model = XMMatrixTranspose(world);
			constantbuffer->ApplyChanges();
			if (entity->GetComponent<StaticMeshComponent>())
			{
				entity->GetComponent<StaticMeshComponent>()->DrawWithoutCBuffer();
			}
		}
	}
	void Scene::UpdateScene(float deltatime)
	{
		for (auto& [id, entity] : m_Entities)
		{
			if (entity->GetComponent<AnimatedMeshComponent>())
			{
				entity->GetComponent<AnimatedMeshComponent>()->Update(deltatime);
			}
		}


	}
	AABB TransformAABB( AABB& localBounds, const XMMATRIX& world)
	{
		

		XMFLOAT3 corners[8] =
		{
			{ localBounds.Minf().x, localBounds.Minf().y, localBounds.Minf().z },
			{ localBounds.Maxf().x, localBounds.Minf().y, localBounds.Minf().z },
			{ localBounds.Minf().x, localBounds.Maxf().y, localBounds.Minf().z },
			{ localBounds.Maxf().x, localBounds.Maxf().y, localBounds.Minf().z },

			{ localBounds.Minf().x, localBounds.Minf().y, localBounds.Maxf().z },
			{ localBounds.Maxf().x, localBounds.Minf().y, localBounds.Maxf().z },
			{ localBounds.Minf().x, localBounds.Maxf().y, localBounds.Maxf().z },
			{ localBounds.Maxf().x, localBounds.Maxf().y, localBounds.Maxf().z }
		};

		AABB result;
		result.Minf() = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
		result.Maxf() = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		for (int i = 0; i < 8; ++i)
		{
			XMVECTOR c = XMLoadFloat3(&corners[i]);
			XMVECTOR w = XMVector3TransformCoord(c, world);

			XMFLOAT3 p;
			XMStoreFloat3(&p, w);
			result.extend(p);
		}

		return result;
	}

	void Scene::QueueDestroyEntity(UUID uuid)
	{
		m_EntitiesToDestroy.push_back(uuid);
	}

	void Scene::FlushDestroyedEntities()
	{
		for (UUID id : m_EntitiesToDestroy)
		{
			DestroyEntity(id);
		}

		m_EntitiesToDestroy.clear();
	}
	AABB Scene::GetSceneAABB()
	{
		AABB SceneAABB;
		for (auto& [id, entity] : m_Entities)
		{
			if (entity->GetComponent<StaticMeshComponent>() && entity->GetComponent<TransformComponent>())
			{
				AABB entityAABB = entity->GetComponent<StaticMeshComponent>()->m_Model.GetAABB();
				
				SceneAABB = SceneAABB.Combine(SceneAABB, TransformAABB(entityAABB, entity->GetComponent<TransformComponent>()->ModelMatrix));
			}
		}
		return SceneAABB;
	}
	void Scene::SetParent(Entity* child, Entity* parent)
	{

		if (parent && IsDescendant(parent, child))
		{
			// Would create cycle.
			return;
		}

		if (!child)
			return;

		UUID oldParentId = child->GetParent();

		if (oldParentId != 0)
		{
			auto oldParent = GetEntityByID(oldParentId);
			if (oldParent)
				oldParent->RemoveChild(child->GetUUID());
		}

		if (parent)
		{
			child->SetParent(parent->GetUUID());
			parent->AddChild(child->GetUUID());
		}
		else
		{
			child->SetParent(0);
		}
	}
	XMMATRIX Scene::GetWorldMatrix(Entity* entity)
	{
		if (!entity)
			return XMMatrixIdentity();

		auto* transform = entity->GetComponent<TransformComponent>();
		transform->CalculateModelMatrix();

		XMMATRIX local =
			transform
			? transform->ModelMatrix
			: XMMatrixIdentity();

		UUID parentId = entity->GetParent();

		if (parentId == 0)
			return local;

		auto parent = GetEntityByID(parentId);

		if (!parent)
			return local;

		XMMATRIX parentWorld = GetWorldMatrix(parent.get());

		// Row-vector convention:
		return local * parentWorld;
	}

	bool Scene::IsDescendant(Entity* possibleChild, Entity* possibleParent)
	{
		if (!possibleChild || !possibleParent)
			return false;

		UUID parentId = possibleChild->GetParent();

		while (parentId != 0)
		{
			if (parentId == possibleParent->GetUUID())
				return true;

			auto parent = GetEntityByID(parentId);

			if (!parent)
				return false;

			parentId = parent->GetParent();
		}

		return false;
	}

	void Scene::SetParentKeepWorld(Entity* child, Entity* newParent)
	{
		if (!child)
			return;

		auto* transform = child->GetComponent<TransformComponent>();
		if (!transform)
			return;

		XMMATRIX oldWorld = GetWorldMatrix(child);

		SetParent(child, newParent);

		XMMATRIX parentWorld = XMMatrixIdentity();

		if (newParent)
			parentWorld = GetWorldMatrix(newParent);

		XMMATRIX newLocal = oldWorld * XMMatrixInverse(nullptr, parentWorld);

		// You need to decompose back into Position/Rotation/Scale.
		XMVECTOR scale;
		XMVECTOR rotation;
		XMVECTOR translation;

		XMMatrixDecompose(&scale, &rotation, &translation, newLocal);

		XMStoreFloat3(&transform->Scale, scale);
		XMStoreFloat3(&transform->Position, translation);

		XMFLOAT4 q;
		XMStoreFloat4(&q, rotation);
		transform->RotationQuat = q;
		// Convert quaternion to Euler if your TransformComponent stores Euler.
		// Easier long-term: store rotation as quaternion.
	}


	void Scene::CreatePhysics()
	{
		if (!m_PhysEngine)
		{
			m_PhysEngine = std::make_shared<PhysicsEngine>();
		}
		m_PhysEngine->CreateColliders(this);
	}
	
	void Scene::UpdatePhysicsTransforms()
	{
		if (!m_PhysEngine)
			return;
		for (auto& [id, entity] : m_Entities)
		{
			auto transform = entity->GetComponent<TransformComponent>();
			auto rb = entity->GetComponent<PhysicsComponent>();

			if (!transform || !rb)
				continue;

			XMFLOAT3 pos;
			XMFLOAT4 rot;
			m_PhysEngine->GetPosAndRot(rb->m_BodyID, &pos, &rot);
			transform->Position = pos;
			transform->RotationQuat = rot;
			transform->Rotation = Utils::QuaternionToEuler(rot);
			transform->CalculateModelMatrix();
		}
	}
	void Scene::InitializeRuntimeResources(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		ConstantBuffer<CB_Anim_VS_vertexShader>& animCB,
		ConstantBuffer<CB_VS_vertexShader>& staticCB)
	{
		for (auto& [id, entity] : m_Entities)
		{
			if (!entity)
				continue;
			
			if (auto* staticMesh = entity->GetComponent<StaticMeshComponent>())
			{
				
				staticMesh->Initialize(Project::ResolveAssetPath(staticMesh->m_filepath).string(), device, context, &staticCB);

			}


			if (auto* anim = entity->GetComponent<AnimatedMeshComponent>())
			{
				anim->Initialize(Project::ResolveAssetPath(anim->m_filepath).string(), device, context, &animCB);
				for (const auto& [Name,paths] : anim->m_AnimPaths)
				{
					anim->AddAnimation(paths, Name);
				}
			}
		}
	}
	void Scene::SerializeScene(const std::string& path)
	{
		nlohmann::json scene;
		scene["Entities"] = nlohmann::json::array();

		for (const auto& [key, ent] : m_Entities)
		{
			nlohmann::json entityJson;

			entityJson["ID"] = static_cast<unsigned long long>(ent->GetUUID());
			entityJson["Name"] = ent->GetName();
			std::vector<unsigned long long> children;

			for (UUID childID : ent->GetChildren())
			{
				children.push_back(static_cast<unsigned long long>(childID));
			}
			entityJson["Parent"] =
				static_cast<unsigned long long>(ent->GetParent());
			entityJson["Children"] = children;
			nlohmann::json components;

			if (auto* transform = ent->GetComponent<TransformComponent>())
			{
				components["TransformComponent"] = {
					{ "Position", { transform->Position.x, transform->Position.y, transform->Position.z } },
					{ "Rotation", { transform->Rotation.x, transform->Rotation.y, transform->Rotation.z } },
					{ "Scale",    { transform->Scale.x,    transform->Scale.y,    transform->Scale.z    } }
				};
			}

			if (auto* staticMesh = ent->GetComponent<StaticMeshComponent>())
			{
				components["StaticMeshComponent"] = {
					{ "ModelPath", staticMesh->m_filepath }
				};
			}

			if (auto* anim = ent->GetComponent<AnimatedMeshComponent>())
			{
				components["AnimatedMeshComponent"] = {
					{ "AnimatedModelPath", anim->m_filepath },
					{ "Animations", anim->m_AnimPaths },
					{ "PlayAnimation", anim->m_PlayAnimation },
					{ "PlayBackSpeed", anim->m_PlaybackSpeed }
				};
			}

			if (auto* phys = ent->GetComponent<PhysicsComponent>())
			{
				nlohmann::json physJson;

				physJson["RigidBodyType"] = static_cast<int>(phys->RigidBodyType);
				physJson["ShapeType"] = static_cast<int>(phys->ColliderType);
				physJson["Friction"] = phys->friction;
				physJson["Restitution"] = phys->restitution;
				physJson["Awake"] = phys->Awake;
				physJson["Sensor"] = phys->IsSensor;

				switch (phys->ColliderType)
				{
				case JPH::EShapeSubType::Capsule:
					physJson["Radius"] = phys->radius;
					physJson["HalfHeight"] = phys->HalfHeight;
					physJson["ColliderPos"] = {
						phys->ColliderPosition.x,
						phys->ColliderPosition.y,
						phys->ColliderPosition.z
					};
					break;

				case JPH::EShapeSubType::Box:
					physJson["ColliderPos"] = {
						phys->ColliderPosition.x,
						phys->ColliderPosition.y,
						phys->ColliderPosition.z
					};
					physJson["HalfSize"] = {
						phys->HalfSize.x,
						phys->HalfSize.y,
						phys->HalfSize.z
					};
					break;
				}

				components["PhysicsComponent"] = physJson;
			}

			if (auto* script = ent->GetComponent<LuaScriptComponent>())
			{
				components["LuaScriptComponent"] = {
					{ "ScriptPath", script->ScriptPath }
				};
			}
			if (auto* dirlight = ent->GetComponent<DirectionalLightComponent>())
			{
				components["DirectionalLightComponent"] = {
					{ "Intensity", dirlight->Intensity },
					{ "LightSize", dirlight->LightSize },
					{ "Radiance", {
						dirlight->Radiance.x,
						dirlight->Radiance.y,
						dirlight->Radiance.z
					}},
					{ "ShadowAmount", dirlight->ShadowAmount },
					{ "CastShadows", dirlight->CastShadows },
					{ "SoftShadows", dirlight->SoftShadows }
				};
			}
			if (auto* pointLight = ent->GetComponent<PointLightComponent>())
			{
				components["PointLightComponent"] = {
					{ "CastsShadows", pointLight->CastsShadows },
					{ "SoftShadows", pointLight->SoftShadows },
					{ "Falloff", pointLight->Falloff },
					{ "Intensity", pointLight->Intensity },
					{ "LightSize", pointLight->LightSize },
					{ "MinRadius", pointLight->MinRadius },
					{ "Radiance", {
						pointLight->Radiance.x,
						pointLight->Radiance.y,
						pointLight->Radiance.z
					}},
					{ "Radius", pointLight->Radius }
				};
			}
		
			if (auto* spotLight = ent->GetComponent<SpotLightComponent>())
			{
				components["SpotLightComponent"] = {
					{ "Radiance", {
						spotLight->Radiance.x,
						spotLight->Radiance.y,
						spotLight->Radiance.z
					}},
					{ "Intensity", spotLight->Intensity },
					{ "Range", spotLight->Range },
					{ "Angle", spotLight->Angle },
					{ "AngleAttenuation", spotLight->AngleAttenuation },
					{ "CastsShadows", spotLight->CastsShadows },
					{ "SoftShadows", spotLight->SoftShadows },
					{ "Falloff", spotLight->Falloff }
				};
			}


			if (auto* camera = ent->GetComponent<CameraComponent>())
			{
				components["CameraComponent"] = {
					{"FarPlane", camera->FarPlane},
					{"FOVDegrees", camera->FOVDegrees},
					{"NearPlane", camera->NearPlane},
					{"Primary", camera->Primary},
					{"Mode", camera->Mode},
					{"OrthoSize", camera->OrthoSize},
				};
			}

			if (auto* audio = ent->GetComponent<AudioComponent>())
			{
				components["AudioComponent"] = {
					{"AudioPath",audio->AudioPath},
					{"loopMode", audio->loopMode},
					{"minDistance",audio->MinDistance},
					{"maxDistance",audio->MaxDistance }
				};
			}

			entityJson["Components"] = components;

			scene["Entities"].push_back(entityJson);
		

		}
		std::ofstream out(path);

		if (!out.is_open())
		{
			OutputDebugStringA("Failed to open scene file for writing\n");
			return;
		}

		out << scene.dump(4);

	}
	std::shared_ptr<Entity> Scene::CreateEntityWithUUID(UUID id, std::string name)
	{
		std::shared_ptr<Entity> entity = std::make_shared<Entity>(name, id, this);
		entity->AddComponent(std::make_unique<TransformComponent>());
		m_Entities.insert({ id, entity });
		m_NameToUUID.insert({ name, id });
		return entity;
	}

	void Scene::LoadScene(const std::string& path)
	{
		m_Entities.clear();
		m_NameToUUID.clear();
		DeserializeScene(path);
	}

	bool Scene::DeserializeScene(const std::string& path)
	{
		std::ifstream in(path);

		if (!in.is_open())
		{
			OutputDebugStringA("Failed to open scene file for reading\n");
			return false;
		}

		nlohmann::json scene;
		in >> scene;

		// Clear current scene first.
		m_Entities.clear();

		if (!scene.contains("Entities") || !scene["Entities"].is_array())
			return false;
		// Store parent links temporarily.
		std::vector<std::pair<UUID, UUID>> parentLinks;
		for (const auto& entityJson : scene["Entities"])
		{
			UUID id = static_cast<UUID>(
				entityJson.value("ID", 0ull)
				);

			std::string name =
				entityJson.value("Name", std::string("Unnamed Entity"));

			auto ent = CreateEntityWithUUID(id, name);
			UUID parentID = static_cast<UUID>(
				entityJson.value("Parent", 0ull)
				);

			if (parentID != 0)
				parentLinks.push_back({ id, parentID });
			if (!entityJson.contains("Components"))
				continue;

			const auto& components = entityJson["Components"];

			if (components.contains("TransformComponent"))
			{
				const auto& t = components["TransformComponent"];

				auto transform = std::make_unique<TransformComponent>();

				if (t.contains("Position"))
				{
					transform->Position = XMFLOAT3(
						t["Position"][0].get<float>(),
						t["Position"][1].get<float>(),
						t["Position"][2].get<float>()
					);
				}

				if (t.contains("Rotation"))
				{
					transform->Rotation = XMFLOAT3(
						t["Rotation"][0].get<float>(),
						t["Rotation"][1].get<float>(),
						t["Rotation"][2].get<float>()
					);
					transform->SetRotationEulerRadians({
							t["Rotation"][0].get<float>(),
							t["Rotation"][1].get<float>(),
							t["Rotation"][2].get<float>()
						});
				}

				if (t.contains("Scale"))
				{
					transform->Scale = XMFLOAT3(
						t["Scale"][0].get<float>(),
						t["Scale"][1].get<float>(),
						t["Scale"][2].get<float>()
					);
				}

				auto newcomp = ent->GetComponent<TransformComponent>();
				newcomp->Position = transform->Position;
				newcomp->Rotation = transform->Rotation;
				newcomp->Scale = transform->Scale;

				newcomp->CalculateModelMatrix();

			}

			if (components.contains("StaticMeshComponent"))
			{
				const auto& sm = components["StaticMeshComponent"];

				auto staticMesh = std::make_unique<StaticMeshComponent>();
				staticMesh->m_filepath = sm.value("ModelPath", std::string());
				staticMesh->Initialized = false;

				ent->AddComponent(std::move(staticMesh));
			}

			if (components.contains("AnimatedMeshComponent"))
			{
				const auto& animJson = components["AnimatedMeshComponent"];

				auto anim = std::make_unique<AnimatedMeshComponent>();

				anim->m_filepath =
					animJson.value("AnimatedModelPath", std::string());

				if (animJson.contains("Animations"))
				{
					anim->m_AnimPaths =
						animJson["Animations"].get<std::map<std::string, std::string>>();
				}

				anim->m_PlayAnimation =
					animJson.value("PlayAnimation", false);

				anim->m_PlaybackSpeed =
					animJson.value("PlayBackSpeed", 1.0f);

				anim->Initialized = false;

				ent->AddComponent(std::move(anim));
			}

			if (components.contains("PhysicsComponent"))
			{
				const auto& physJson = components["PhysicsComponent"];

				auto phys = std::make_unique<PhysicsComponent>();

				phys->RigidBodyType =
					static_cast<JPH::EMotionType>(
						physJson.value("RigidBodyType", 0)
						);

				phys->ColliderType =
					static_cast<JPH::EShapeSubType>(
						physJson.value("ShapeType", static_cast<int>(JPH::EShapeSubType::Box))
						);

				phys->friction =
					physJson.value("Friction", 0.5f);

				phys->restitution =
					physJson.value("Restitution", 0.0f);

				phys->Awake =
					physJson.value("Awake", true);

				phys->IsSensor =
					physJson.value("Sensor", false);

				if (physJson.contains("ColliderPos"))
				{
					phys->ColliderPosition = XMFLOAT3(
						physJson["ColliderPos"][0].get<float>(),
						physJson["ColliderPos"][1].get<float>(),
						physJson["ColliderPos"][2].get<float>()
					);
				}

				switch (phys->ColliderType)
				{
				case JPH::EShapeSubType::Capsule:
					phys->radius =
						physJson.value("Radius", 0.5f);

					phys->HalfHeight =
						physJson.value("HalfHeight", 1.0f);
					break;

				case JPH::EShapeSubType::Box:
					if (physJson.contains("HalfSize"))
					{
						phys->HalfSize = XMFLOAT3(
							physJson["HalfSize"][0].get<float>(),
							physJson["HalfSize"][1].get<float>(),
							physJson["HalfSize"][2].get<float>()
						);
					}
					break;

				default:
					break;
				}

				// Do not create Jolt body here unless you are entering Play mode.
				ent->AddComponent(std::move(phys));
			}

			if (components.contains("LuaScriptComponent"))
			{
				const auto& scriptJson = components["LuaScriptComponent"];

				auto script = std::make_unique<LuaScriptComponent>();
				script->ScriptPath =
					scriptJson.value("ScriptPath", std::string("Scripts/test.lua"));

				// Do not load script here unless desired.
				// Runtime will load on Play().
				ent->AddComponent(std::move(script));
			}

			if (components.contains("DirectionalLightComponent"))
			{
				const auto& lightJson = components["DirectionalLightComponent"];

				auto light = std::make_unique<DirectionalLightComponent>();

				light->Intensity =
					lightJson.value("Intensity", 1.0f);

				light->LightSize =
					lightJson.value("LightSize", 0.5f);

				light->ShadowAmount =
					lightJson.value("ShadowAmount", 1.0f);

				light->CastShadows =
					lightJson.value("CastShadows", true);

				light->SoftShadows =
					lightJson.value("SoftShadows", true);

				if (lightJson.contains("Radiance"))
				{
					light->Radiance = XMFLOAT3(
						lightJson["Radiance"][0].get<float>(),
						lightJson["Radiance"][1].get<float>(),
						lightJson["Radiance"][2].get<float>()
					);
				}

				ent->AddComponent(std::move(light));
			}

			if (components.contains("PointLightComponent"))
			{
				const auto& lightJson = components["PointLightComponent"];

				auto light = std::make_unique<PointLightComponent>();

				light->CastsShadows =
					lightJson.value("CastsShadows", true);

				light->SoftShadows =
					lightJson.value("SoftShadows", true);

				light->Falloff =
					lightJson.value("Falloff", 1.0f);

				light->Intensity =
					lightJson.value("Intensity", 1.0f);

				light->LightSize =
					lightJson.value("LightSize", 0.5f);

				light->MinRadius =
					lightJson.value("MinRadius", 1.0f);

				light->Radius =
					lightJson.value("Radius", 10.0f);

				if (lightJson.contains("Radiance"))
				{
					light->Radiance = XMFLOAT3(
						lightJson["Radiance"][0].get<float>(),
						lightJson["Radiance"][1].get<float>(),
						lightJson["Radiance"][2].get<float>()
					);
				}

				ent->AddComponent(std::move(light));
			}

			if (components.contains("SpotLightComponent"))
			{
				const auto& lightJson = components["SpotLightComponent"];

				auto light = std::make_unique<SpotLightComponent>();

				light->Intensity =
					lightJson.value("Intensity", 1.0f);

				light->Range =
					lightJson.value("Range", 10.0f);

				light->Angle =
					lightJson.value("Angle", 60.0f);

				light->AngleAttenuation =
					lightJson.value("AngleAttenuation", 5.0f);

				light->CastsShadows =
					lightJson.value("CastsShadows", false);

				light->SoftShadows =
					lightJson.value("SoftShadows", false);

				light->Falloff =
					lightJson.value("Falloff", 1.0f);

				if (lightJson.contains("Radiance"))
				{
					light->Radiance = XMFLOAT3(
						lightJson["Radiance"][0].get<float>(),
						lightJson["Radiance"][1].get<float>(),
						lightJson["Radiance"][2].get<float>()
					);
				}

				ent->AddComponent(std::move(light));
			}


			if (components.contains("CameraComponent"))
			{
				const auto& camJson = components["CameraComponent"];

				auto camera = std::make_unique<CameraComponent>();

				camera->FarPlane = camJson.value("FarPlane", 1000.0f);
				camera->NearPlane = camJson.value("NearPlane", 0.1f);
				camera->FOVDegrees = camJson.value("FOVDegrees", 90.0f);
				camera->Mode = (Engine::CameraMode)camJson.value("Mode", Engine::CameraMode::Perspective);
				camera->OrthoSize = camJson.value("OrthoSize", 10.0f);
				camera->Primary = camJson.value("Primary",true);
				ent->AddComponent(std::move(camera));
			}

		}
		for (const auto& [childID, parentID] : parentLinks)
		{
			auto child = GetEntityByID(childID);
			auto parent = GetEntityByID(parentID);

			if (!child || !parent)
				continue;

			SetParent(child.get(), parent.get());
		}

		
		return true;
	}
	std::unique_ptr<Scene> Scene::Copy()
	{
		auto saveScene = std::make_unique<Scene>();
		for (const auto& [id, ent] : m_Entities)
		{
			saveScene->m_Entities.insert({id, ent->Clone(ent.get())});
		}
		return saveScene;
	}
	Entity* Scene::GetPrimaryCameraEntity()
	{
		for (auto& [id, entity] : m_Entities)
		{
			if (!entity)
				continue;

			auto* camera = entity->GetComponent<CameraComponent>();

			if (camera && camera->Primary)
				return entity.get();
		}

		return nullptr;
	}

	void UpdateCameraFromTransform(
		CameraComponent& camera,
		const TransformComponent& transform,
		float aspectRatio)
	{
		XMVECTOR position = XMLoadFloat3(&transform.Position);

		XMMATRIX rotation =
			XMMatrixRotationRollPitchYaw(
				transform.Rotation.x,
				transform.Rotation.y,
				transform.Rotation.z
			);

		XMVECTOR forward =
			XMVector3TransformNormal(
				XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
				rotation
			);

		XMVECTOR up =
			XMVector3TransformNormal(
				XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
				rotation
			);

		XMVECTOR target = position + forward;

		camera.ViewMatrix =
			XMMatrixLookAtLH(position, target, up);

		if (camera.Mode == CameraMode::Perspective)
		{
			camera.ProjectionMatrix =
				XMMatrixPerspectiveFovLH(
					XMConvertToRadians(camera.FOVDegrees),
					aspectRatio,
					camera.NearPlane,
					camera.FarPlane
				);
		}
		else
		{
			float height = camera.OrthoSize;
			float width = height * aspectRatio;

			camera.ProjectionMatrix =
				XMMatrixOrthographicLH(
					width,
					height,
					camera.NearPlane,
					camera.FarPlane
				);
		}
		XMStoreFloat3(&camera.ForwardVector, forward);
		camera.ViewProjectionMatrix =
			camera.ViewMatrix * camera.ProjectionMatrix;
	}

	XMMATRIX Scene::GetActiveCameraViewProjection(float aspectRatio)
	{
		Entity* cameraEntity = GetPrimaryCameraEntity();

		if (!cameraEntity)
			return XMMatrixIdentity();

		auto* transform = cameraEntity->GetComponent<TransformComponent>();
		auto* camera = cameraEntity->GetComponent<CameraComponent>();

		if (!transform || !camera)
			return XMMatrixIdentity();

		UpdateCameraFromTransform(*camera, *transform, aspectRatio);

		return camera->ViewProjectionMatrix;
	}

	void Scene::UpdateRuntimeCamera(Graphics& gfx)
	{
		Entity* cameraEntity = GetPrimaryCameraEntity();

		if (!cameraEntity)
			return;

		auto* transform = cameraEntity->GetComponent<TransformComponent>();
		auto* camera = cameraEntity->GetComponent<CameraComponent>();

		if (!transform || !camera)
			return;

		XMMATRIX world = GetWorldMatrix(cameraEntity);

		XMVECTOR scale;
		XMVECTOR rotationQuat;
		XMVECTOR position;

		XMMatrixDecompose(&scale, &rotationQuat, &position, world);

		XMMATRIX rotation = XMMatrixRotationQuaternion(rotationQuat);

		XMVECTOR forward = XMVector3TransformNormal(
			XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
			rotation
		);

		XMVECTOR up = XMVector3TransformNormal(
			XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
			rotation
		);

		XMMATRIX view = XMMatrixLookAtLH(
			position,
			position + forward,
			up
		);

		float aspect =
			static_cast<float>(Graphics::windowWidth) /
			static_cast<float>(Graphics::windowHeight);

		gfx.camera.SetProjectionValues(
			camera->FOVDegrees,
			aspect,
			camera->NearPlane,
			camera->FarPlane
		);
		XMStoreFloat3(&camera->ForwardVector, forward);
		XMStoreFloat3(&camera->UpVector, up);
		gfx.camera.SetViewMartix(view);
	}
	void Scene::UpdateListener(float dt)
	{
		for (auto& ent : GetEntities())
		{
			if (!ent)
				continue;

			auto* listener = ent->GetComponent<AudioListenerComponent>();

			if (!listener || !listener->IsListening)
				continue;

			auto* transform = ent->GetComponent<TransformComponent>();

			if (!transform)
				continue;

			XMMATRIX world = GetWorldMatrix(ent.get());

			XMVECTOR scale;
			XMVECTOR rotationQuat;
			XMVECTOR position;

			if (!XMMatrixDecompose(&scale, &rotationQuat, &position, world))
				continue;

			XMMATRIX rotation =
				XMMatrixRotationQuaternion(rotationQuat);

			XMVECTOR forward = XMVector3Normalize(
				XMVector3TransformNormal(
					XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
					rotation
				)
			);

			XMVECTOR up =XMVector3Normalize(
				XMVector3TransformNormal(
					XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
					rotation
				)
			);

			XMFLOAT3 pos;
			XMFLOAT3 fwd;
			XMFLOAT3 upVec;

			XMStoreFloat3(&pos, position);
			XMStoreFloat3(&fwd, forward);
			XMStoreFloat3(&upVec, up);

			XMFLOAT3 velocity = { 0.0f, 0.0f, 0.0f };

			if (auto* rb = ent->GetComponent<PhysicsComponent>())
			{
				velocity = m_PhysEngine->GetLinearVelocity(ent.get());
			}

			m_SoundSystem->SetListenerAttribs(
				pos,
				velocity,
				fwd,
				upVec
			);

			// Usually only one active listener.
			break;
		}
	}

	void Scene::CreateNavMesh()
	{
		if (!m_NavMeshSystem )
		{
			m_NavMeshSystem = std::make_unique<Engine::NavMeshSystem>();
		}
		m_NavMeshSystem->BuildFromScene(*this);

	}
	void Scene::DrawNavMesh()
	{
		if (!m_NavMeshSystem )
		{
			m_NavMeshSystem = std::make_unique<Engine::NavMeshSystem>();
		}
		if (m_PhysEngine)
		{
			m_PhysEngine->DebugDraw();
		}
		m_NavMeshSystem->DebugDraw();
	}
	float GetColliderBottomOffset(
		const PhysicsComponent& physics)
	{
		switch (physics.ColliderType)
		{
		case JPH::EShapeSubType::Capsule:
			return physics.HalfHeight + physics.radius;

		case JPH::EShapeSubType::Box:
			return physics.HalfSize.y;

		case JPH::EShapeSubType::Sphere:
			return physics.radius;

		default:
			return 0.0f;
		}
	}

	XMFLOAT3 Scene::BodyPositionToNavPosition(
		const XMFLOAT3& bodyCenter,
		const PhysicsComponent& physics) const
	{
		XMFLOAT3 result = bodyCenter;
		result.y -= GetColliderBottomOffset(physics);
		return result;
	}

	XMFLOAT3 Scene::NavPositionToBodyPosition(
		const XMFLOAT3& navPosition,
		const PhysicsComponent& physics) const
	{
		XMFLOAT3 result = navPosition;
		result.y += GetColliderBottomOffset(physics);
		return result;
	}
	void Scene::UpdateAgents(float dt)
	{

		if (!m_NavMeshSystem || !m_PhysEngine)
			return;

		for (auto& ent : GetEntities())
		{
			if (!ent)
				continue;

			auto* patrol =
				ent->GetComponent<PatrolAgentComponent>();

			auto* transform =
				ent->GetComponent<TransformComponent>();

			if (!patrol || !transform)
				continue;

			auto* physics =
				ent->GetComponent<PhysicsComponent>();

			// This is the position used for navmesh queries.
			// Ideally this should represent the agent's feet.
			XMFLOAT3 navPosition = transform->Position;

			if (physics && !physics->m_BodyID.IsInvalid())
			{
				const JPH::RVec3 bodyCenter =
					m_PhysEngine->Get()->GetBodyInterface()
					.GetPosition(physics->m_BodyID);

				// Convert collider center to feet/navmesh position.
				navPosition =
					BodyPositionToNavPosition(
						PhysicsEngine::FromJoltVector(bodyCenter),
						*physics
					);
			}

			if (!patrol->HasPath)
			{
				patrol->WaitTimer -= dt;

				if (patrol->WaitTimer > 0.0f)
					continue;

				XMFLOAT3 randomDestination = {};

				if (!m_NavMeshSystem->FindRandomPointAround(
					navPosition,
					patrol->PatrolRadius,
					randomDestination))
				{
					continue;
				}

				patrol->Path.clear();

				if (!m_NavMeshSystem->FindPath(
					navPosition,
					randomDestination,
					patrol->Path) ||
					patrol->Path.size() < 2)
				{
					continue;
				}

				patrol->HasPath = true;
				patrol->CurrentPathIndex = 1;
			}

			if (!patrol->HasPath)
				continue;

			if (patrol->CurrentPathIndex >= patrol->Path.size())
			{
				patrol->HasPath = false;
				patrol->WaitTimer = patrol->WaitTime;
				continue;
			}

			const XMFLOAT3 target =
				patrol->Path[patrol->CurrentPathIndex];

			// Navigation controls horizontal movement.
			XMFLOAT3 horizontalDelta =
			{
				target.x - navPosition.x,
				0.0f,
				target.z - navPosition.z
			};

			const float horizontalDistance =
				std::sqrt(
					horizontalDelta.x * horizontalDelta.x +
					horizontalDelta.z * horizontalDelta.z
				);

			if (horizontalDistance <= patrol->StoppingDistance)
			{
				++patrol->CurrentPathIndex;

				if (patrol->CurrentPathIndex >= patrol->Path.size())
				{
					patrol->HasPath = false;
					patrol->WaitTimer = patrol->WaitTime;

					// Stop horizontal movement at the final point.
					if (physics && !physics->m_BodyID.IsInvalid())
					{
						const JPH::Vec3 currentVelocity =
							m_PhysEngine->Get()->GetBodyInterface()
							.GetLinearVelocity(physics->m_BodyID);

						m_PhysEngine->Get()->GetBodyInterface()
							.SetLinearVelocity(
								physics->m_BodyID,
								JPH::Vec3(
									0.0f,
									currentVelocity.GetY(),
									0.0f
								)
							);
					}
				}

				continue;
			}

			const float inverseDistance =
				1.0f / horizontalDistance;

			XMFLOAT3 direction =
			{
				horizontalDelta.x * inverseDistance,
				0.0f,
				horizontalDelta.z * inverseDistance
			};

			const XMFLOAT3 horizontalVelocity =
			{
				direction.x * patrol->Speed,
				0.0f,
				direction.z * patrol->Speed
			};

			if (physics && !physics->m_BodyID.IsInvalid())
			{
				JPH::BodyInterface& bodyInterface =
					m_PhysEngine->Get()->GetBodyInterface();

				const JPH::Vec3 currentVelocity =
					bodyInterface.GetLinearVelocity(
						physics->m_BodyID
					);

				// Preserve gravity, falling and jumping velocity.
				bodyInterface.SetLinearVelocity(
					physics->m_BodyID,
					JPH::Vec3(
						horizontalVelocity.x,
						currentVelocity.GetY(),
						horizontalVelocity.z
					)
				);

				// Rotate the physics body as well as the render transform.
				const float yaw =
					std::atan2(
						direction.x,
						direction.z
					);

				bodyInterface.SetRotation(
					physics->m_BodyID,
					JPH::Quat::sRotation(
						JPH::Vec3::sAxisY(),
						yaw
					),
					JPH::EActivation::Activate
				);

				transform->SetRotationEulerRadians(
					XMFLOAT3(0.0f, yaw, 0.0f)
				);
			}
			else
			{
				transform->Position.x +=
					horizontalVelocity.x * dt;

				transform->Position.z +=
					horizontalVelocity.z * dt;

				const float yaw =
					std::atan2(
						direction.x,
						direction.z
					);

				transform->SetRotationEulerRadians(
					XMFLOAT3(0.0f, yaw, 0.0f)
				);
			}
		}
	}


}
#include "pch.h"
#include "System.h"
#include "Scene\Project.h"
#include "Channel.h"

namespace Engine
{

	SoundSystem::~SoundSystem()
	{
		m_System->close();
		m_System->release();
	}

	bool SoundSystem::Initialize()
	{
		FMOD::System_Create(&m_System);
		m_System->init(100, FMOD_INIT_NORMAL, nullptr);
		return true;
	}

	void SoundSystem::CreateSound(const std::string& filepath, FMOD::Sound** inSound)
	{
		m_System->createSound(Project::ResolveAssetPath(filepath).string().c_str(), FMOD_3D, 0, inSound);
	}

	void SoundSystem::PlaySound(FMOD::Sound* sound, bool Paused, Channel outchannel)
	{
		if (sound && m_System)
		{
			auto channel = outchannel.GetChannel();
			m_System->playSound(sound, 0, Paused, &channel);
		}
	}
	void SoundSystem::Update()
	{
		m_System->update();
	}

	void SoundSystem::DestroySounds(Entity* entity)
	{
		if(!entity)
			return;
		auto* audio = entity->GetComponent<AudioComponent>();
		if(!audio)
			return;
		


	}
}
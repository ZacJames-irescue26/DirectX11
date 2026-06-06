#include "pch.h"
#include "Channel.h"
 
namespace Engine
{

	Channel::~Channel()
	{
		m_Channel->stop();
	}

	void Channel::Set3DAttrib(XMFLOAT3 pos, XMFLOAT3 vel)
	{
		FMOD_VECTOR fmodPos = FMOD_VECTOR(pos.x, pos.y, pos.z);
		FMOD_VECTOR fmodVel = FMOD_VECTOR(vel.x, vel.y, vel.z);

		m_Channel->set3DAttributes(&fmodPos, &fmodVel);
	}

	void Channel::SetPaused(bool isPaused)
	{
		m_Channel->setPaused(isPaused);
	}

}
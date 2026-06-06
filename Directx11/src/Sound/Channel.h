#pragma once
#include "fmod.hpp"
namespace Engine
{
	class Channel
	{
	public:
		~Channel();
		void Set3DAttrib(XMFLOAT3 pos, XMFLOAT3 vel);
		void SetPaused(bool isPaused);
		FMOD::Channel* GetChannel()
		{
			return m_Channel;
		}
		FMOD::Channel** GetChannelAddress()
		{
			return &m_Channel;
		}
		FMOD::Channel* Get()
		{
			return m_Channel;
		}
	private:
		FMOD::Channel* m_Channel;
	};
}
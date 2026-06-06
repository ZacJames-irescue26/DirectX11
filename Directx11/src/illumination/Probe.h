#pragma once


namespace Engine
{
	struct Probe
	{
		XMFLOAT3 Position;
		float CaptureRadius;
		
		XMFLOAT3 AmbientCube0;
		float Pad0;

		XMFLOAT3 AmbientCube1;
		float Pad1;

		XMFLOAT3 AmbientCube2;
		float Pad2;

		XMFLOAT3 AmbientCube3;
		float Pad3;

		XMFLOAT3 AmbientCube4;
		float Pad4;

		XMFLOAT3 AmbientCube5;
		float Pad5;
	};
	
	struct ProbeVolumeDesc
	{
		XMFLOAT3 Min;
		XMFLOAT3 Max;

		float Spacing;
		float CaptureRadius;
	};
	inline std::vector<Probe> CreateProbeGrid(const ProbeVolumeDesc& desc)
	{
		std::vector<Probe> probes;

		for (float z = desc.Min.z; z <= desc.Max.z; z += desc.Spacing)
		{
			for (float y = desc.Min.y; y <= desc.Max.y; y += desc.Spacing)
			{
				for (float x = desc.Min.x; x <= desc.Max.x; x += desc.Spacing)
				{
					Probe p = {};
					p.Position = XMFLOAT3(x, y, z);
					p.CaptureRadius = desc.CaptureRadius;

					p.AmbientCube0 = XMFLOAT3(0, 0, 0);
					p.AmbientCube1 = XMFLOAT3(0, 0, 0);
					p.AmbientCube2 = XMFLOAT3(0, 0, 0);
					p.AmbientCube3 = XMFLOAT3(0, 0, 0);
					p.AmbientCube4 = XMFLOAT3(0, 0, 0);
					p.AmbientCube5 = XMFLOAT3(0, 0, 0);

					probes.push_back(p);
				}
			}
		}

		return probes;
	}
}
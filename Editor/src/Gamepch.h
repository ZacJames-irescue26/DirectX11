#pragma once
#define JPH_EXTERNAL_PROFILE
#include <windows.h>
#include <comdef.h>

// DirectX includes
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include<wrl.h>



// STL includes
#include <intsafe.h>
#include <iostream>
#include <string>
#include <cassert>
#include <ctime>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <exception>
#include <queue>
#include <bitset>
#include <optional>
#include <chrono>
#include <map>
#include <cmath>
#include <thread>
//// Link library dependencies
//#pragma comment(lib, "d3d11.lib")
//#pragma comment(lib, "dxgi.lib")
//#pragma comment(lib, "d3dcompiler.lib")
//#pragma comment(lib, "winmm.lib")

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "stb_image.h"

#include <Jolt/Jolt.h>

// All Jolt symbols are in the JPH namespace
using namespace JPH;

// If you want your code to compile using single or double precision write 0.0_r to get a Real value that compiles to double or float depending if JPH_DOUBLE_PRECISION is set or not.
using namespace JPH::literals;

using namespace DirectX;
// ======================================================================== //
// Copyright 2018-2019 Ingo Wald                                            //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "cuda.h"
#include "cudaD3D11.h"
#include "cuda_d3d11_interop.h"
// our own classes, partly shared between host and device
#include "CUDABuffer.h"
#include "LaunchParams.h"
#include "../ModelSimple.h"
#include "src/Game/GameObject.h"
#include "../Graphics.h"
#include "optix.h"
#include <vector>
/*! \namespace osc - Optix Siggraph Course */

namespace osc {


	struct Camera {
        Camera(vec3f From, vec3f At, vec3f Up)
        : from(From), at(At), up(Up)
        { }
        Camera() {}
		/*! camera position - *from* where we are looking */
		vec3f from;
		/*! which point we are looking *at* */
		vec3f at;
		/*! general up-vector */
		vec3f up;
	};
    /*! a sample OptiX-7 renderer that demonstrates how to set up
        context, module, programs, pipeline, SBT, etc, and perform a
        valid launch that renders some pixel (using a simple test
        pattern, in this case */
    class SampleRenderer
    {
        // ------------------------------------------------------------------
        // publicly accessible interface
        // ------------------------------------------------------------------
    public:
        /*! constructor - performs all setup, including initializing
          optix, creates module, pipeline, programs, SBT, etc. */
        SampleRenderer(Microsoft::WRL::ComPtr < ID3D11Device>& dev, Microsoft::WRL::ComPtr < ID3D11DeviceContext>& devcontext, Microsoft::WRL::ComPtr<ID3D11Texture2D>& textureToLink, std::vector<Engine::GameObject> objects);

        
        /*! render one frame */
        void render();

        static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> irrSRV;
    protected:
        // ------------------------------------------------------------------
        // internal helper functions
        // ------------------------------------------------------------------

        /*! helper function that initializes optix and checks for errors */
        void initOptix();

        /*! creates and configures a optix device context (in this simple
          example, only for the primary GPU device) */
        void createContext();


        
        OptixTraversableHandle buildAccel(std::vector<Engine::GameObject> objects);
        /*! creates the module that contains all the programs we are going
          to use. in this simple example, we use a single module from a
          single .cu file, using a single embedded ptx string */
        void createModule();

        /*! does all setup for the raygen program(s) we are going to use */
        void createRaygenPrograms();

        /*! does all setup for the miss program(s) we are going to use */
        void createMissPrograms();

        /*! does all setup for the hitgroup program(s) we are going to use */
        void createHitgroupPrograms();

        /*! assembles the full pipeline of all programs */
        void createPipeline();

        /*! constructs the shader binding table */
        void buildSBT();

    protected:

		ID3D11Device* d3dDevice = nullptr;
		ID3D11DeviceContext* d3dContext = nullptr;
		// shareable atlas
		ID3D11Texture2D* irrTexDX = nullptr;
		cudaGraphicsResource*   irrResCUDA = nullptr;
		cudaSurfaceObject_t  irrSurfCuda = 0;
       
        bool IsCreated = false;
        bool IsExported = false;
        std::unique_ptr<Engine::Model> model;
        std::vector<Engine::Vertex> CompleteModel;

        std::vector<uint32_t> CompleteIndex;
		std::vector<CUDABuffer> vertexBuffer;
		std::vector<CUDABuffer> normalBuffer;
		std::vector<CUDABuffer> texcoordBuffer;
		std::vector<CUDABuffer> indexBuffer;
		
		std::vector<OptixTraversableHandle> blasHandles;
		std::vector<CUDABuffer> blasBuffers; // Store compacted memory for lifetime
        //! buffer that keeps the (final, compacted) accel structure
		CUDABuffer asBuffer;


	    unsigned int SCREEN_WIDTH = 16;
	    unsigned int SCREEN_HEIGHT = 16;

        /*! @{ CUDA device context and stream that optix pipeline will run
            on, as well as device properties for this device */
        CUcontext          cudaContext;
        CUstream           stream;
        cudaDeviceProp     deviceProps;
        /*! @} */

        //! the optix context that our pipeline will run in.
        OptixDeviceContext optixContext;

        /*! @{ the pipeline we're building */
        OptixPipeline               pipeline;
        OptixPipelineCompileOptions pipelineCompileOptions = {};
        OptixPipelineLinkOptions    pipelineLinkOptions = {};
        /*! @} */
        OptixPipeline pipelinePack;
		OptixPipelineCompileOptions pipelineCompileOptions1 = {};
		OptixPipelineLinkOptions    pipelineLinkOptions1 = {};
        /*! @{ the module that contains out device programs */
		OptixModule                 module;
		OptixModule                 Packmodule;
        OptixModuleCompileOptions   moduleCompileOptions = {};
        OptixModuleCompileOptions   moduleCompileOptions1 = {};
        /* @} */

        /*! vector of all our program(group)s, and the SBT built around
            them */
        std::vector<OptixProgramGroup> raygenPGs;
        CUDABuffer raygenRecordsBuffer;
        std::vector<OptixProgramGroup> missPGs;
        CUDABuffer missRecordsBuffer;
        std::vector<OptixProgramGroup> hitgroupPGs;
        CUDABuffer hitgroupRecordsBuffer;
        OptixShaderBindingTable sbt = {};
        OptixShaderBindingTable sbtPack = {};
        /*! @{ our launch parameters, on the host, and the buffer to store
            them on the device */
        LaunchParams launchParams;
        CUDABuffer   launchParamsBuffer;
        /*! @} */

		CUDABuffer colorBuffer;
		CUDABuffer LightMapBuffer;
        cudaGraphicsResource* cudaLightmapResource;

        std::vector<uint32_t> pixels; 
        std::vector<uint32_t> LightMap;


		/*! the camera we are to render with. */
		Camera lastSetCamera;

		/*! @{ one texture object and pixel array per used texture */

		std::vector<cudaArray_t>         textureArrays;
		std::vector<cudaTextureObject_t> textureObjects;

        std::vector<vec3f> ProbePositions;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> irrTex;
        Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> irrUAV;
        CUDABuffer probePosBuffer;
        CUDABuffer          irrAccumBuffer;
        void* cudaLinearMemory;
        size_t pitch;
    };

} // ::osc

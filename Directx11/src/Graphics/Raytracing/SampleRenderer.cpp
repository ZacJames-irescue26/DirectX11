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
#include "pch.h"



#include <iostream>
#include "SampleRenderer.h"
// this include may only appear in a single source file:
#include <optix_function_table_definition.h>
#include "../ModelSimple.h"
#include "stb_image_write.h"
#include "Game/GameObject.h"

/*! \namespace osc - Optix Siggraph Course */




namespace osc {

	
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SampleRenderer::irrSRV = nullptr;

    extern "C" char imageBytes[];
	extern "C" char imageBytesPack[];
	/*! SBT record for a raygen program */
	struct __align__(OPTIX_SBT_RECORD_ALIGNMENT) RaygenRecord
	{
		__align__(OPTIX_SBT_RECORD_ALIGNMENT) char header[OPTIX_SBT_RECORD_HEADER_SIZE];
		// just a dummy value - later examples will use more interesting
		// data here
		void* data;
	};

	/*! SBT record for a miss program */
	struct __align__(OPTIX_SBT_RECORD_ALIGNMENT) MissRecord
	{
		__align__(OPTIX_SBT_RECORD_ALIGNMENT) char header[OPTIX_SBT_RECORD_HEADER_SIZE];
		// just a dummy value - later examples will use more interesting
		// data here
		void* data;
	};

	/*! SBT record for a hitgroup program */
	struct __align__(OPTIX_SBT_RECORD_ALIGNMENT) HitgroupRecord
	{
		__align__(OPTIX_SBT_RECORD_ALIGNMENT) char header[OPTIX_SBT_RECORD_HEADER_SIZE];
		TriangleMeshSBTData data;
	};
	
    /*! constructor - performs all setup, including initializing
      optix, creates module, pipeline, programs, SBT, etc. */
    SampleRenderer::SampleRenderer(Microsoft::WRL::ComPtr < ID3D11Device>& dev, 
	Microsoft::WRL::ComPtr < ID3D11DeviceContext>& devcontext, 
	Microsoft::WRL::ComPtr<ID3D11Texture2D>& textureToLink, 
	std::vector<Engine::GameObject> objects)
    {

        initOptix();
		model = std::make_unique<Engine::Model>(objects[0].model);
		//launchParams.light.origin = {-1000.00000, 1000.000000, 1000.000000 };
		//launchParams.light.du = {400.000000, 400.00000000, 400.00000000};
		//launchParams.light.dv = {400.00000000, 400.00000000, 400.000000};
		//launchParams.light.power = {0.0,0.0,0.0};
		




        std::cout << "#osc: creating optix context ..." << std::endl;
        createContext();

        std::cout << "#osc: setting up module ..." << std::endl;
        createModule();

        std::cout << "#osc: creating raygen programs ..." << std::endl;
        createRaygenPrograms();
        std::cout << "#osc: creating miss programs ..." << std::endl;
        createMissPrograms();
        std::cout << "#osc: creating hitgroup programs ..." << std::endl;
        createHitgroupPrograms();

        launchParams.tlas = buildAccel(objects);
		float ProbeSpacingX = 1.0;
		float 	ProbeSpacingY = 2.5;      
		float 	ProbeSpacingZ = 1.0;

		for (int x = -5; x < 5; x++)
		{
			for (int y = 0; y < 5; y++)
			{
				for (int z = 0; z < 5; z++)
				{
					ProbePositions.push_back({x*ProbeSpacingX,y*ProbeSpacingY,z*ProbeSpacingZ});
				}
			}
		}



		const size_t texels = ProbePositions.size() * 6 /*faces*/ * 36 /*6×6*/;
		irrAccumBuffer.alloc(texels * sizeof(vec4f));   // one-time allocation
		launchParams.probeCount = ProbePositions.size();
		launchParams.texels = static_cast<uint32_t>(ProbePositions.size() * 6 * 36);
		                             
		probePosBuffer.alloc_and_upload(ProbePositions);       // malloc + memcpy
		launchParams.probePos = reinterpret_cast<vec3f*>(probePosBuffer.d_pointer());
		launchParams.irrAccum = reinterpret_cast<vec4f*>(irrAccumBuffer.d_pointer());
		// 1) after you have filled ProbePositions …
		const UINT probeCount = static_cast<UINT>(ProbePositions.size());   // N?probes
		const UINT facesPerProbe = 6;
		const UINT faceRes = 6;                       // 6×6 texels per face
		const UINT arraySize = probeCount * facesPerProbe;   // one slice = one face
		//--------------------------------------------------------------------
 // 2) 2-D array texture (CUDA/OptiX writes R32_UINT)
 //--------------------------------------------------------------------
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = faceRes;
		texDesc.Height = faceRes;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = arraySize;                    //  derived – never hard-code
		texDesc.Format = DXGI_FORMAT_R32_UINT;         // 32-bit uint  (CUDA OK)
		texDesc.SampleDesc = { 1, 0 };                     // no MSAA
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE |  // we’ll read it in shaders
			D3D11_BIND_UNORDERED_ACCESS;  // OptiX/CUDA write via UAV
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;                            // same-process, no sharing
		
		dev->CreateTexture2D(&texDesc, nullptr, irrTex.GetAddressOf());

		//--------------------------------------------------------------------
		// 3) UAV     (OptiX writes through CUDA surface  UAV binding not used
		//             by D3D11 itself but required for REGISTER_RESOURCE)
		//--------------------------------------------------------------------
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = texDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc.Texture2DArray.MipSlice = 0;
		uavDesc.Texture2DArray.FirstArraySlice = 0;
		uavDesc.Texture2DArray.ArraySize = arraySize;
		
		dev->CreateUnorderedAccessView(irrTex.Get(), &uavDesc, &irrUAV);

		//--------------------------------------------------------------------
		// 4) SRV     (show one particular face in ImGui or the debugger)
		//--------------------------------------------------------------------
		UINT probeToShow = 100;          // choose a probe 0 probeCount-1
		UINT faceToShow = 0;            // 0=+X,1=?X,2=+Y,3=?Y,4=+Z,5=?Z
		UINT slice = probeToShow * facesPerProbe + faceToShow;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.FirstArraySlice = 0;      // just that slice
		srvDesc.Texture2DArray.ArraySize = arraySize;
		
		dev->CreateShaderResourceView(irrTex.Get(), &srvDesc, irrSRV.GetAddressOf());

		//--------------------------------------------------------------------
		// 5) CUDA / OptiX registration  (once, after texture creation)
		//--------------------------------------------------------------------
		
		CUDA_CHECK(GraphicsD3D11RegisterResource(
			&irrResCUDA,
			irrTex.Get(),                        // the texture
			CU_GRAPHICS_REGISTER_FLAGS_SURFACE_LDST));

	   std::cout << "#osc: setting up optix pipeline ..." << std::endl;
        createPipeline();

        std::cout << "#osc: building SBT ..." << std::endl;
        buildSBT();

        launchParamsBuffer.alloc(sizeof(LaunchParams));
        std::cout << "#osc: context, module, pipeline, etc, all set up ..." << std::endl;

       
        std::cout << "#osc: Optix 7 Sample fully set up" << std::endl;
		//for (int i = 0; i < launchParams.texels; i++)
		//{
		//	unsigned flat = i;
		//	unsigned probe = flat / (6u * 36u);
		//	unsigned face = (flat / 36u) % 6u;
		//	unsigned texel = flat % 36u;
		//	unsigned u = texel % 6u;
		//	unsigned v = texel / 6u;
		//	unsigned layer = probe * 6u + face;
		//	unsigned idx = layer * 36u + v * 6u + u;

		//	std::cout << "layer: " << layer << " idx: "  << idx << " u: " << u << " v " << v << std::endl;

		//}
    }

    
    /*! helper function that initializes optix and checks for errors */
    void SampleRenderer::initOptix()
    {
        std::cout << "#osc: initializing optix..." << std::endl;

        // -------------------------------------------------------
        // check for available optix7 capable devices
        // -------------------------------------------------------
        cudaFree(0);
        int numDevices;
        cudaGetDeviceCount(&numDevices);
        if (numDevices == 0)
            throw std::runtime_error("#osc: no CUDA capable devices found!");
        std::cout << "#osc: found " << numDevices << " CUDA devices" << std::endl;

        // -------------------------------------------------------
        // initialize optix
        // -------------------------------------------------------
        OPTIX_CHECK(optixInit());
        std::cout
            << "#osc: successfully initialized optix... yay!"
            << std::endl;
    }

    static void context_log_cb(unsigned int level,
        const char* tag,
        const char* message,
        void*)
    {
        fprintf(stderr, "[%2d][%12s]: %s\n", (int)level, tag, message);
    }

    /*! creates and configures a optix device context (in this simple
        example, only for the primary GPU device) */
    void SampleRenderer::createContext()
    {
        // for this sample, do everything on one device
        const int deviceID = 0;
        CUDA_CHECK(SetDevice(deviceID));
        CUDA_CHECK(StreamCreate(&stream));

        cudaGetDeviceProperties(&deviceProps, deviceID);
        std::cout << "#osc: running on device: " << deviceProps.name << std::endl;

        CUresult  cuRes = cuCtxGetCurrent(&cudaContext);
        if (cuRes != CUDA_SUCCESS)
            fprintf(stderr, "Error querying current context: error code %d\n", cuRes);

        OPTIX_CHECK(optixDeviceContextCreate(cudaContext, 0, &optixContext));
        OPTIX_CHECK(optixDeviceContextSetLogCallback
        (optixContext, context_log_cb, nullptr, 4));
    }
	
	OptixTraversableHandle SampleRenderer::buildAccel(std::vector<Engine::GameObject> objects)
	{

		vertexBuffer.resize(model->GetMeshes().size());
		normalBuffer.resize(model->GetMeshes().size());
		texcoordBuffer.resize(model->GetMeshes().size());
		indexBuffer.resize(model->GetMeshes().size());

		OptixTraversableHandle asHandle{ 0 };

		// ==================================================================
		// triangle inputs
		// ==================================================================
		
		std::vector<OptixInstance> instances;
		for (auto& obj : objects)
		{
			assert(objects.size() == 1 && "Only supports one model at the moment");
			OptixBuildInput triangleInput;
			CUdeviceptr  d_vertices;
			CUdeviceptr  d_indices;
			uint32_t  triangleInputFlags;

			for (int meshID = 0; meshID < obj.model.GetMeshes().size(); meshID++) {
				// upload the model to the device: the builder
			
				Engine::Mesh* mesh = &obj.model.GetMeshes()[meshID];
			
		
				vertexBuffer[meshID].alloc_and_upload(mesh->vertices);
				indexBuffer[meshID].alloc_and_upload(mesh->indices);
				std::vector<XMFLOAT3> Normals;
				std::vector<XMFLOAT2> Texcoords;
				for (const auto& normal: mesh->vertices)
				{

					Normals.push_back(normal.normal);
				}

				for (const auto& tex : mesh->vertices)
				{

					Texcoords.push_back(tex.texCoord);
				}
				normalBuffer[meshID].alloc_and_upload(Normals);
				texcoordBuffer[meshID].alloc_and_upload(Texcoords);
				triangleInput = {};
				triangleInput.type
					= OPTIX_BUILD_INPUT_TYPE_TRIANGLES;

				// create local variables, because we need a *pointer* to the
				// device pointers
				d_vertices = vertexBuffer[meshID].d_pointer();
				d_indices = indexBuffer[meshID].d_pointer();

				triangleInput.triangleArray.vertexFormat = OPTIX_VERTEX_FORMAT_FLOAT3;
				triangleInput.triangleArray.vertexStrideInBytes = sizeof(Engine::Vertex);
				triangleInput.triangleArray.numVertices = (int)mesh->vertices.size();
				triangleInput.triangleArray.vertexBuffers = &d_vertices;
				triangleInput.triangleArray.indexFormat = OPTIX_INDICES_FORMAT_UNSIGNED_INT3;
				triangleInput.triangleArray.indexStrideInBytes = sizeof(DWORD) * 3;
				triangleInput.triangleArray.numIndexTriplets = (int)mesh->indices.size()/3;
				triangleInput.triangleArray.indexBuffer = d_indices;

				triangleInputFlags = 0;

				// in this example we have one SBT entry, and no per-primitive
				// materials:
				triangleInput.triangleArray.flags = &triangleInputFlags;
				triangleInput.triangleArray.numSbtRecords = 1;
				triangleInput.triangleArray.sbtIndexOffsetBuffer = 0;
				triangleInput.triangleArray.sbtIndexOffsetSizeInBytes = 0;
				triangleInput.triangleArray.sbtIndexOffsetStrideInBytes = 0;
				// ==================================================================
				// BLAS setup
				// ==================================================================

				OptixAccelBuildOptions accelOptions = {};
				accelOptions.buildFlags = OPTIX_BUILD_FLAG_NONE
					| OPTIX_BUILD_FLAG_ALLOW_COMPACTION
					;
				accelOptions.motionOptions.numKeys = 1;
				accelOptions.operation = OPTIX_BUILD_OPERATION_BUILD;

				OptixAccelBufferSizes blasBufferSizes;
				OPTIX_CHECK(optixAccelComputeMemoryUsage
				(optixContext,
					&accelOptions,
					&triangleInput,
					1,  // num_build_inputs
					&blasBufferSizes
				));

				// ==================================================================
				// prepare compaction
				// ==================================================================

				CUDABuffer compactedSizeBuffer;
				compactedSizeBuffer.alloc(sizeof(uint64_t));

				OptixAccelEmitDesc emitDesc;
				emitDesc.type = OPTIX_PROPERTY_TYPE_COMPACTED_SIZE;
				emitDesc.result = compactedSizeBuffer.d_pointer();

				// ==================================================================
				// execute build (main stage)
				// ==================================================================

				CUDABuffer tempBuffer;
				tempBuffer.alloc(blasBufferSizes.tempSizeInBytes);

				CUDABuffer outputBuffer;
				outputBuffer.alloc(blasBufferSizes.outputSizeInBytes);

				OPTIX_CHECK(optixAccelBuild(optixContext,
					/* stream */0,
					&accelOptions,
					&triangleInput,
					(int)1,
					tempBuffer.d_pointer(),
					tempBuffer.sizeInBytes,

					outputBuffer.d_pointer(),
					outputBuffer.sizeInBytes,

					&asHandle,

					&emitDesc, 1
				));
				CUDA_SYNC_CHECK();

				// ==================================================================
				// perform compaction
				// ==================================================================
				uint64_t compactedSize;
				compactedSizeBuffer.download(&compactedSize, 1);
				CUDABuffer blasCompact;
				blasCompact.alloc(compactedSize);
				OPTIX_CHECK(optixAccelCompact(optixContext,
					/*stream:*/0,
					asHandle,
					blasCompact.d_pointer(),
					blasCompact.sizeInBytes,
					&asHandle));
				CUDA_SYNC_CHECK();

				blasHandles.push_back(asHandle);
				blasBuffers.push_back(std::move(blasCompact));

				outputBuffer.free(); // << the UNcompacted, temporary output buffer
				tempBuffer.free();
				compactedSizeBuffer.free();

		// ==================================================================
		// aaaaaand .... clean up
		// ==================================================================

			}
				
				for (int i = 0; i < blasHandles.size(); ++i) {
					OptixInstance instance = {};
					float transform[12] = {
						1, 0, 0, 0,
						0, 1, 0, 0,
						0, 0, 1, 0
					};
					memcpy(instance.transform, transform, sizeof(float) * 12);

					instance.instanceId = i;
					instance.sbtOffset = i * RAY_TYPE_COUNT; // Can vary if you want per-material SBT entries
					instance.visibilityMask = 255;
					instance.flags = OPTIX_INSTANCE_FLAG_NONE;
					instance.traversableHandle = blasHandles[i];

					instances.push_back(instance);
				}

			CUDABuffer d_instances;
			d_instances.alloc_and_upload(instances);
			OptixBuildInput tlasInput = {};
			tlasInput.type = OPTIX_BUILD_INPUT_TYPE_INSTANCES;
			tlasInput.instanceArray.instances = d_instances.d_pointer();
			tlasInput.instanceArray.numInstances = (uint32_t)instances.size();

			OptixAccelBuildOptions tlasOptions = {};
			tlasOptions.buildFlags = OPTIX_BUILD_FLAG_ALLOW_COMPACTION;
			tlasOptions.operation = OPTIX_BUILD_OPERATION_BUILD;

			OptixAccelBufferSizes tlasSizes;
			optixAccelComputeMemoryUsage(optixContext, &tlasOptions, &tlasInput, 1, &tlasSizes);

			CUDABuffer tlasTemp, tlasOutput, tlasCompactSize;
			tlasTemp.alloc(tlasSizes.tempSizeInBytes);
			tlasOutput.alloc(tlasSizes.outputSizeInBytes);
			tlasCompactSize.alloc(sizeof(uint64_t));

			OptixAccelEmitDesc emitDesc = {};
			emitDesc.type = OPTIX_PROPERTY_TYPE_COMPACTED_SIZE;
			emitDesc.result = tlasCompactSize.d_pointer();

			OptixTraversableHandle tlasHandle = 0;
			optixAccelBuild(
				optixContext, 0,
				&tlasOptions,
				&tlasInput, 1,
				tlasTemp.d_pointer(), tlasTemp.sizeInBytes,
				tlasOutput.d_pointer(), tlasOutput.sizeInBytes,
				&tlasHandle,
				&emitDesc, 1
			);
			CUDA_SYNC_CHECK();

			uint64_t tlasCompactBytes;
			tlasCompactSize.download(&tlasCompactBytes, 1);

			asBuffer.alloc(tlasCompactBytes);
			optixAccelCompact(optixContext, 0, tlasHandle,
				asBuffer.d_pointer(), tlasCompactBytes, &tlasHandle);
			CUDA_SYNC_CHECK();

			// Cleanup
			tlasTemp.free();
			tlasOutput.free();
			tlasCompactSize.free();
			d_instances.free();
			return tlasHandle;
		}
	}

    /*! creates the module that contains all the programs we are going
        to use. in this simple example, we use a single module from a
        single .cu file, using a single embedded ptx string */
    void SampleRenderer::createModule()
    {
        moduleCompileOptions.maxRegisterCount = 0;
        moduleCompileOptions.optLevel = OPTIX_COMPILE_OPTIMIZATION_DEFAULT;
        moduleCompileOptions.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_DEFAULT;
        pipelineCompileOptions = {};
        pipelineCompileOptions.traversableGraphFlags = OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_SINGLE_GAS;
        pipelineCompileOptions.usesMotionBlur = false;
        pipelineCompileOptions.numPayloadValues = 3;
        pipelineCompileOptions.numAttributeValues = 2;
        pipelineCompileOptions.exceptionFlags = OPTIX_EXCEPTION_FLAG_NONE;
        pipelineCompileOptions.pipelineLaunchParamsVariableName = "lp";

        pipelineLinkOptions.maxTraceDepth = 2;
		
		moduleCompileOptions1.maxRegisterCount = 0;
		moduleCompileOptions1.optLevel = OPTIX_COMPILE_OPTIMIZATION_DEFAULT;
		moduleCompileOptions1.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_DEFAULT;
		pipelineCompileOptions1 = {};
		pipelineCompileOptions1.traversableGraphFlags = OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_SINGLE_GAS;
		pipelineCompileOptions1.usesMotionBlur = false;
		pipelineCompileOptions1.numPayloadValues = 3;
		pipelineCompileOptions1.numAttributeValues = 2;
		pipelineCompileOptions1.exceptionFlags = OPTIX_EXCEPTION_FLAG_NONE;
		pipelineCompileOptions1.pipelineLaunchParamsVariableName = "lp";

		pipelineLinkOptions1.maxTraceDepth = 2;


        const std::string ptxCode = imageBytes;
		const std::string PackptxCOde = imageBytesPack;
        char log[2048];
        size_t sizeof_log = sizeof(log);
#if OPTIX_VERSION >= 70700
        OPTIX_CHECK(optixModuleCreate(optixContext,
            &moduleCompileOptions,
            &pipelineCompileOptions,
            ptxCode.c_str(),
            ptxCode.size(),
            log, &sizeof_log,
            &module
        ));
        if (sizeof_log > 1) PRINT(log);

		OPTIX_CHECK(optixModuleCreate(optixContext,
			&moduleCompileOptions1,
			&pipelineCompileOptions1,
			PackptxCOde.c_str(),
			PackptxCOde.size(),
			log, &sizeof_log,
			&Packmodule
		));
#else
        OPTIX_CHECK(optixModuleCreateFromPTX(optixContext,
            &moduleCompileOptions,
            &pipelineCompileOptions,
            ptxCode.c_str(),
            ptxCode.size(),
            log,      // Log string
            &sizeof_log,// Log string sizse
            &module
        ));
#endif
    }



    /*! does all setup for the raygen program(s) we are going to use */
    void SampleRenderer::createRaygenPrograms()
    {
        // we do a single ray gen program in this example:
        raygenPGs.resize(2);

        OptixProgramGroupOptions pgOptions = {};
        OptixProgramGroupDesc pgDesc = {};
        pgDesc.kind = OPTIX_PROGRAM_GROUP_KIND_RAYGEN;
        pgDesc.raygen.module = module;
        pgDesc.raygen.entryFunctionName = "__raygen__ddgi_accum";

        // OptixProgramGroup raypg;
        char log[2048];
        size_t sizeof_log = sizeof(log);
        OPTIX_CHECK(optixProgramGroupCreate(optixContext,
            &pgDesc,
            1,
            &pgOptions,
            log, &sizeof_log,
            &raygenPGs[0]
        ));


		pgDesc = {};
		pgDesc.kind = OPTIX_PROGRAM_GROUP_KIND_RAYGEN;
		pgDesc.raygen.module = Packmodule;
		pgDesc.raygen.entryFunctionName = "__raygen__pack_texels";

		
		OPTIX_CHECK(optixProgramGroupCreate(optixContext,
			&pgDesc, 1, &pgOptions, log, &sizeof_log, &raygenPGs[1]));
		   // now raygenPGs.size() == 2

        if (sizeof_log > 1) PRINT(log);
    }

    /*! does all setup for the miss program(s) we are going to use */
    void SampleRenderer::createMissPrograms()
    {
        // we do a single ray gen program in this example:
        missPGs.resize(1);

        OptixProgramGroupOptions pgOptions = {};
        OptixProgramGroupDesc pgDesc = {};
        pgDesc.kind = OPTIX_PROGRAM_GROUP_KIND_MISS;
        pgDesc.miss.module = module;
        pgDesc.miss.entryFunctionName = "__miss__radiance";

        // OptixProgramGroup raypg;
        char log[2048];
        size_t sizeof_log = sizeof(log);
        OPTIX_CHECK(optixProgramGroupCreate(optixContext,
            &pgDesc,
            1,
            &pgOptions,
            log, &sizeof_log,
            &missPGs[0]
        ));
        if (sizeof_log > 1) PRINT(log);
    }

    /*! does all setup for the hitgroup program(s) we are going to use */
    void SampleRenderer::createHitgroupPrograms()
    {
		hitgroupPGs.resize(RAY_TYPE_COUNT);          // 2 ray-types

		char  log[2048];
		size_t sizeof_log = sizeof(log);
		OptixProgramGroupOptions pgOpts = {};

		// helper lambda: creates one PG and stores handle in vector
		auto makePG = [&](int rtSlot,
			OptixModule modCH, const char* nameCH,
			OptixModule modAH, const char* nameAH)
			{
				OptixProgramGroupDesc pgDesc = {};
				pgDesc.kind = OPTIX_PROGRAM_GROUP_KIND_HITGROUP;

				pgDesc.hitgroup.moduleIS = nullptr;   // built-in triangles
				pgDesc.hitgroup.entryFunctionNameIS = nullptr;

				pgDesc.hitgroup.moduleCH = modCH;
				pgDesc.hitgroup.entryFunctionNameCH = nameCH;

				pgDesc.hitgroup.moduleAH = modAH;
				pgDesc.hitgroup.entryFunctionNameAH = nameAH;

				OPTIX_CHECK(optixProgramGroupCreate(optixContext,
					&pgDesc, 1, &pgOpts, log, &sizeof_log,
					&hitgroupPGs[rtSlot]));
				if (sizeof_log > 1) PRINT(log);
			};

		// ---------------- radiance ray-type ----------------------------
		makePG(/*rtSlot*/ RADIANCE_RAY_TYPE,
			/*CH*/ module, "__closesthit__radiance",
			/*AH*/ nullptr, nullptr);                // no any-hit

		// ---------------- shadow ray-type ------------------------------
		makePG(/*rtSlot*/ SHADOW_RAY_TYPE,
			/*CH*/ nullptr, nullptr,                 // no closest-hit
			/*AH*/ nullptr, nullptr);                // no any-hit
	}


    /*! assembles the full pipeline of all programs */
    void SampleRenderer::createPipeline()
    {
		std::vector<OptixProgramGroup> programGroups;
			programGroups.push_back(raygenPGs[0]);
		for (auto pg : missPGs)
			programGroups.push_back(pg);
		for (auto pg : hitgroupPGs)
			programGroups.push_back(pg);

		char log[2048];
		size_t sizeof_log = sizeof(log);
		OPTIX_CHECK(optixPipelineCreate(optixContext,
			&pipelineCompileOptions,
			&pipelineLinkOptions,
			programGroups.data(),
			(int)programGroups.size(),
			log, &sizeof_log,
			&pipeline
		));
        if (sizeof_log > 1) PRINT(log);

        OPTIX_CHECK(optixPipelineSetStackSize
        (/* [in] The pipeline to configure the stack size for */
            pipeline,
            /* [in] The direct stack size requirement for direct
               callables invoked from IS or AH. */
            2 * 1024,
            /* [in] The direct stack size requirement for direct
               callables invoked from RG, MS, or CH.  */
            2 * 1024,
            /* [in] The continuation stack requirement. */
            2 * 1024,
            /* [in] The maximum depth of a traversable graph
               passed to trace. */
            1));
		std::vector<OptixProgramGroup> programGroups1;
		programGroups1.push_back(raygenPGs[1]);
		for (auto pg : missPGs)
			programGroups1.push_back(pg);
		for (auto pg : hitgroupPGs)
			programGroups1.push_back(pg);
		OPTIX_CHECK(optixPipelineCreate(optixContext,
			&pipelineCompileOptions1, &pipelineLinkOptions1,
			programGroups1.data(), (int)programGroups1.size(),
			log, &sizeof_log, &pipelinePack));

        if (sizeof_log > 1) PRINT(log);
    }


    /*! constructs the shader binding table */
    void SampleRenderer::buildSBT()
    {
		// --- Raygen ---
		std::vector<RaygenRecord> raygenRecords;

		for (int i = 0; i < raygenPGs.size(); i++) {
			RaygenRecord rec = {};
			OPTIX_CHECK(optixSbtRecordPackHeader(raygenPGs[i], &rec));
			rec.data = {};
			raygenRecords.push_back(rec);
		}
		raygenRecordsBuffer.alloc_and_upload(raygenRecords);
		sbt.raygenRecord = raygenRecordsBuffer.d_pointer();

		// --- Miss ---
		std::vector<MissRecord> missRecords;
		for (int i = 0; i < missPGs.size(); i++) {
			MissRecord rec = {};
			OPTIX_CHECK(optixSbtRecordPackHeader(missPGs[i], &rec));
			rec.data = {};
			missRecords.push_back(rec);
		}
		missRecordsBuffer.alloc_and_upload(missRecords);
		sbt.missRecordBase = missRecordsBuffer.d_pointer();
		sbt.missRecordStrideInBytes = sizeof(MissRecord);
		sbt.missRecordCount = (int)missRecords.size();

		// --- Hitgroup ---
		std::vector<HitgroupRecord> hitgroupRecords;
		int numMeshes = (int)model->GetMeshes().size();
		for (int meshID = 0; meshID < numMeshes; meshID++) {
			for (int rayID = 0; rayID < RAY_TYPE_COUNT; rayID++) {
				HitgroupRecord rec = {};
				OPTIX_CHECK(optixSbtRecordPackHeader(hitgroupPGs[rayID], &rec));

				auto& mesh = model->GetMeshes()[meshID];
				rec.data.vertex = (vec3f*)vertexBuffer[meshID].d_pointer();
				rec.data.index = (vec3i*)indexBuffer[meshID].d_pointer();
				rec.data.normal = (vec3f*)normalBuffer[meshID].d_pointer();
				rec.data.texcoord = (vec2f*)texcoordBuffer[meshID].d_pointer();
				rec.data.hasTexture = false;
				rec.data.color = { 1.0f, 1.0f, 1.0f };

				hitgroupRecords.push_back(rec);
			}
		}
		hitgroupRecordsBuffer.alloc_and_upload(hitgroupRecords);
		sbt.hitgroupRecordBase = hitgroupRecordsBuffer.d_pointer();
		sbt.hitgroupRecordStrideInBytes = sizeof(HitgroupRecord);
		sbt.hitgroupRecordCount = (int)hitgroupRecords.size(); // meshCount * RAY_TYPE_COUNT

		// --------------------------------------------------------------------
	// D) fill TWO OptixShaderBindingTable structs
	// --------------------------------------------------------------------
		CUdeviceptr rgBase = raygenRecordsBuffer.d_pointer();
		CUdeviceptr msBase = missRecordsBuffer.d_pointer();
		CUdeviceptr hgBase = hitgroupRecordsBuffer.d_pointer();

		const uint32_t msStride = sizeof(MissRecord);
		const uint32_t hgStride = sizeof(HitgroupRecord);
		const uint32_t msCount = static_cast<uint32_t>(missRecords.size());
		const uint32_t hgCount = static_cast<uint32_t>(hitgroupRecords.size());
		OptixShaderBindingTable sbtAccum;
		// ---- SBT 0 : accumulate pass ---------------------------------------
		sbtAccum = {};
		sbtAccum.raygenRecord = rgBase;            // record 0
		sbtAccum.missRecordBase = msBase;
		sbtAccum.missRecordStrideInBytes = msStride;
		sbtAccum.missRecordCount = msCount;
		sbtAccum.hitgroupRecordBase = hgBase;
		sbtAccum.hitgroupRecordStrideInBytes = hgStride;
		sbtAccum.hitgroupRecordCount = hgCount;

		// ---- SBT 1 : packing pass ------------------------------------------
		sbtPack = sbtAccum;       // copy common fields
		sbtPack.raygenRecord = rgBase + sizeof(RaygenRecord);  // record 1
    }

	



	/*! render one frame */
    void SampleRenderer::render()
    {
		// 0) update params + clear accumulation
		CUDA_CHECK(MemsetAsync(
			reinterpret_cast<void*>(irrAccumBuffer.d_pointer()),
			0,
			irrAccumBuffer.sizeInBytes,
			stream));

		// 1) map D3D11 texture get CUarray
		CUDA_CHECK(GraphicsMapResources(1, &irrResCUDA, stream));
		
		
		// 2) fetch the *layered* array handle (this is key!)
		cudaMipmappedArray_t mipmappedArray = nullptr;
		CUDA_CHECK(GraphicsResourceGetMappedMipmappedArray(
			&mipmappedArray,
			irrResCUDA));

		cudaArray_t layeredArray = nullptr;
		// level 0 of the mipmapped array is the full texture2DArray
		CUDA_CHECK(GetMipmappedArrayLevel(&layeredArray, mipmappedArray, 0));



		// 3) build your surface object once per frame
		cudaResourceDesc rdesc = {};
		rdesc.resType = cudaResourceTypeArray;
		rdesc.res.array.array = layeredArray;
		// 2) create a fresh surface object for this CUarray
		cudaSurfaceObject_t surfObj = 0;
		CUDA_CHECK(CreateSurfaceObject(&surfObj, &rdesc));

		// 3) patch your params to point at it
		launchParams.irrSurf = surfObj;
		launchParams.irrAccum = reinterpret_cast<vec4f*>(irrAccumBuffer.d_pointer());
		static_assert(offsetof(LaunchParams, irrSurf) % alignof(cudaSurfaceObject_t) == 0,
			"irrSurf must be 8-byte aligned!");
		launchParamsBuffer.upload(&launchParams, 1);
		// 4) do your two OptiX launches
		dim3 dimA(256, launchParams.probeCount, 1);
		optixLaunch(pipeline, stream,
			launchParamsBuffer.d_pointer(),
			sizeof(LaunchParams), &sbt,
			dimA.x, dimA.y, dimA.z);

		dim3 dimP(launchParams.texels, 1, 1);
		optixLaunch(pipelinePack, stream,
			launchParamsBuffer.d_pointer(),
			sizeof(LaunchParams), &sbtPack,
			dimP.x, dimP.y, dimP.z);

		// 5) tear down: destroy surface + unmap
		CUDA_CHECK(DestroySurfaceObject(surfObj));
		CUDA_CHECK(GraphicsUnmapResources(1, &irrResCUDA, stream));

		// 6) wait for everything to finish before next frame
		CUDA_CHECK(StreamSynchronize(stream));
		launchParams.frameID++;
    }

  
} // ::osc
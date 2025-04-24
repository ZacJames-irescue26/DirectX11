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




    extern "C" char imageBytes[];

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
    SampleRenderer::SampleRenderer(std::vector<Engine::GameObject> objects)
    {

        initOptix();
		model = std::make_unique<Engine::Model>(objects[0].model);
		launchParams.light.origin = {-1000.00000, 1000.000000, 1000.000000 };
		launchParams.light.du = {400.000000, 400.00000000, 400.00000000};
		launchParams.light.dv = {400.00000000, 400.00000000, 400.000000};
		launchParams.light.power = {0.0,0.0,0.0};



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

        launchParams.traversable = buildAccel(objects);

        std::cout << "#osc: setting up optix pipeline ..." << std::endl;
        createPipeline();

        std::cout << "#osc: building SBT ..." << std::endl;
        buildSBT();

        launchParamsBuffer.alloc(sizeof(launchParams));
        std::cout << "#osc: context, module, pipeline, etc, all set up ..." << std::endl;

       
        std::cout << "#osc: Optix 7 Sample fully set up" << std::endl;
        resize(gdt::vec2i(SCREEN_WIDTH, SCREEN_HEIGHT));
    }

	//void SampleRenderer::createTextures()
	//{

	//	int numTextures = 0;
	//	for (const auto& mesh : model->GetMeshes())
	//	{
	//		numTextures += mesh.textures.size();
	//	}

	//	textureArrays.resize(numTextures);
	//	textureObjects.resize(numTextures);
	//	uint32_t textureid = 0;
	//	for ( auto& mesh : model->meshes)
	//	{
	//	for ( auto& texture: mesh.textures) {
	//		std::string filename = std::string(texture.path);
	//		filename = texture.Directory + '/' + filename;

	//		int m_width, m_height, nrComponents;
	//		uint8_t* data = stbi_load(filename.c_str(), &m_width, &m_height, &nrComponents, 4);

	//		cudaResourceDesc res_desc = {};

	//		cudaChannelFormatDesc channel_desc;
	//		int32_t width = m_width;
	//		int32_t height = m_height;
	//		int32_t numComponents = 4;
	//		int32_t pitch = width * numComponents * sizeof(uint8_t);
	//		channel_desc = cudaCreateChannelDesc<uchar4>();
	//		cudaArray_t& pixelArray = textureArrays[textureid];
	//		switch (texture.type)
	//		{
	//		case Radiant::TextureType::Diffuse:
	//			pixelArray = mesh.DiffuseTexture;
	//			break;

	//		case Radiant::TextureType::Normal:
	//			pixelArray = mesh.NormaltextureArrays;
	//			break;
	//		case Radiant::TextureType::Roughness:
	//			pixelArray = mesh.RoughnesstextureArrays;
	//			break;

	//		}
	//		CUDA_CHECK(MallocArray(&pixelArray,
	//			&channel_desc,
	//			width, height));

	//		CUDA_CHECK(Memcpy2DToArray(pixelArray,
	//			/* offset */0, 0,
	//			data,
	//			pitch, pitch, height,
	//			cudaMemcpyHostToDevice));

	//		res_desc.resType = cudaResourceTypeArray;
	//		res_desc.res.array.array = pixelArray;

	//		cudaTextureDesc tex_desc = {};
	//		tex_desc.addressMode[0] = cudaAddressModeWrap;
	//		tex_desc.addressMode[1] = cudaAddressModeWrap;
	//		tex_desc.filterMode = cudaFilterModeLinear;
	//		tex_desc.readMode = cudaReadModeNormalizedFloat;
	//		tex_desc.normalizedCoords = 1;
	//		tex_desc.maxAnisotropy = 1;
	//		tex_desc.maxMipmapLevelClamp = 99;
	//		tex_desc.minMipmapLevelClamp = 0;
	//		tex_desc.mipmapFilterMode = cudaFilterModePoint;
	//		tex_desc.borderColor[0] = 1.0f;
	//		tex_desc.sRGB = 0;

	//		// Create texture object
	//		cudaTextureObject_t cuda_tex = 0;
	//		CUDA_CHECK(CreateTextureObject(&cuda_tex, &res_desc, &tex_desc, nullptr));
	//		textureObjects[textureid] = cuda_tex;
	//		auto& cuda = cuda_tex;
	//		switch (texture.type)
	//		{
	//		case Radiant::TextureType::Diffuse:
	//			mesh.DiffusetextureObjects = cuda_tex;
	//			break;

	//		case Radiant::TextureType::Normal:
	//			mesh.NormaltextureObjects = cuda_tex;
	//			break;
	//		case Radiant::TextureType::Roughness:
	//			mesh.RoughnesstextureObjects = cuda_tex;
	//			break;

	//		}
	//		textureid++;
	//		stbi_image_free(data);
	//	}
	//	}

	//}

    
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
	//void SampleRenderer::createTextures()
	//{
	//	int numTextures = (int)model->meshes[0].textures.size();

	//	textureArrays.resize(numTextures);
	//	textureObjects.resize(numTextures);
	//	for (const auto& meshes : model->meshes)
	//	{
	//		for (int textureID = 0; textureID < numTextures; textureID++) {
	//			auto texture = model->meshes[0].textures[textureID];

	//			cudaResourceDesc res_desc = {};

	//			cudaChannelFormatDesc channel_desc;
	//			int32_t width = texture->resolution.x;
	//			int32_t height = texture->resolution.y;
	//			int32_t numComponents = 4;
	//			int32_t pitch = width * numComponents * sizeof(uint8_t);
	//			channel_desc = cudaCreateChannelDesc<uchar4>();

	//			cudaArray_t& pixelArray = textureArrays[textureID];
	//			CUDA_CHECK(MallocArray(&pixelArray,
	//				&channel_desc,
	//				width, height));

	//			CUDA_CHECK(Memcpy2DToArray(pixelArray,
	//				/* offset */0, 0,
	//				texture->pixel,
	//				pitch, pitch, height,
	//				cudaMemcpyHostToDevice));

	//			res_desc.resType = cudaResourceTypeArray;
	//			res_desc.res.array.array = pixelArray;

	//			cudaTextureDesc tex_desc = {};
	//			tex_desc.addressMode[0] = cudaAddressModeWrap;
	//			tex_desc.addressMode[1] = cudaAddressModeWrap;
	//			tex_desc.filterMode = cudaFilterModeLinear;
	//			tex_desc.readMode = cudaReadModeNormalizedFloat;
	//			tex_desc.normalizedCoords = 1;
	//			tex_desc.maxAnisotropy = 1;
	//			tex_desc.maxMipmapLevelClamp = 99;
	//			tex_desc.minMipmapLevelClamp = 0;
	//			tex_desc.mipmapFilterMode = cudaFilterModePoint;
	//			tex_desc.borderColor[0] = 1.0f;
	//			tex_desc.sRGB = 0;

	//			// Create texture object
	//			cudaTextureObject_t cuda_tex = 0;
	//			CUDA_CHECK(CreateTextureObject(&cuda_tex, &res_desc, &tex_desc, nullptr));
	//			textureObjects[textureID] = cuda_tex;
	//		}
	//	}
	//	}
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
        moduleCompileOptions.maxRegisterCount = 50;
        moduleCompileOptions.optLevel = OPTIX_COMPILE_OPTIMIZATION_DEFAULT;
        moduleCompileOptions.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_NONE;

        pipelineCompileOptions = {};
        pipelineCompileOptions.traversableGraphFlags = OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_SINGLE_GAS;
        pipelineCompileOptions.usesMotionBlur = false;
        pipelineCompileOptions.numPayloadValues = 2;
        pipelineCompileOptions.numAttributeValues = 2;
        pipelineCompileOptions.exceptionFlags = OPTIX_EXCEPTION_FLAG_NONE;
        pipelineCompileOptions.pipelineLaunchParamsVariableName = "optixLaunchParams";

        pipelineLinkOptions.maxTraceDepth = 2;
		
        const std::string ptxCode = imageBytes;

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
        if (sizeof_log > 1) PRINT(log);
    }



    /*! does all setup for the raygen program(s) we are going to use */
    void SampleRenderer::createRaygenPrograms()
    {
        // we do a single ray gen program in this example:
        raygenPGs.resize(1);

        OptixProgramGroupOptions pgOptions = {};
        OptixProgramGroupDesc pgDesc = {};
        pgDesc.kind = OPTIX_PROGRAM_GROUP_KIND_RAYGEN;
        pgDesc.raygen.module = module;
        pgDesc.raygen.entryFunctionName = "__raygen__renderFrame";

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
		// for this simple example, we set up a single hit group
		hitgroupPGs.resize(RAY_TYPE_COUNT);

		char log[2048];
		size_t sizeof_log = sizeof(log);

		OptixProgramGroupOptions pgOptions = {};
		OptixProgramGroupDesc    pgDesc = {};
		pgDesc.kind = OPTIX_PROGRAM_GROUP_KIND_HITGROUP;
		pgDesc.hitgroup.moduleCH = module;
		pgDesc.hitgroup.moduleAH = module;

		// -------------------------------------------------------
		// radiance rays
		// -------------------------------------------------------
		pgDesc.hitgroup.entryFunctionNameCH = "__closesthit__radiance";
		pgDesc.hitgroup.entryFunctionNameAH = "__anyhit__radiance";

		OPTIX_CHECK(optixProgramGroupCreate(optixContext,
			&pgDesc,
			1,
			&pgOptions,
			log, &sizeof_log,
			&hitgroupPGs[RADIANCE_RAY_TYPE]
		));
		if (sizeof_log > 1) PRINT(log);

		// -------------------------------------------------------
		// shadow rays: technically we don't need this hit group,
		// since we just use the miss shader to check if we were not
		// in shadow
		// -------------------------------------------------------
		pgDesc.hitgroup.entryFunctionNameCH = "__closesthit__shadow";
		pgDesc.hitgroup.entryFunctionNameAH = "__anyhit__shadow";

		OPTIX_CHECK(optixProgramGroupCreate(optixContext,
			&pgDesc,
			1,
			&pgOptions,
			log, &sizeof_log,
			&hitgroupPGs[SHADOW_RAY_TYPE]
		));
		if (sizeof_log > 1) PRINT(log);

    }


    /*! assembles the full pipeline of all programs */
    void SampleRenderer::createPipeline()
    {
        std::vector<OptixProgramGroup> programGroups;
        for (auto pg : raygenPGs)
            programGroups.push_back(pg);
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
    }

	//void SampleRenderer::Draw()
	//{
	//	
 //       glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//	
	//	downloadPixels(pixels.data());
	//	if (!IsCreated)			
	//		glGenTextures(1, &fbTexture); IsCreated = true; ;

	//	glBindTexture(GL_TEXTURE_2D, fbTexture);
	//	GLenum texFormat = GL_RGBA;
	//	GLenum texelType = GL_UNSIGNED_BYTE;
	//	glTexImage2D(GL_TEXTURE_2D, 0, texFormat, launchParams.frame.size.x, launchParams.frame.size.y, 0, GL_RGBA,
	//		texelType, pixels.data());
	//	
	//	
	//	glClearColor(0.2,0.2,0.2,1.0);

	//	glEnable(GL_TEXTURE_2D);
	//	glBindTexture(GL_TEXTURE_2D, fbTexture);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//	glDisable(GL_DEPTH_TEST);

	//	glViewport(0, 0, launchParams.frame.size.x, launchParams.frame.size.y);
 //      
 //      FullScreenProgram->bind();
	//   FullScreenProgram->setInt("tex", 0);
 //      glActiveTexture(GL_TEXTURE0);
 //      glBindTexture(GL_TEXTURE_2D, fbTexture);
 //      ScreenQuad->Bind();
 //      ScreenQuad->Draw();
	//    
	//}

	/*! render one frame */
    void SampleRenderer::render()
    {
		// sanity check: make sure we launch only after first resize is
		 // already done:
		if (launchParams.frame.size.x == 0) return;

		launchParamsBuffer.upload(&launchParams, 1);

		OPTIX_CHECK(optixLaunch(/*! pipeline we're launching launch: */
			pipeline, stream,
			/*! parameters and SBT */
			launchParamsBuffer.d_pointer(),
			launchParamsBuffer.sizeInBytes,
			&sbt,
			/*! dimensions of the launch: */
			launchParams.frame.size.x,
			launchParams.frame.size.y,
			1
		));
		// sync - make sure the frame is rendered before we download and
		// display (obviously, for a high-performance application you
		// want to use streams and double-buffering, but for this simple
		// example, this will have to do)
		CUDA_SYNC_CHECK();
    }

    /*! resize frame buffer to given resolution */
    void SampleRenderer::resize(const gdt::vec2i& newSize)
    {
		// if window minimized
		if (newSize.x == 0 | newSize.y == 0) return;

		// resize our cuda frame buffer
		colorBuffer.resize(newSize.x * newSize.y * sizeof(uint32_t));
		

		// update the launch parameters that we'll pass to the optix
		// launch:
		launchParams.frame.size = newSize;
		launchParams.frame.colorBuffer = (uint32_t*)colorBuffer.d_pointer();
        pixels.resize(newSize.x * newSize.y);
		// and re-set the camera, since aspect may have changed
		
		Camera camera = Camera (vec3f(-1293.07f, 154.681f, -0.7304f),
	        vec3f(10.f, 10.f, 10.f),
	        vec3f(0.f, 1.f, 0.f));
        setCamera(camera);

    }

    /*! download the rendered color buffer */
    void SampleRenderer::downloadPixels(uint32_t h_pixels[])
    {
		
		colorBuffer.download(h_pixels,
			launchParams.frame.size.x * launchParams.frame.size.y);
    }
	/*! set camera to render with */
	void SampleRenderer::setCamera(const Camera& camera)
	{
		lastSetCamera = camera;
		launchParams.camera.position = camera.from;
		launchParams.camera.direction = normalize(camera.at - camera.from);
		const float cosFovy = 0.66f;
		const float aspect = launchParams.frame.size.x / float(launchParams.frame.size.y);
		launchParams.camera.horizontal
			= cosFovy * aspect * normalize(cross(launchParams.camera.direction,
				camera.up));
		launchParams.camera.vertical
			= cosFovy * normalize(cross(launchParams.camera.horizontal,
				launchParams.camera.direction));
	}

} // ::osc
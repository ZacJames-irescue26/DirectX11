#pragma once
#include <cuda_runtime.h>
#include <optix.h>
#include <optix_stubs.h>
#include <sstream>
#include <stdexcept>
//#define CUDA_CHECK( call ) ErrorChecking::cudaCheck( call, #call, __FILE__, __LINE__ )
//
//#define CUDA_SYNC_CHECK() ErrorChecking::cudaSyncCheck( __FILE__, __LINE__ )
//
//#define OPTIX_CHECK( call )                                                    \
//    ErrorChecking::optixCheck( call, #call, __FILE__, __LINE__ )
//
//namespace ErrorChecking
//{
//	inline void cudaCheck(cudaError_t error, const char* call, const char* file, unsigned int line)
//	{
//		if (error != cudaSuccess)
//		{
//			std::stringstream ss;
//			ss << "CUDA call (" << call << " ) failed with error: '"
//				<< cudaGetErrorString(error) << "' (" << file << ":" << line << ")\n";
//			throw std::runtime_error(ss.str().c_str());
//		}
//	}
//
//	inline void cudaSyncCheck(const char* file, unsigned int line)
//	{
//		cudaDeviceSynchronize();
//		cudaError_t error = cudaGetLastError();
//		if (error != cudaSuccess)
//		{
//			std::stringstream ss;
//			ss << "CUDA error on synchronize with error '"
//				<< cudaGetErrorString(error) << "' (" << file << ":" << line << ")\n";
//			throw std::runtime_error(ss.str().c_str());
//		}
//	}
//
//
//	inline void optixCheck(OptixResult res, const char* call, const char* file, unsigned int line)
//	{
//		if (res != OPTIX_SUCCESS)
//		{
//			std::stringstream ss;
//			ss << "Optix call '" << call << "' failed: " << file << ':' << line << ")\n";
//			throw std::runtime_error(ss.str().c_str());
//		}
//	}
//
//}

#define CUDA_CHECK(call)							\
    {									\
      cudaError_t rc = cuda##call;                                      \
      if (rc != cudaSuccess) {                                          \
        std::stringstream txt;                                          \
        cudaError_t err =  rc; /*cudaGetLastError();*/                  \
        txt << "CUDA Error " << cudaGetErrorName(err)                   \
            << " (" << cudaGetErrorString(err) << ")";                  \
        throw std::runtime_error(txt.str());                            \
      }                                                                 \
    }

#define CUDA_CHECK_NOEXCEPT(call)                                        \
    {									\
      cuda##call;                                                       \
    }

#define OPTIX_CHECK( call )                                             \
  {                                                                     \
    OptixResult res = call;                                             \
    if( res != OPTIX_SUCCESS )                                          \
      {                                                                 \
        fprintf( stderr, "Optix call (%s) failed with code %d (line %d)\n", #call, res, __LINE__ ); \
        exit( 2 );                                                      \
      }                                                                 \
  }

#define CUDA_SYNC_CHECK()                                               \
  {                                                                     \
    cudaDeviceSynchronize();                                            \
    cudaError_t error = cudaGetLastError();                             \
    if( error != cudaSuccess )                                          \
      {                                                                 \
        fprintf( stderr, "error (%s: line %d): %s\n", __FILE__, __LINE__, cudaGetErrorString( error ) ); \
        exit( 2 );                                                      \
      }                                                                 \
  }


#define PRINT(inLog) std::cout << inLog << std::endl;
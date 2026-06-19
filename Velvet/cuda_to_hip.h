#pragma once

// CUDA-to-HIP compatibility header for Velvet.
// Provides CUDA API mappings when building with HIP for AMD GPUs.
//
// Copyright (c) 2026 Advanced Micro Devices, Inc.
// Author: Jeff Daily <jeff.daily@amd.com>

#if defined(USE_HIP) || defined(__HIP_PLATFORM_AMD__)

#include <hip/hip_runtime.h>
#include <hip/hip_gl_interop.h>

// Runtime API - memory
#define cudaMallocManaged          hipMallocManaged
#define cudaFree                   hipFree
#define cudaMemcpy                 hipMemcpy
#define cudaMemcpyDefault          hipMemcpyDefault
#define cudaMemsetAsync            hipMemsetAsync
#define cudaDeviceSynchronize      hipDeviceSynchronize
#define cudaMemcpyHostToDevice     hipMemcpyHostToDevice

// hipMemcpyToSymbolAsync requires offset and kind args; CUDA has defaults
#define cudaMemcpyToSymbolAsync(symbol, src, size) \
    hipMemcpyToSymbolAsync(symbol, src, size, 0, hipMemcpyHostToDevice, 0)

// Events
#define cudaEvent_t                hipEvent_t
#define cudaEventCreate            hipEventCreate
#define cudaEventRecord            hipEventRecord
#define cudaEventSynchronize       hipEventSynchronize
#define cudaEventElapsedTime       hipEventElapsedTime
#define cudaEventDestroy           hipEventDestroy

// Error handling
#define cudaError_t                hipError_t
#define cudaSuccess                hipSuccess
#define cudaGetErrorName           hipGetErrorName
#define cudaGetLastError           hipGetLastError

// OpenGL interop
#define cudaGraphicsResource       hipGraphicsResource
#define cudaGraphicsGLRegisterBuffer       hipGraphicsGLRegisterBuffer
#define cudaGraphicsMapResources           hipGraphicsMapResources
#define cudaGraphicsResourceGetMappedPointer hipGraphicsResourceGetMappedPointer
#define cudaGraphicsUnmapResources         hipGraphicsUnmapResources
#define cudaGraphicsUnregisterResource     hipGraphicsUnregisterResource
#define cudaGraphicsRegisterFlagsNone      hipGraphicsRegisterFlagsNone

// checkCudaErrors macro for HIP
#ifndef checkCudaErrors
#define checkCudaErrors(val) __checkCudaErrors((val), #val, __FILE__, __LINE__)

template <typename T>
void __checkCudaErrors(T result, const char *const func, const char *const file,
                       int const line) {
  if (result != hipSuccess) {
    fprintf(stderr, "HIP error at %s:%d code=%d(%s) \"%s\" \n", file, line,
            static_cast<unsigned int>(result), hipGetErrorName(result), func);
    exit(EXIT_FAILURE);
  }
}
#endif

#else  // CUDA path

#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <helper_cuda.h>

#endif

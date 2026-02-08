#include <cuda_runtime.h>
#include <cstdio>

extern "C" __declspec(dllexport)
void runCudaSanity()
{
    int deviceCount = 0;
    cudaError_t err = cudaGetDeviceCount(&deviceCount);

    if (err != cudaSuccess || deviceCount == 0)
    {
        printf("CUDA ERROR: No device found (%s)\n",
            cudaGetErrorString(err));
        return;
    }

    int device = 0;
    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, device);

    printf("=== CUDA SANITY INFO ===\n");
    printf("Device ID          : %d\n", device);
    printf("Name               : %s\n", prop.name);
    printf("Compute Capability : %d.%d\n", prop.major, prop.minor);
    printf("SM Count           : %d\n", prop.multiProcessorCount);
    printf("Warp Size          : %d\n", prop.warpSize);
    printf("Max Threads/Block  : %d\n", prop.maxThreadsPerBlock);
    printf("Global Memory      : %.2f GB\n",
        prop.totalGlobalMem / (1024.0 * 1024.0 * 1024.0));

    cudaDeviceSynchronize();
}

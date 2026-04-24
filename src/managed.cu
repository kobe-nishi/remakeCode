#include <cuda.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>
#include <managed.h>

void* Managed::operator new(size_t len) {
  void *ptr;
  cudaMallocManaged(&ptr, len);
  cudaDeviceSynchronize();
  return ptr;
}

void Managed::operator delete(void *ptr) {
  cudaDeviceSynchronize();
  cudaFree(ptr);
}
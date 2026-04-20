#include <cstring>
#include <iostream>
#include <cuda.h>
#include <helper_cuda.h>

#include "mydef.h"

bool Init::restart       = false;
int Init::device         = DEV;
std::string Init::config = "config";

void Init::srice_device(int argc, char** argv) {
  std::cerr << "argc: " << argc << std::endl;
  for (int j = 0; j < argc; ++j) {
    std::cerr << "argv[" << j << "]: "
              << (argv[j] ? argv[j] : const_cast<char*>("(null)"))
              << std::endl;
  }
  int i = 1;
  while (argc > 1) {
    if (strncmp(argv[i], "-help", std::strlen("-help")) == 0 || strncmp(argv[i], "-h", 2) == 0) {
      std::cout << "Usage: " << argv[0] << " [options]\n"
                << "Options:\n"
                << "-h, -help        Show this help message\n"
                << "-r, -restart     Restart the simulation\n"
                << "-c, -config      Specify the configuration file (default: config)\n";
      exit(0);
    } else if (strncmp(argv[i], "-restart", std::strlen("-restart")) == 0 || strncmp(argv[i], "-r", 2) == 0) {
      restart = true;
      std::cout << "Restart the simulation" << std::endl;
      argc -= 1;
      i += 1;
    } else if (strncmp(argv[i], "-cuda_set_device", std::strlen("-cuda_set_device")) == 0) {
      device = atoi(argv[i + 1]);
      argc -= 2;
      i += 2;
    } else if (strncmp(argv[i], "-config", std::strlen("-config")) == 0 || strncmp(argv[i], "-c", 2) == 0) {
      config = argv[i + 1];
      argc -= 2;
      i += 2;
    } else {
      std::cerr << "Unknown option: " << argv[i] << std::endl;
      std::cerr << "Use -h or -help to see the usage." << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}

void Init::GPU_init() {
  int dev;
  cudaDeviceProp prop;
  cudaSetDevice(0);
  cudaGetDevice(&dev);
  cudaGetDeviceProperties(&prop, dev);

  if (prop.major == 9999 && prop.minor == 9999) {
    std::cout << "GPU initialize error" << std::endl;
  }

  std::cout << "========================= GPU Information ========================\n"
            << "Device Name: " << prop.name << "\n"
            << "Device Number: " << device << "\n"
            << "Compute capability: " << prop.major << "." << prop.minor << "\n"
            << "Architecture: sm_" << prop.major << prop.minor << "\n"
            << "Total Global Memory: " << prop.totalGlobalMem / (1024 * 1024) << " MB\n"
            << "Shared Memory per Block: " << prop.sharedMemPerBlock / 1024 << " KB\n"
            << "Registers per Block: " << prop.regsPerBlock << "\n"
            << "Warp Size: " << prop.warpSize << "\n"
            << "Max Threads per Block: " << prop.maxThreadsPerBlock << "\n"
            << "Max Threads Dimensions: ("
            << prop.maxThreadsDim[0] << ", "
            << prop.maxThreadsDim[1] << ", "
            << prop.maxThreadsDim[2] << ")\n"
            << "Max Grid Size: ("
            << prop.maxGridSize[0] << ", "
            << prop.maxGridSize[1] << ", "
            << prop.maxGridSize[2] << ")\n";
}

Init::Init(int argc, char** argv) {
  srice_device(argc, argv);
  GPU_init();
}
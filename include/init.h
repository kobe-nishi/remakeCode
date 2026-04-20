#pragma once

#include <string>

class Init {
  private:
  static void srice_device(int argc, char **argv);
  static void GPU_init();

  public:
  static bool restart;
  static int device;
  static std::string config;
  Init(int argc, char **argv);
  ~Init(){};
};
#pragma once

#include <array>
#include <filesystem>
#include <vector>

class Domain : public Managed {
    public:
    /*== core ==*/
    int device         = 0;
    bool restart       = false;
    long long iter     = 0;
    long long iter_max = 0;
    double time        = 0.0;
    double time_max    = 1.0e2;
    double dt          = 1.0e-2;
    std::filesystem::path config_file;
    std::filesystem::path results_dir;
    xdmf3::Format xdmf_format = xdmf3::Format::Unknown;
    float elapsed_time[10]{};
    bool other_datasets_restart = false;

    public:
    Domain(const bool& nop = false);
    ~Domain() {};
    void print();
    void time_print(const int div = 1);
    void read_config();
    void write_config(const std::filesystem::path& dirpath);
}
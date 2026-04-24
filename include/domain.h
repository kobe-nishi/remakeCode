#pragma once
#define OUTPUT_BINARY
#include <array>
#include <filesystem>
#include <vector>
#include <string>

#include "managed.h"
#include "vec3.h"
#include "xdmf3.h"
using Double3 = vec3<double>;

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

    /*== config ==*/
    Double3 mesh_length;
    std::string mesh_shape;
    std::string lsp_shape;
    std::array<bool, 6> dirichlet_bc; // 0: east, 1: north, 2: west, 3: south, 4: top, 5: bottom

    bool reset_lsp_matrix       = true;
    bool reset_current_data     = true;

    double neo_mu = 1.0;

    /*=== newton-raphson condition ===*/
    int newton_max     = 1e2;
    double newton_tole = 1.0e-10;

    int ng = 4;
    /*=== output condition ===*/
    int output_idx              = 0;
    int output_div              = 1;
    int output_interval         = 1e2;
    int output_restart_interval = 1e0;

    /*=== growth points ===*/
    // bool growth_point_restart = false;
    // int growth_point_num      = 0;
    // std::vector<int> growth_point_idx;

    public:
    Domain(const bool& nop = false);
    ~Domain() {};
    void print();
    void time_print(const int div = 1);
    void read_config();
    void write_config(const std::filesystem::path& dirpath);
};
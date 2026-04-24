#pragma once

#include <algorithm>
#include <cstdint>
#include <cxxabi.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <typeinfo>
#include <unordered_map>
#include <vector>

class Cp : public Double3 {
    public:
        double weight;
        Double3 pos;
        double w;
        Cp() : Double3(), w(1.0) {};
        Cp(double x_, double y_, double z_, double w_ = 1.0)
            : Double3(x_, y_, z_), w(w_) {};
        ~Cp() {};

        double& operator[](int i) {
            switch (i) {
            // clang-format off
            case 0: return weight;
            case 1: return pos.x; case 2: return pos.y; case 3: return pos.z;
            // clang-format on
            default:
            std::cerr << "Index " << i << " is out of range in Cp." << std::endl;
            exit(EXIT_FAILURE);
            }
        }
        const double& operator[](int i) const {
            switch (i) {
            // clang-format off
            case 0: return weight;
            case 1: return pos.x; case 2: return pos.y; case 3: return pos.z;
            // clang-format on
            default:
            std::cerr << "Index " << i << " is out of range in Cp." << std::endl;
            exit(EXIT_FAILURE);
            }
        }
        friend std::ostream& operator<<(std::ostream& os, const Cp& cp) {
            //clang-format off
            os << "weight: "
            << cp.weight << ", pos: " << cp.pos;
            //clang-format on
            return os;
        }
};
class Spline{
    public:
        int p[3]{0}; //Polynominal degree of nurbs basis
        int nf[3]{0}; //Number of faces
        Double3 offset[3];
        int cp_dim = 4;

        std::vector<Cp> cp;

    public:
        void cp_write(const std::filesystem::path& dirpath,const std::string& filename);
        void cp_read(const std::filesystem::path& dirpath,const std::string& filename);
        void write_config(const std::filesystem::path& dirpath);
        void read_config(const std::filesystem::path& dirpath);
        void write_2d_array(const std::filesystem::path& filepath, const std::vector<std::vector<int>>& array_2d);
        void write_1d_array(const std::filesystem::path& filepath, const std::vector<int>& array_1d);
        void save_mesh_data(const std::filesystem::path& dirpath);

};
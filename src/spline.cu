#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "mydef.h"

using namespace std;
using uint        = unsigned int;
using json        = nlohmann::json;
using orderd_json = nlohmann::ordered_json;
namespace fs      = std::filesystem;

#define THREAD 128

void Spline::cp_read(const fs::path& dirpath, const std::string& filename) {
    fs::path filepath = dirpath / filename;
    ifstream ifs(filepath, ios::binary | ios::in);
    if (ifs.fail()) {
        cerr << "Could not open the file: " << filepath << endl;
        exit(EXIT_FAILURE);
    }
    int size = 0;
    ifs.read(reinterpret_cast<char*>(&size), sizeof(int));
    cp.resize(size);
    for (int i = 0; i < size; i++) {
        ifs.read(reinterpret_cast<char*>(&cp[i].x), sizeof(double));
        ifs.read(reinterpret_cast<char*>(&cp[i].y), sizeof(double));
        ifs.read(reinterpret_cast<char*>(&cp[i].z), sizeof(double));
        ifs.read(reinterpret_cast<char*>(&cp[i].w), sizeof(double));
    }
    ifs.close();
}

void Spline::cp_write(const fs::path& dirpath, const string& filename) {
    fs::path filepath = dirpath / filename;
    ofstream ofs(filepath, ios::binary);
    if (ofs.fail()) {
        cerr << "Could not open the file: " << filepath << endl;
        exit(EXIT_FAILURE);
    }

    int size = cp.size();
    ofs.write(reinterpret_cast<const char*>(&size), sizeof(int));
    for(int i = 0; i < size; i++){
        ofs.write(reinterpret_cast<const char*>(&cp[i].x), sizeof(double));
        ofs.write(reinterpret_cast<const char*>(&cp[i].y), sizeof(double));
        ofs.write(reinterpret_cast<const char*>(&cp[i].z), sizeof(double));
        ofs.write(reinterpret_cast<const char*>(&cp[i].w), sizeof(double));
    }
    ofs.close();
}

void Spline::write_config(const fs::path& dirpath){
    if(!fs::exists(dirpath)) fs::create_directories(dirpath);

    json m_json;
    m_json["root"] = dirpath.string();
    for(int i = 0; i < 3; i++){
        m_json["p"][i]           = p[i];
        m_json["nf"][i]          = nf[i];
        m_json["offset"][i]["x"] = offset[i].x;
        m_json["offset"][i]["y"] = offset[i].y;
        m_json["offset"][i]["z"] = offset[i].z;
    }
    m_json["cp_dim"] = cp_dim;

    ofstream ofs(dirpath / "spline.json");
    ofs << m_json.dump(4) << endl;
    ofs.close();

    cp_write(dirpath, "cp.bin");

    save_mesh_data(dirpath);
}

void Spline::read_config(const fs::path& dirpath) {
    fs::path filepath = dirpath / "spline.json";
    ifstream ifs(filepath);
    if (ifs.fail()) {
        cerr << "Could not open the file: " << filepath << endl;
        exit(EXIT_FAILURE);
    }
    json m_json = json::parse(ifs);
    ifs.close();

    m_json["root"] = dirpath.string();

    for(int i = 0; i < 3; i++){
        this->p[i]           = m_json["p"][i];
        this->nf[i]          = m_json["nf"][i];
        this->offset[i].x    = m_json["offset"][i]["x"];
        this->offset[i].y    = m_json["offset"][i]["y"];
        this->offset[i].z    = m_json["offset"][i]["z"];
    }
    this->cp_dim      = m_json["cp_dim"];

    cp_read(dirpath, "cp.bin");
}

void Spline::write_2d_array(const fs::path& filepath, const vector<vector<int>>& array_2d) {
    ofstream ofs(filepath, ios::binary);
    if (!ofs) { cerr << "Open error: " << filepath << endl; exit(1); }
    int num_rows = array_2d.size();
    int num_cols = num_rows > 0 ? array_2d[0].size() : 0;
    ofs.write(reinterpret_cast<const char*>(&num_rows), sizeof(int));
    ofs.write(reinterpret_cast<const char*>(&num_cols), sizeof(int));
    for (int i = 0; i < num_rows; i++) {
        ofs.write(reinterpret_cast<const char*>(array_2d[i].data()), num_cols * sizeof(int));
    }
    ofs.close();
}

void Spline::write_1d_array(const fs::path& filepath, const vector<int>& array_1d) {
    ofstream ofs(filepath, ios::binary);
    if (!ofs) { cerr << "Open error: " << filepath << endl; exit(1); }
    int size = array_1d.size();
    ofs.write(reinterpret_cast<const char*>(&size), sizeof(int));
    ofs.write(reinterpret_cast<const char*>(array_1d.data()), size * sizeof(int));
    ofs.close();
}

void Spline::save_mesh_data(const fs::path& dirpath) {
    int p1 = p[0], p2 =p[1], p3 = p[2];
    int ncp1 = nf[0] + p1;
    int ncp2 = nf[1];
    int ncp3 = nf[2] + p3;

    int num_elements = nf[0] * nf[1] * nf[2];
    int nodes_per_elem = (p1 + 1) * (p2 + 1) * (p3 + 1);
    vector<vector<int>> volume_ien(num_elements, vector<int>(nodes_per_elem));

    int elem_idx = 0;
    for (int e1 = 0; e1 < nf[0]; e1++) {
        for (int e2 = 0; e2 < nf[1]; e2++) {
            for (int e3 = 0; e3 < nf[2]; e3++) {
                int local_node = 0;
                for (int k1 = 0; k1 <= p1; k1++) {
                    int i1 = e1 + k1;
                    for (int k2 = 0; k2 <= p2; k2++) {
                        int i2 = (e2 + k2) % ncp2;
                        for (int k3 = 0; k3 <= p3; k3++) {
                            int i3 = e3 + k3;
                            volume_ien[elem_idx][local_node] = i1 * (ncp2 * ncp3) + i2 * ncp3 + i3;
                            local_node++;
                        }
                    }
                }
                elem_idx++;
            }
        }
    }
    write_2d_array(dirpath / "volume.bin", volume_ien);

    vector<int> face_u_min, face_u_max, face_w_min, face_w_max;
    for (int i1 = 0; i1 < ncp1; i1++) {
        for (int i2 = 0; i2 < ncp2; i2++) {
            for (int i3 = 0; i3 < ncp3; i3++) {
                int global_cp_idx = i1 * (ncp2 * ncp3) + i2 * ncp3 + i3;
                if (i1 == 0) face_u_min.push_back(global_cp_idx);
                if (i1 == ncp1 - 1) face_u_max.push_back(global_cp_idx);
                if (i3 == 0) face_w_min.push_back(global_cp_idx);
                if (i3 == ncp3 - 1) face_w_max.push_back(global_cp_idx);
            }
        }
    }
    write_1d_array(dirpath / "face_u_min.bin", face_u_min);
    write_1d_array(dirpath / "face_u_max.bin", face_u_max);
    write_1d_array(dirpath / "face_w_min.bin", face_w_min);
    write_1d_array(dirpath / "face_w_max.bin", face_w_max);
}
#include <fstream>
#include <iostream>

#include "domain.h"
#include "init.h"
#include "json.h"

using namespace std;
using json        = nlohmann::json;
using orderd_json = nlohmann::ordered_json;
namespace fs      = std::filesystem;

void Domain::print() {
  cout << "======================= Domain Configuration =====================" << endl;
  cout << "=== core ===" << endl;
  cout << "device: " << this->device << endl;
  cout << boolalpha << "restart: " << this->restart << endl;
  cout << "iter: " << this->iter << endl;
  cout << "iter_max: " << this->iter_max << endl;
  cout << "time: " << this->time << endl;
  cout << "time_max: " << this->time_max << endl;
  cout << "dt: " << this->dt << endl;
  cout << "config_file: " << this->config_file << endl;
  cout << "results_dir: " << this->results_dir << endl;

  cout << "=== config ===" << endl;
  cout << "mesh_length: " << this->mesh_length << endl;
  cout << "mesh_shape: " << this->mesh_shape << endl;
  cout << "lsp_shape: " << this->lsp_shape << endl;
  cout << "dirichlet_bc" << endl;
  cout << boolalpha << "(e, n, w, s, u, d): ("
       << this->dirichlet_bc[0] << ", " << this->dirichlet_bc[1] << ", "
       << this->dirichlet_bc[2] << ", " << this->dirichlet_bc[3] << ", "
       << this->dirichlet_bc[4] << ", " << this->dirichlet_bc[5] << ")" << endl;

  cout << "newton_max: " << this->newton_max << endl;
  cout << "newton_tole: " << this->newton_tole << endl;

  cout << "=== material parameters ===" << endl;
//   cout << "neo_mu: " << this->neo_mu << endl;


//   cout << "=== consititution ===" << endl;
//   if (this->neoHookean)
//     cout << "neoHookean" << endl;
//   if (this->passive)
//     cout << "passive" << endl;

  cout << "ng: " << this->ng << endl;

  cout << "=== output parameters ===" << endl;
  cout << "output_idx: " << this->output_idx << endl;
  cout << "output_div: " << this->output_div << endl;
  cout << "output_interval: " << this->output_interval << endl;
  cout << "output_restart_interval: " << this->output_restart_interval << endl;

  cout << "=================================================================";
  cout << "=================================================================" << endl;
}

void Domain::write_config(fs::path const& dirpath) {
  fs::path filepath = dirpath / "config.json";
  orderd_json m_json;
  ofstream ofs(filepath);
  if (ofs.fail()) {
    cerr << "Could not open the config file to write: " << filepath << endl;
    exit(EXIT_FAILURE);
  }

  m_json["iter"]     = iter;
  m_json["iter_max"] = iter_max;
  m_json["time"]     = time;
  m_json["time_max"] = time_max;
  m_json["dt"]       = dt;

  m_json["mesh_length"][0]  = mesh_length[0];
  m_json["mesh_length"][1]  = mesh_length[1];
  m_json["mesh_length"][2]  = mesh_length[2];
  m_json["mesh_shape"]      = mesh_shape;
  m_json["lsp_shape"]       = lsp_shape;
  m_json["dirichlet_bc"][0] = dirichlet_bc[0];
  m_json["dirichlet_bc"][1] = dirichlet_bc[1];
  m_json["dirichlet_bc"][2] = dirichlet_bc[2];
  m_json["dirichlet_bc"][3] = dirichlet_bc[3];
  m_json["dirichlet_bc"][4] = dirichlet_bc[4];
  m_json["dirichlet_bc"][5] = dirichlet_bc[5];

  m_json["newton_max"]  = newton_max;
  m_json["newton_tole"] = newton_tole;

  m_json["neo_mu"]          = neo_mu;

  m_json["ng"] = ng;

  m_json["output_idx"]              = output_idx;
  m_json["output_div"]              = output_div;
  m_json["output_interval"]         = output_interval;
  m_json["output_restart_interval"] = output_restart_interval;

  ofs << m_json.dump(4) << endl;
  ofs.close();
}

void Domain::read_config() {
  ifstream ifs(config_file);
  if (ifs.fail()) {
    cerr << "Could not open the config file: " << config_file << endl;
    exit(EXIT_FAILURE);
  }
  json m_json = json::parse(ifs);
  ifs.close();

  this->iter     = m_json["iter"];
  this->time     = m_json["time"];
  this->time_max = m_json["time_max"];
  this->dt       = m_json["dt"];
  this->iter_max = (this->time_max - this->time) / this->dt + this->iter;

  this->mesh_length[0]  = m_json["mesh_length"][0];
  this->mesh_length[1]  = m_json["mesh_length"][1];
  this->mesh_length[2]  = m_json["mesh_length"][2];
  this->mesh_shape      = m_json["mesh_shape"];
  this->lsp_shape       = m_json["lsp_shape"];
  this->dirichlet_bc[0] = m_json["dirichlet_bc"][0];
  this->dirichlet_bc[1] = m_json["dirichlet_bc"][1];
  this->dirichlet_bc[2] = m_json["dirichlet_bc"][2];
  this->dirichlet_bc[3] = m_json["dirichlet_bc"][3];
  this->dirichlet_bc[4] = m_json["dirichlet_bc"][4];
  this->dirichlet_bc[5] = m_json["dirichlet_bc"][5];

  this->newton_max  = m_json["newton_max"];
  this->newton_tole = m_json["newton_tole"];

  this->neo_mu          = m_json["neo_mu"];

  this->ng = m_json["ng"];

  this->output_idx              = m_json["output_idx"];
  this->output_div              = m_json["output_div"];
  this->output_interval         = m_json["output_interval"];
  this->output_restart_interval = m_json["output_restart_interval"];

}

void Domain::time_print(const int div) {
  static string name[2] = {
      "Struct",
      "AMR"};
  int num_name = 2;
  if (this->iter % (this->output_interval / div) == 0) {
    float total_time = 0.0;
    auto now         = chrono::system_clock::now();
    time_t end_time  = chrono::system_clock::to_time_t(now);
    for (int i = 0; i < num_name; i++)
      total_time += this->elapsed_time[i];

    printf("===============time print================\n");
    printf("Date: %s", ctime(&end_time));
    printf("iter: %d, time: %.3e [s]\n", this->iter, total_time / 1000);
    for (int i = 0; i < num_name; i++) {
      printf("%s: %.3e [s] (%6.2f %%)\n", name[i].c_str(), this->elapsed_time[i] / 1000, this->elapsed_time[i] / total_time * 100);
      
    }
    printf("=========================================\n");
    for (int i = 0; i < num_name; i++)
      this->elapsed_time[i] = 0;
    fflush(stdout);
  }
}


Domain::Domain(const bool& nop){
    if(nop) return;

    this->restart     = Init::restart;
    this->device      = Init::device;
    this->config_file = Init::config;

    if (this->config_file.string() == "config") {
        this->results_dir = "results";
    } else {
        this->results_dir = "results" / this->config_file;
    }

    if (!fs::exists(this->results_dir / "cp"))
        fs::create_directories(this->results_dir / "cp");
    if (!fs::exists(this->results_dir / "gp"))
        fs::create_directories(this->results_dir / "gp");
    if (!fs::exists(this->results_dir / "otherdata"))
        fs::create_directories(this->results_dir / "otherdata");

    if (this->restart) {
        this->config_file = this->results_dir / "bin/restart/config.json";
        read_config();
    } else {
        string tmp        = "config/" + this->config_file.string() + ".json";
        this->config_file = tmp;
        read_config();
    }
    print();

    #if defined(OUTPUT_ASCII)
    this->xdmf_format = xdmf3::Format::ASCII;
    #elif defined(OUTPUT_BINARY)
    this->xdmf_format = xdmf3::Format::Binary;
    #elif defined(OUTPUT_HDF)
    this->xdmf_format = xdmf3::Format::HDF;
    #endif

    if(xdmf_format == xdmf3::Format::Unknown){
        cerr << "Error: Unknown XDMF format. Please define OUTPUT_ASCII, OUTPUT_BINARY, or OUTPUT_HDF." << endl;
        exit(EXIT_FAILURE);
    }

    this->other_datasets_restart = this->restart;
}
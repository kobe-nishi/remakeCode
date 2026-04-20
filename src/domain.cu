#include <fstream>
#include <iostream>

#include "mydef.h"

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

//   cout << "=== config ===" << endl;
//   cout << "mesh_length: " << this->mesh_length << endl;
//   cout << "mesh_shape: " << this->mesh_shape << endl;
//   cout << "lsp_shape: " << this->lsp_shape << endl;
//   cout << "dirichlet_bc" << endl;
//   cout << boolalpha << "(e, n, w, s): ("
//        << this->dirichlet_bc[0] << ", " << this->dirichlet_bc[1] << ", "
//        << this->dirichlet_bc[2] << ", " << this->dirichlet_bc[3] << ")" << endl;

  cout << "newton_max: " << this->newton_max << endl;
  cout << "newton_tole: " << this->newton_tole << endl;

  cout << "=== material parameters ===" << endl;
//   cout << "neo_mu: " << this->neo_mu << endl;


  cout << "=== consititution ===" << endl;
  if (this->neoHookean)
    cout << "neoHookean" << endl;
  if (this->passive)
    cout << "passive" << endl;

//   cout << "=== Lanrange multiplier method ===" << endl;
//   cout << boolalpha << "lagrange_multiplier: " << this->lagrange_multiplier << endl;
//   cout << boolalpha << "constant_vol: " << this->constant_vol << endl;
//   cout << "target_vol: " << this->target_vol << endl;
//   cout << "lambda_vol: " << this->lambda_vol << endl;
//   cout << boolalpha << "constant_area: " << this->constant_area << endl;
//   cout << "target_area: " << this->target_area << endl;
//   cout << "lambda_area: " << this->lambda_area << endl;

//   cout << "=== AMR parameters ===" << endl;
//   cout << boolalpha << "amr: " << this->amr << endl;
//   cout << boolalpha << "refine: " << this->refine << endl;
//   cout << boolalpha << "coarsen: " << this->coarsen << endl;
//   cout << "adaptive_interval: " << this->adaptive_interval << endl;
//   cout << "base_ele_length: " << this->base_ele_length << endl;

//   cout << "ng: " << this->ng << endl;

//   cout << "=== output parameters ===" << endl;
//   cout << "output_idx: " << this->output_idx << endl;
//   cout << "output_div: " << this->output_div << endl;
//   cout << "output_interval: " << this->output_interval << endl;
//   cout << "output_restart_interval: " << this->output_restart_interval << endl;

//   cout << "=== growth points ===" << endl;
//   cout << "growth_point_num: " << this->growth_point_num << endl;
//   cout << "growth_point_idx: ";
//   for (int i = 0; i < this->growth_point_num; i++) {
//     cout << this->growth_point_idx[i] << " ";
//   }
  cout << endl;

  cout << "=================================================================";
  cout << "=================================================================" << endl;
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

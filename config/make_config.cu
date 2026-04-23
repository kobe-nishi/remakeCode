#include <cstring>
#include <iostream>
#include <regex>
#include <vector>

#include "mydef.h"

using namespace std;
using json         = nlohmann::json;
using ordered_json = nlohmann::ordered_json;
namespace fs       = std::filesystem;

/*=== Number of faces ===*/
#define NF1 16
#define NF2 16
#define NF3 8

/*=== Polynomial degrees ===*/
#define P1 3
#define P2 3
#define P3 3

/*=== Length ===*/
#define L1 1.0 
#define L2 1.0 
#define L3 0.5

/*=== choose shape ===*/
#define CUBE                // NCP, P, L (1: x-axis. 2, y-axis. 3, z-axis)
// #define FULL_TUBE_PERIODIC //NCP, P (1: Length, 2: Arc, 3: Radius), L (1: Length, 2:Outer Radius, 3: Inner Radius)

using namespace std;

class Mesh : public Domain{
    public:
        Mesh(const std::string& name){
            mesh_shape = name;
        };
        ~Mesh(){};

        void set_boundary_condition(Domain& dom){
            if(mesh_shape == "cube"){
                dom.dirichlet_bc[0] = true;
                dom.dirichlet_bc[1] = true;
                dom.dirichlet_bc[2] = true;
                dom.dirichlet_bc[3] = true;
                dom.dirichlet_bc[4] = true;
                dom.dirichlet_bc[5] = true;
            }
            else if (dom.mesh_shape == "tube") {
                // 1.(Axial)
                dom.dirichlet_bc[0] = true;  
                dom.dirichlet_bc[1] = true;  

                // 2.(Circumferential):
                dom.dirichlet_bc[2] = false; 
                dom.dirichlet_bc[3] = false; 

                // 3.(Radial):
                dom.dirichlet_bc[4] = false;  
                dom.dirichlet_bc[5] = false;  
            }
        }
};

class Cpw : public Double3 {
    public:
        double w;
        Cpw() : Double3(), w(1.0) {};
        Cpw(double x_, double y_, double z_, double w_ = 1.0)
            : Double3(x_, y_, z_), w(w_) {};
        ~Cpw() {};
};

void knot_insertion(vector<double>& knotvector, vector<double>& insert_knot, vector<vector<double>>& Cm, int ncp, int p) {
  double ins = insert_knot.front();
  int k;
  for (int i = 0; i < knotvector.size() - 1; i++) {
    if (knotvector[i] <= ins && ins < knotvector[i + 1]) {
      k = i;
      break;
    }
  }
  int m = ncp + 1;
  Cm.resize(ncp);
  for (auto& Cm_ : Cm) {
    Cm_.resize(m);
    memset(Cm_.data(), 0, sizeof(double) * m);
  }
  for (int i = 0; i < m; i++) {
    double alpha;
    if (i <= k - p) {
      alpha = 1.0;
    } else if (i <= k) {
      alpha = (ins - knotvector[i]) / (knotvector[i + p] - knotvector[i]);
    } else {
      alpha = 0.0;
    }
    if (i < ncp) {
      Cm[i][i] = alpha;
    }
    if (i > 0) {
      Cm[i - 1][i] = 1.0 - alpha;
    }
  }
  knotvector.insert(knotvector.begin() + k + 1, ins);
  insert_knot.erase(insert_knot.begin());
}

vector<vector<double>> get_matrix_for_degree_elevation_and_knot_insertion(const int ncp, const int p, int ncp_, int p_){
    vector<vector<double>> C = get_indentity_matrix_2d<double>(ncp_);
    while(p_ < p){
        // Degree elevation
        vector<vector<double>> Cm(ncp_, vector<double>(ncp_ + 1, 0));
        for(int i = 0; i < ncp_ + 1; i++){
                if(i == 0){
                    Cm[0][i] = 1.0;
                }
                else if ( i == ncp_){
                    Cm[ncp_ - 1][i] = 1.0;
                }
                else{
                    double alpha = (double)i / (double)(ncp_);
                        Cm[i][i]     = 1 - alpha;
                        Cm[i - 1][i] = alpha;
                }
            }

            C = MM_product_2d(C,Cm);
            p_++;
            ncp_++;
     }

    // Knot insertion
    vector<double> knotvector(2 * ncp_);
    fill(knotvector.begin(), knotvector.begin() + ncp_, 0.0);
    fill(knotvector.begin() + ncp_, knotvector.end(),   1.0);
    int nins = ncp - ncp_;
    vector<double> inset_knot(nins);
    for(int i = 0; i < nins; i++){
        inset_knot[i] = (double)(i + 1) / (double)(nins + 1);
    }

    while (!inset_knot.empty()){
        vector<vector<double>> Cm;
        knot_insertion(knotvector, inset_knot,Cm,ncp_,p);
        C = MM_product_2d(C,Cm);
        ncp_++;
    }
    return C;
}

void set_line_cpw(vector<Cpw>& cpw, const int& p, const int& ncp) {
    int p_   = 1;
    int ncp_ = 2;
    Vector<Cpw> cpw_(2);
    cpw_[0].x = 0.0;
    cpw_[0].w = 1.0;
    cpw_[1].x = 1.0;
    cpw_[1].w = 1.0;

    vector<vector<double>> C = get_matrix_for_degree_elevation_and_knot_insertion(ncp,p,ncp_,p_);
    cpw.resize(ncp);
    for(int i = 0; i < ncp; i++){
        cpw[i].w = 0.0;
        for(int j = 0; j < ncp_; j++){
            cpw[i].x += C[j][i] * cpw_[j].x;
            cpw[i].w += C[j][i] * cpw_[j].w;
        }
    }

}

void set_arc_cpw(vector<Cpw>& cpw, const int& p, const int& ncp, const double& arc) {
    int p_   = 2;
    int ncp_ = 3;
    vector<Cpw> cpw_(ncp_);

    cpw_[0] = {1.0, 0.0, 0.0, 1.0};
    cpw_[1] = {1.0, tan(arc / 2.0), 0.0, cos(arc / 2.0)};
    cpw_[2] = {cos(arc), sin(arc), 0.0, 1.0};

    vector<vector<double>> C = get_matrix_for_degree_elevation_and_knot_insertion(ncp, p, ncp_, p_);
    
    cpw.resize(ncp);
    for (int i = 0; i < ncp; i++) {
        cpw[i].w = 0.0;
        double tmp_x = 0.0;
        double tmp_y = 0.0;

        for (int j = 0; j < ncp_; j++) {
            tmp_x += C[j][i] * (cpw_[j].x * cpw_[j].w);
            tmp_y += C[j][i] * (cpw_[j].y * cpw_[j].w);
            cpw[i].w += C[j][i] * cpw_[j].w;
        }

        cpw[i].x = tmp_x / cpw[i].w;
        cpw[i].y = tmp_y / cpw[i].w;
        cpw[i].z = 0.0;
    }
}
void set_cube_cpw(vector<Cpw>& cpw, Domain& dom, Spline& spl){
    int ncp1 = spl.nf[0] + spl.p[0];
    int ncp2 = spl.nf[1] + spl.p[1];
    int ncp3 = spl.nf[2] + spl.p[2];

    vector<Cpw> cpw_x, cpw_y, cpw_z;
    set_line_cpw(cpw_x, spl.p[0],ncp1);
    set_line_cpw(cpw_y, spl.p[1],ncp2);
    set_line_cpw(cpw_z, spl.p[2],ncp3);

    cpw.resize(ncp1 * ncp2 * ncp3);

    for (int i1 = 0; i1 < ncp1; i1++) {
        for (int i2 = 0; i2 < ncp2; i2++) {
            for (int i3 = 0; i3 < ncp3; i3++) {
                int idx    = i1 * ncp2 * ncp3 + i2 * ncp3 + i3;
                cpw[idx].w = cpw_x[i1].w * cpw_y[i2].w * cpw_z[i3].w;
                cpw[idx].x = dom.mesh_length[0] * cpw_x[i1].x;
                cpw[idx].y = dom.mesh_length[1] * cpw_y[i2].x;
                cpw[idx].z = dom.mesh_length[2] * cpw_z[i3].x;
            }
        }
    }
}

void set_tube_cpw(vector<Cpw>& cpw, Domain& dom, Spline& spl){
    int ncp1 = spl.nf[0] + spl.p[0];
    int ncp2 = spl.nf[1];
    int ncp3 = spl.nf[2] + spl.p[2];

    vector<Cpw> cpw_len, cpw_ang, cpw_rad;
    set_line_cpw(cpw_len, spl.p[0],ncp1);
    set_arc_cpw(cpw_ang, spl.p[1],ncp2, dom.mesh_length[1] * M_PI);
    set_line_cpw(cpw_rad, spl.p[2],ncp3);

    cpw.resize(ncp1 * ncp2 * ncp3);
    for (int i1 = 0; i1 < ncp1; i1++) {
        for (int i2 = 0; i2 < ncp2; i2++) {
            for (int i3 = 0; i3 < ncp3; i3++) {
                int idx    = i1 * ncp2 * ncp3 + i2 * ncp3 + i3;

                cpw[idx].w = cpw_len[i1].w * cpw_ang[i2].w * cpw_rad[i3].w;

                double r_inner = dom.mesh_length[2];
                double r_outer = dom.mesh_length[1];
                double R       = r_inner + (r_outer - r_inner) * cpw_rad[i3].x;
                cpw[idx].x = R * cpw_ang[i2].x;
                cpw[idx].y = dom.mesh_length[0] * cpw_len[i1].x;
                cpw[idx].z =R * cpw_ang[i2].y;
            }
        }
    }
}


void set_init_cp(vector<Cpw>& cpw, Spline& spl, Domain& dom){

#if defined(CUBE)
        set_cube_cpw(cpw, dom, spl);
#elif defined(FULL_TUBE_PERIODIC)
        set_tube_cpw(cpw, dom, spl);
#else
    if(dom.mesh_shape.empty()){
        cerr << "Error: Mesh shape is not defined" << endl;
        exit(1);
    }
#endif

    cout << "Mesh shape: \"" << dom.mesh_shape << "\"" << endl;

    int nn = (int)cpw.size();
    spl.cp.resize(nn);
    for(auto i = 0; i < nn; i++){
        double w         = cpw[i].w;
        spl.cp[i].weight = w;
        spl.cp[i].pos    = cpw[i] * w;
        spl.cp[i].diffu  = 0.0;
    }
}

void make_config_data(Domain& dom, Spline& spl){
    fs::path file_path = "./config";

    if(!fs::exists(file_path)) fs::create_directory(file_path);

    dom.write_config(file_path);
    spl.write_config(file_path);
}

void make_mydef_header(Spline& spl) {
  ifstream ifs("./include/template_mydef.h");
  if (!ifs) {
    cerr << "Error: Could not open template_mydef.h" << endl;
    exit(1);
  }
  string tmp((istreambuf_iterator<char>(ifs)),
             istreambuf_iterator<char>());

  auto replace_all = [](string& str, const string& from, const string& to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != string::npos) {
      str.replace(start_pos, from.length(), to);
      start_pos += to.length();
    }
  };

  replace_all(tmp, "@DEGP@", to_string(spl.p[0]));
  replace_all(tmp, "@DEGQ@", to_string(spl.p[1]));
  replace_all(tmp, "@DEGR@", to_string(spl.p[2]));
  regex reP(R"(#define\s+DEG_P\s+\d+)");
  regex reQ(R"(#define\s+DEG_Q\s+\d+)");
  regex reR(R"(#define\s+DEG_R\s+\d+)");
  tmp = regex_replace(tmp, reP, "#define DEG_P " + to_string(spl.p[0]));
  tmp = regex_replace(tmp, reQ, "#define DEG_Q " + to_string(spl.p[1]));
  tmp = regex_replace(tmp, reR, "#define DEG_R " + to_string(spl.p[2]));

  if (fs::exists("include/mydef.h")) {
    ifstream current ("include/mydef.h");
    if (current) {
      string current_content((istreambuf_iterator<char>(current)),
                             istreambuf_iterator<char>());
      if (current_content == tmp) {
        cout << "No changes in \"include/mydef.h\"" << endl;
        return;
      }
    }
  }

  ofstream ofs("include/mydef.h");
  if (!ofs) {
    cerr << "Error: Could not open mydef.h for writing" << endl;
    exit(1);
  }
  ofs << tmp;
  cout << "Generated mydef.h with DEG_P=" << spl.p[0] << ", DEG_Q=" << spl.p[1] << ", DEG_R=" << spl.p[2] << endl;
}

int main(){
    Domain dom(true);
    Spline spl;
    Mesh mesh;

    vector<Cpw> cpw;

    dom.mesh_length[0] = L1;
    dom.mesh_length[1] = L2;
    dom.mesh_length[2] = L3;

    spl.p[0] = P1;
    spl.p[1] = P2;
    spl.p[2] = P3;

    spl.nf[0] = NF1;
    spl.nf[1] = NF2;
    spl.nf[2] = NF3;

    #if defined(CUBE)
      Mesh mesh("cube");
    #elif defined(FULL_TUBE_PERIODIC)
      Mesh mesh("tube");
    #endif

    set_init_cp(cpw,spl,dom);

    mesh.set_boundary_condition(dom);

    make_config_data(dom, spl);

    make_mydef_header();

    return 0;
}
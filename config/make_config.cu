#include <cstring>
#include <iostream>
#include <regex>
#include <vector>

#include "mydef.h"

using namespace std;
using json         = nlohmann::json;
using ordered_json = nlohmann::ordered_json;
namespace fs       = std::filesystem;

/*=== Number of control points ===*/
#define NCP1 16
#define NCP2 16
#define NCP2 8

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

class Cpw : public Double3 {
    public:
        double w;
        Cpw() : Double3(), w(1.0) {};
        Cpw(double x_, double y_, double z_, double w_ = 1.0)
            : Double3(x_, y_, z_), w(w_) {};
        ~Cpw() {};
};


void set_init_cp(Spline& spl, Domain& dom){
    vector<Cpw> cpw;                                                                             

#if defined(CUBE)
        set_cpw(cpw, dom, spl);
#elif defined(FULL_TUBE_PERIODIC)
        set_tube_cpw(cpw, dom, spl);
#elif
    if(dom.mesh_shape.empty()){
        cerr << "Error: Mesh shape is not defined" << endl;
        exit(1);
    }

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
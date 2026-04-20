#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "mydef.h"

#define THREAD 128

using namespace std;

using json        = nlohmann::json;
using orderd_json = nlohmann::ordered_json;
namespace fs      = std::filesystem;


Output::Output(Iga& iga, Domain& dom){
    if(dom.iter % dom.output_interval == 0)
    {
        cudaDeviceSynchronize();
        dom.output_idx++;
    }
    if(dom.iter % (dom.output_interval * dom.output_restart_interval) == 0)
    {

    }
}
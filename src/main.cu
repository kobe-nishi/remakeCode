#include <cuda.h>
#include <cuda_runtime.h>
#include <iostream>

#include "mydef.h"

using namespace std;

int main(int argc, char** argv) {

  Init(argc, argv);
  Domain& dom = *(new Domain());
  Iga& iga    = *(new Iga(dom));
  if (!dom.restart) {
    Output(iga, dom);
  }

  dom.time += dom.dt;
  dom.iter++;

//   for (; dom.iter <= dom.iter_max; dom.iter++, dom.time += dom.dt) {
//     std::cout << "Iter: " << dom.iter << ", time: " << dom.time << std::endl;
//     Struct(iga, dom);
//     iga.amr(dom);
//     Output(iga, dom);
//     dom.time_print();
//   }
}
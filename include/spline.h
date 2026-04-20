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



class Spline {
    public:
        int p[3]{0}; //Polynominal degree of nurbs basis
        int nf[3]{0}; //Number of faces
};
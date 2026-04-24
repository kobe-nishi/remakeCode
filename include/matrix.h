#pragma once

#include <vector>

class Csr {
  public:
  int n           = 0;       // Number of rows
  int nnz         = 0;       // Number of non-zero elements
  int* row_ptr    = nullptr; // starting index of each row in col_idx and A
  int* col_idx    = nullptr; // column indices of non-zero elements
  int* loc_to_glb = nullptr;
  double* A       = nullptr;
  double* x       = nullptr;
  double* b       = nullptr;

  public:
//   void malloc();
//   void print_pattern();
//   void print_val();
//   void set_dof_1d(const std::vector<std::vector<int>>& neighbor);
//   void set_diagonal_dof_3d(const std::vector<std::vector<int>>& neighbor);
//   void set_full_dof_3d(const std::vector<std::vector<int>>& neighbor);
};
#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <thrust/device_ptr.h>
#include <thrust/transform_reduce.h>
#include <thrust/inner_product.h>

template <typename T>
T thrust_norm2(T* value, const int n) {
  thrust::device_ptr<T> start(value);
  thrust::device_ptr<T> end(start + n);
  return sqrt(thrust::transform_reduce(
      start, end, thrust::square<T>(), 0.0, thrust::plus<T>()));
}

template <typename T>
T thrust_dot(T* vec1, T* vec2, const int n) {
  thrust::device_ptr<T> start1(vec1);
  thrust::device_ptr<T> end1(start1 + n);
  thrust::device_ptr<T> start2(vec2);
  return thrust::inner_product(start1, end1, start2, T(0));
}

template <typename T>
void thrust_scale(T* vec, const T alpha, const int n) {
  thrust::device_ptr<T> start(vec);
  thrust::device_ptr<T> end(start + n);  
  thrust::transform(start, end, thrust::make_constant_iterator<T>(alpha), start, thrust::multiplies<T>());
}

template <typename T>
void thrust_plus_assign(T* vec1, T* vec2, const int n) {
  thrust::device_ptr<T> start_1(vec1);
  thrust::device_ptr<T> end_1(start_1 + n);

  thrust::device_ptr<const T> start_2(vec2);
  thrust::transform(start_1, end_1, start_2, start_1, thrust::plus<T>());
}

__host__ __device__ inline int combi(int n, int r) {
  if (r > n - r)
    r = n - r;
  int ans = 1;
  for (int i = 0; i < r; i++) {
    ans *= (n - i);
    ans /= (i + 1);
  }
  return ans;
}

template <typename T>
int my_bit_width(T x) {
  int width = 0;
  while (x > 0) {
    x >>= 1;
    width++;
  }
  return width;
}

inline bool is_inside(int idx, int thickness, int total) {
  return thickness <= idx && idx < total - thickness - 1;
}

inline bool is_outside(int idx, int thickness, int total) {
  return idx < thickness || total - thickness <= idx;
}

inline bool is_pow_of_two(int n) {
  return (n & (n - 1)) == 0 && n > 0;
}

template <typename T>
void write_json(nlohmann::json& m_json, std::string key, int dim, bool is_offset, T* data, int num) {
  if (!m_json.contains("root")) {
    std::cerr << "root is not found in json." << std::endl;
    exit(EXIT_FAILURE);
  }
  std::filesystem::path filepath, root = m_json["root"].get<std::string>();
  if (dim == -1) {
    if (is_offset) {
      filepath                     = std::string(key + "_offset.bin");
      m_json[key]["offset"]["dir"] = filepath.string();
      m_json[key]["offset"]["dim"] = num;
    } else {
      filepath           = std::string(key + ".bin");
      m_json[key]["dir"] = filepath.string();
      m_json[key]["dim"] = num;
    }
  } else {
    if (is_offset) {
      filepath                          = std::string(key + std::to_string(dim + 1) + "_offset.bin");
      m_json[key][dim]["offset"]["dir"] = filepath.string();
      m_json[key][dim]["offset"]["dim"] = num;
    } else {
      filepath                = std::string(key + std::to_string(dim + 1) + ".bin");
      m_json[key][dim]["dir"] = filepath.string();
      m_json[key][dim]["dim"] = num;
    }
  }

  filepath = root / filepath;
  std::ofstream ofs(filepath, std::ios::out | std::ios::binary);
  if (ofs.fail()) {
    std::cerr << filepath << "Could not open the file: " << filepath << std::endl;
    exit(EXIT_FAILURE);
  }
  ofs.write(reinterpret_cast<char*>(data), sizeof(T) * num);
  ofs.close();
}

template <typename T>
void read_json(nlohmann::json& m_json, std::string key, int dim, bool is_offset, T* data) {
  std::string filepath;
  std::string root = m_json["root"].get<std::string>() + "/";
  int num;

  if (dim == -1) {
    filepath = root + (is_offset ? m_json[key]["offset"]["dir"].get<std::string>() : m_json[key]["dir"].get<std::string>());
    if (m_json[key]["dim"].is_array())
      num = m_json[key]["dim"][2];
    else
      num = m_json[key]["dim"];
  } else {
    filepath = root + (is_offset ? m_json[key][dim]["offset"]["dir"].get<std::string>() : m_json[key][dim]["dir"].get<std::string>());
    num      = (is_offset ? m_json[key][dim]["offset"]["dim"].get<int>() : m_json[key][dim]["dim"].get<int>());
  }

  std::ifstream ifs(filepath, std::ios::in | std::ios::binary);
  if (ifs.fail()) {
    std::cerr << "Could not open the file: " << filepath << std::endl;
    exit(EXIT_FAILURE);
  }
  ifs.read(reinterpret_cast<char*>(data), sizeof(T) * num);
  ifs.close();
}

template <typename T>
void printVector(const std::vector<T>& V) {
  std::cout << std::boolalpha;
  std::cout << "(" << std::ends;
  for (auto i = 0; i < V.size(); i++) {
    std::cout << V[i] << std::ends;
    if (i != (int)V.size() - 1)
      std::cout << ", " << std::ends;
  }
  std::cout << ")" << std::endl;
}

template <typename T>
void printMatrix(const std::vector<T>& M, const int& row) {
  int col = M.size() / row;
  if (M.size() % row != 0) {
    std::cerr << "Matrix size is error. (printMatrix)" << std::endl;
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < row; i++) {
    std::cout << "[" << std::ends;
    for (int j = 0; j < col; j++) {
      std::cout << M[i * col + j] << " " << std::ends;
    }
    std::cout << "]" << std::endl;
  }
}

template <typename T>
void printMatrix_2d(const std::vector<std::vector<T>>& M) {
  for (auto i = 0; i < M.size(); i++) {
    std::cout << "[" << std::ends;
    for (auto j = 0; j < M[0].size(); j++) {
      std::cout << M[i][j] << " " << std::ends;
    }
    std::cout << "]" << std::endl;
  }
}

template <typename T>
std::vector<T> get_identity_tensor(int a) {
  std::vector<T> A(a * a);
  for (int i = 0; i < a; i++) {
    for (int j = 0; j < a; j++) {
      A[i * a + j] = (i == j);
    }
  }
  return A;
}

template <typename T>
std::vector<std::vector<T>> get_identity_tensor_2d(int a) {
  std::vector<std::vector<T>> A(a, std::vector<T>(a, 0));
  for (int i = 0; i < a; i++) {
    for (int j = 0; j < a; j++) {
      A[i][j] = (i == j);
    }
  }
  return A;
}

template <typename T>
std::vector<T>
MM_product(const std::vector<T>& A, const int& a_row,
           const std::vector<T>& B, const int& b_row) {
  int row_num = a_row;
  int col_num = B.size() / b_row;
  int iter    = A.size() / a_row;
  std::vector<T> C(row_num * col_num, 0);
  if (iter != b_row) {
    std::cerr << "Matrix size is error. (MM_product)" << std::endl;
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < row_num; i++) {
    for (int j = 0; j < col_num; j++) {
      T tmp = 0;
      for (int k = 0; k < iter; k++) {
        tmp += A[i * iter + k] * B[k * col_num + j];
      }
      C[i * col_num + j] = tmp;
    }
  }
  return C;
}

template <typename T>
__host__ __device__ inline void
MM_product(T* C, const T* A, const T* B,
           const int a_row, const int a_col,
           const int b_row, const int b_col) {
  for (int i = 0; i < a_row; i++) {
    for (int j = 0; j < b_col; j++) {
      T tmp = 0;
      for (int k = 0; k < a_col; k++) {
        tmp += A[i * a_col + k] * B[k * b_col + j];
      }
      C[i * b_col + j] = tmp;
    }
  }
}

template <typename T>
std::vector<std::vector<T>>
MM_product_2d(const std::vector<std::vector<T>>& A,
              const std::vector<std::vector<T>>& B) {
  int row_num = A.size();
  int col_num = B[0].size();
  int iter    = A[0].size();
  std::vector<std::vector<T>> C(row_num, std::vector<T>(col_num, 0));
  if (iter != (int)B.size()) {
    std::cerr << "Matrix size is error. (MM_product)" << std::endl;
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < row_num; i++) {
    for (int j = 0; j < col_num; j++) {
      double tmp = 0;
      for (int k = 0; k < iter; k++) {
        tmp += A[i][k] * B[k][j];
      }
      C[i][j] = tmp;
    }
  }

  return C;
}

template <typename T>
std::vector<T>
MV_product(const std::vector<T>& A, const int& a_row, const std::vector<T>& x) {
  int row_num = a_row;
  int col_num = A.size() / a_row;
  if (col_num != (int)x.size()) {
    std::cerr << "Matrix size is error. (MV_product)" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::vector<T> b(row_num, 0);
  for (int i = 0; i < row_num; i++) {
    T tmp = 0;
    for (int j = 0; j < col_num; j++) {
      tmp += A[i * col_num + j] * x[j];
    }
    b[i] = tmp;
  }
  return b;
}

template <typename T>
__host__ __device__ inline void
MV_product(T* b, const T* A, const T* x,
           const int a_row, const int a_col) {
  for (int i = 0; i < a_row; i++) {
    T tmp = 0;
    for (int j = 0; j < a_col; j++) {
      tmp += A[i * a_col + j] * x[j];
    }
    b[i] = tmp;
  }
}

template <typename T>
std::vector<T> MV_product_2d(const std::vector<std::vector<T>>& A, const std::vector<T>& x) {
  int row_num = A.size();
  int col_num = A[0].size();
  if (col_num != (int)x.size()) {
    std::cerr << "Matrix size is error. (MV_product)" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::vector<T> b(row_num, 0);
  for (int i = 0; i < row_num; i++) {
    T tmp = 0;
    for (int j = 0; j < col_num; j++) {
      tmp += A[i][j] * x[j];
    }
    b[i] = tmp;
  }
  return b;
}

template <typename T>
std::vector<T>
VM_product(const std::vector<T>& x,
           const std::vector<T>& A, const int& a_row) {
  int row_num = a_row;
  int col_num = A.size() / a_row;
  if (row_num != (int)x.size()) {
    std::cerr << "Matrix size is error. (VM_product)" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::vector<T> b(col_num);
  for (int i = 0; i < col_num; i++) {
    T tmp = 0;
    for (int j = 0; j < row_num; j++) {
      tmp += x[j] * A[j * col_num + i];
    }
    b[i] = tmp;
  }
  return b;
}

template <typename T>
__host__ __device__ inline void
VM_product(T* b, const T* x, const T* A,
           const int a_row, const int a_col) {
  for (int i = 0; i < a_col; i++) {
    T tmp = 0;
    for (int j = 0; j < a_row; j++) {
      tmp += x[j] * A[j * a_col + i];
    }
    b[i] = tmp;
  }
}

template <typename T>
std::vector<T>
VM_product_2d(const std::vector<T>& x,
              const std::vector<std::vector<T>>& A) {
  int row_num = A.size();
  int col_num = A[0].size();
  if (row_num != (int)x.size()) {
    std::cerr << "Matrix size is error. (VM_product)" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::vector<T> b(col_num);
  for (int i = 0; i < col_num; i++) {
    T tmp = 0;
    for (int j = 0; j < row_num; j++) {
      tmp += x[j] * A[j][i];
    }
    b[i] = tmp;
  }
  return b;
}

template <typename T>
std::vector<T> MM_tensor_product(const std::vector<T>& A, const int& a_row,
                                 const std::vector<T>& B, const int& b_row) {
  int a_col = A.size() / a_row;
  int b_col = B.size() / b_row;
  std::vector<T> C(a_row * b_row * a_col * b_col);
  int c_col = a_col * b_col;
  for (int i1 = 0; i1 < a_row; i1++) {
    for (int i2 = 0; i2 < b_row; i2++) {
      int row_idx = i1 * b_row + i2;
      for (int j1 = 0; j1 < a_col; j1++) {
        for (int j2 = 0; j2 < b_col; j2++) {
          int col_idx = j1 * b_col + j2;
          int idx     = row_idx * c_col + col_idx;
          C[idx]      = A[i1 * a_col + j1] * B[i2 * b_col + j2];
        }
      }
    }
  }
  return C;
}

template <typename T>
__host__ __device__ inline void
MM_tensor_product(T* C, const T* A, const T* B,
                  const int a_row, const int a_col,
                  const int b_row, const int b_col) {
  int c_col = a_col * b_col;
  for (int i1 = 0; i1 < a_row; i1++) {
    for (int i2 = 0; i2 < b_row; i2++) {
      int row_idx = i1 * b_row + i2;
      for (int j1 = 0; j1 < a_col; j1++) {
        for (int j2 = 0; j2 < b_col; j2++) {
          int col_idx = j1 * b_col + j2;
          int idx     = row_idx * c_col + col_idx;
          C[idx]      = A[i1 * a_col + j1] * B[i2 * b_col + j2];
        }
      }
    }
  }
}

template <typename T>
std::vector<std::vector<T>>
MM_tensor_product_2d(const std::vector<std::vector<T>>& A,
                     const std::vector<std::vector<T>>& B) {
  int a_row = A.size();
  int a_col = A[0].size();
  int b_row = B.size();
  int b_col = B[0].size();
  std::vector<std::vector<T>> C(a_row * b_row, std::vector<T>(a_col * b_col));
  for (int i1 = 0; i1 < a_row; i1++) {
    for (int i2 = 0; i2 < b_row; i2++) {
      int row_idx = i1 * b_row + i2;
      for (int j1 = 0; j1 < a_col; j1++) {
        for (int j2 = 0; j2 < b_col; j2++) {
          int col_idx         = j1 * b_col + j2;
          C[row_idx][col_idx] = A[i1][j1] * B[i2][j2];
        }
      }
    }
  }
  return C;
}

template <typename T>
std::vector<T> get_trans_tensor(const std::vector<T>& A, const int& a_row) {
  int a_col = A.size() / a_row;
  std::vector<T> nA(a_col * a_row);
  for (int i = 0; i < a_row; i++) {
    for (int j = 0; j < a_col; j++) {
      nA[j * a_row + i] = A[i * a_col + j];
    }
  }
  return nA;
}

template <typename T>
__host__ __device__ inline void
get_trans_tensor(T* nA, const T* A,
                 const int a_row, const int a_col) {
  for (int i = 0; i < a_row; i++) {
    for (int j = 0; j < a_col; j++) {
      nA[j * a_row + i] = A[i * a_col + j];
    }
  }
}

template <typename T>
std::vector<std::vector<T>> get_trans_tensor_2d(const std::vector<std::vector<T>>& A) {
  int arow = A.size();
  int acol = A[0].size();
  std::vector<std::vector<T>> nA(acol, std::vector<T>(arow));
  for (int i = 0; i < arow; i++) {
    for (int j = 0; j < acol; j++) {
      nA[j][i] = A[i][j];
    }
  }
  return nA;
}

template <typename T>
std::vector<T> VV_tensor_product(const std::vector<T>& a, const std::vector<T>& b) {
  int a_size = a.size();
  int b_size = b.size();
  std::vector<T> c(a_size * b_size);
  for (int i = 0; i < a_size; i++) {
    for (int j = 0; j < b_size; j++) {
      c[i * b_size + j] = a[i] * b[j];
    }
  }
  return c;
}

template <typename T>
__host__ __device__ inline void
VV_tensor_product(T* c, const T* a, const T* b,
                  const int a_size, const int b_size) {
  for (int i = 0; i < a_size; i++) {
    for (int j = 0; j < b_size; j++) {
      c[i * b_size + j] = a[i] * b[j];
    }
  }
}

template <typename T>
std::vector<T>
solve_gauss_elimination(std::vector<T> A, int a_row, std::vector<T> b) {
  int n = b.size();
  std::vector<T> x(n);
  // Partial pivoting
  for (int i = 0; i < n; i++) {
    T piv      = A[i * n + i];
    int pivIdx = i;
    for (int j = i + 1; j < n; j++) {
      if (fabs(piv) < fabs(A[j * n + i])) {
        piv    = A[j * n + i];
        pivIdx = j;
      }
    }
    if (piv == 0) {
      std::cerr << "This matrix can't be solved!!" << std::endl;
      exit(EXIT_FAILURE);
    }
    if (i != pivIdx) {
      // Swap rows in A
      for (int k = 0; k < n; k++) {
        std::swap(A[i * n + k], A[pivIdx * n + k]);
      }
      std::swap(b[i], b[pivIdx]);
    }
    // Forward elimination
    T piv_val = A[i * n + i];
    for (int k = 0; k < n; k++)
      A[i * n + k] /= piv_val;
    b[i] /= piv_val;
    for (int j = i + 1; j < n; j++) {
      T tmp = A[j * n + i];
      for (int k = i; k < n; k++) {
        A[j * n + k] -= tmp * A[i * n + k];
      }
      b[j] -= tmp * b[i];
    }
  }
  // Back substitution
  x[n - 1] = b[n - 1];
  for (int i = n - 2; i >= 0; i--) {
    T tmp = 0.0;
    for (int j = i + 1; j < n; j++) {
      tmp += A[i * n + j] * x[j];
    }
    x[i] = b[i] - tmp;
  }
  return x;
}

template <typename T>
__host__ __device__ inline void
solve_gauss_elimination(T* x, T* A, T* b, const int n) {
  if (n != (DEG_P + 1) * (DEG_Q + 1)) {
    printf("Error: Gauss elimination size mismatch. (%d, %d)\n", n, (DEG_P + 1) * (DEG_Q + 1));
  }
  for (int i = 0; i < n; i++) {
    T piv  = A[i * n  + i];
    int pivIdx = i;
    for (int j = i + 1; j < n; j++) {
      if (abs(piv) < abs(A[j * n + i])) {
        piv    = A[j * n + i];
        pivIdx = j;
      }
    }
    if (piv == 0) {
      printf("Error: This matrix can't be solved!!\n");
    }
    if (i != pivIdx) {
      // Swap rows in A
      for (int k = 0; k < n; k++) {
        T tmp = A[i * n + k];
        A[i * n + k] = A[pivIdx * n + k];
        A[pivIdx * n + k] = tmp;
      }
      T tmp = b[i];
      b[i] = b[pivIdx];
      b[pivIdx] = tmp;
    }
    // Forward elimination
    T piv_val = A[i * n + i];
    for (int k = 0; k < n; k++)
      A[i * n + k] /= piv_val;
    b[i] /= piv_val;
    for (int j = i + 1; j < n; j++) {
      T tmp = A[j * n + i];
      for (int k = i; k < n; k++) {
        A[j * n + k] -= tmp * A[i * n + k];
      }
      b[j] -= tmp * b[i];
    }
  }
  // Back substitution
  x[n - 1] = b[n - 1];
  for (int i = n - 2; i >= 0; i--) {
    T tmp = 0.0;
    for (int j = i + 1; j < n; j++) {
      tmp += A[i * n + j] * x[j];
    }
    x[i] = b[i] - tmp;
  }
}

template <typename T>
std::vector<T> solve_gauss_elimination_2d(std::vector<std::vector<T>> A, std::vector<T> b) {
  int n = b.size();
  std::vector<T> x(n, 0);
  for (int i = 0; i < n; i++) {
    // Partial pivoting
    T piv      = A[i][i];
    int pivIdx = i;
    for (int j = i + 1; j < n; j++) {
      if (fabs(piv) < fabs(A[j][i])) {
        piv    = A[j][i];
        pivIdx = j;
      }
    }
    if (piv == 0) {
      printf("This matrix can't be solved!!\n");
      exit(EXIT_FAILURE);
    }
    if (i != pivIdx) {
      T tmp;
      int tmpIdx;
      for (int j = 0; j < n; j++) {
        tmp          = A[i][j];
        A[i][j]      = A[pivIdx][j];
        A[pivIdx][j] = tmp;
      }
      tmp         = b[i];
      b[i]        = b[pivIdx];
      b[pivIdx]   = tmp;
    }
    // Forward elimination
    for (int j = 0; j < n; j++) {
      A[i][j] /= piv;
    }
    b[i] /= piv;
    for (int j = i + 1; j < n; j++) {
      T tmp = A[j][i];
      for (int k = i; k < n; k++) {
        A[j][k] -= tmp * A[i][k];
      }
      b[j] -= tmp * b[i];
    }
  }
  // Back substitution
  x[n - 1] = b[n - 1];
  for (int i = n - 2; i >= 0; i--) {
    T tmp = 0.0;
    for (int j = i + 1; j < n; j++) {
      tmp += A[i][j] * x[j];
    }
    b[i] -= tmp;
    x[i] = b[i];
  }

  return x;
}

inline std::vector<double> get_split_operatorA(double alpha, int p) {
  std::vector<double> S((p + 1) * (p + 1));
  for (int a = 0; a < p + 1; a++) {
    for (int b = 0; b < p + 1; b++) {
      if (a <= b && b <= p) {
        S[a * (p + 1) + b] = combi(b, a) * pow(alpha, a) * pow(1.0 - alpha, b - a);
      } else {
        S[a * (p + 1) + b] = 0.0;
      }
    }
  }
  return S;
}

inline std::vector<double> get_split_operatorB(double beta, int p) {
  std::vector<double> S((p + 1) * (p + 1));
  for (int a = 0; a < p + 1; a++) {
    for (int b = 0; b < p + 1; b++) {
      if (b <= a) {
        S[a * (p + 1) + b] = combi(p - b, a - b) * pow(beta, a - b) * pow(1.0 - beta, p - a);
      } else {
        S[a * (p + 1) + b] = 0.0;
      }
    }
  }
  return S;
}

inline std::vector<double> get_split_operatorAB(double max, double min, double max_, double min_, int p) {
  double L     = max - min;
  double alpha = max_ - min;
  double beta  = min_ - min;
  return MM_product(get_split_operatorA(alpha / L, p), p + 1,
                    get_split_operatorB(beta / alpha, p), p + 1);
}

__host__ __device__ inline void
set_split_operatorA(double* S, double alpha, int p) {
  for (int a = 0; a < p + 1; a++) {
    for (int b = 0; b < p + 1; b++) {
      if (a <= b && b <= p) {
        S[a * (p + 1) + b] = combi(b, a) * pow(alpha, a) * pow(1.0 - alpha, b - a);
      } else {
        S[a * (p + 1) + b] = 0.0;
      }
    }
  }
}

__host__ __device__ inline void
set_split_operatorB(double* S, double beta, int p) {
  for (int a = 0; a < p + 1; a++) {
    for (int b = 0; b < p + 1; b++) {
      if (b <= a) {
        S[a * (p + 1) + b] = combi(p - b, a - b) * pow(beta, a - b) * pow(1.0 - beta, p - a);
      } else {
        S[a * (p + 1) + b] = 0.0;
      }
    }
  }
}

__host__ __device__ inline void
set_split_operatorAB(double* S, double max, double min, double max_, double min_, int p) {
  int size     = p + 1;
  double L     = max - min;
  double alpha = max_ - min;
  double beta  = min_ - min;
  double SA[(MAX_PQ + 1) * (MAX_PQ + 1)];
  double SB[(MAX_PQ + 1) * (MAX_PQ + 1)];
  set_split_operatorA(SA, alpha / L, p);
  set_split_operatorB(SB, beta / alpha, p);
  MM_product(S, SA, SB, size, size, size, size);
}

inline std::vector<std::vector<double>> get_split_operatorA_2d(double alpha, int p) {
  std::vector<std::vector<double>> S(p + 1, std::vector<double>(p + 1, 0));
  for (int a = 0; a < p + 1; a++) {
    for (int b = 0; b < p + 1; b++) {
      if (a <= b && b <= p) {
        S.at(a).at(b) = combi(b, a) * pow(alpha, a) * pow(1.0 - alpha, b - a);
      } else {
        S.at(a).at(b) = 0.0;
      }
    }
  }
  return S;
}

inline std::vector<std::vector<double>> get_split_operatorB_2d(double beta, int p) {
  std::vector<std::vector<double>> S(p + 1, std::vector<double>(p + 1, 0));
  for (int a = 0; a < p + 1; a++) {
    for (int b = 0; b < p + 1; b++) {
      if (b <= a) {
        S.at(a).at(b) =
            combi(p - b, a - b) * pow(beta, a - b) * pow(1.0 - beta, p - a);
      } else {
        S.at(a).at(b) = 0.0;
      }
    }
  }
  return S;
}

inline std::vector<std::vector<double>> get_split_operatorAB_2d(double max, double min, double max_,
                                                                double min_, int p) {
  double L     = max - min;
  double alpha = max_ - min;
  double beta  = min_ - min;
  return MM_product_2d(get_split_operatorA_2d(alpha / L, p),
                       get_split_operatorB_2d(beta / alpha, p));
}
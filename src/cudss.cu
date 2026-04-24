#include <iostream>
#include <cmath>
#include <cassert>
#include <cuda.h>
#include <cuda_runtime.h>

#include <cudss.h>

#include "mydef.h"
        
#define CUDA_CALL_AND_CHECK(call, msg) \
    do { \
        cuda_error = call; \
        if (cuda_error != cudaSuccess) { \
            printf("Example FAILED: CUDA API returned error = %d, details: " #msg "\n", cuda_error); \
            return -1; \
        } \
    } while(0);
 
        
#define CUDSS_CALL_AND_CHECK(call, status, msg) \
    do { \
        status = call; \
        if (status != CUDSS_STATUS_SUCCESS) { \
            printf("Example FAILED: CUDSS call ended unsuccessfully with status = %d, details: " #msg "\n", status); \
            return -2; \
        } \
    } while(0);
 
// int cudss(double *p_a, int *row_ptr, int *col_idx, double *p_x, double *p_b, int nnz, int num_row) {
int cudss(Csr& csr) {
  static bool is_first = true;
  double *p_a = csr.A;
  int *row_ptr = csr.row_ptr;
  int *col_idx = csr.col_idx;
  double *p_x = csr.x;
  double *p_b = csr.b;
  int nnz = csr.nnz;
  int num_row = csr.n;
  cudaError_t cuda_error = cudaSuccess;
  cudssStatus_t cudss_status = CUDSS_STATUS_SUCCESS; 

  static cudaStream_t stream = nullptr;
  static cudssHandle_t handle;

  if (is_first) {
    CUDA_CALL_AND_CHECK(cudaStreamCreate(&stream), "Failed to create CUDA stream");
    CUDSS_CALL_AND_CHECK(cudssCreate(&handle), cudss_status, "Failed to create cuDSS handle");
    CUDSS_CALL_AND_CHECK(cudssSetStream(handle, stream), cudss_status, "Failed to set stream for cuDSS handle");
    is_first = false;
  }

  cudssConfig_t solver_config;
  cudssData_t   solver_data;

  CUDSS_CALL_AND_CHECK(cudssConfigCreate(&solver_config), cudss_status, "Failed to create cuDSS config");
  CUDSS_CALL_AND_CHECK(cudssDataCreate(handle, &solver_data), cudss_status, "Failed to create cuDSS data");

  cudssMatrix_t A, x, b;
  /* Create handlers for x and b */
  CUDSS_CALL_AND_CHECK(cudssMatrixCreateDn(&x, num_row, 1, num_row, p_x, CUDA_R_64F,
                       CUDSS_LAYOUT_COL_MAJOR), cudss_status, "Failed to create cuDSS matrix x");
  CUDSS_CALL_AND_CHECK(cudssMatrixCreateDn(&b, num_row, 1, num_row, p_b, CUDA_R_64F,
                       CUDSS_LAYOUT_COL_MAJOR), cudss_status, "Failed to create cuDSS matrix b");

  /* Create handler for A */
  cudssMatrixType_t     mtype = CUDSS_MTYPE_GENERAL; 
  cudssMatrixViewType_t mview = CUDSS_MVIEW_FULL;
  cudssIndexBase_t      base  = CUDSS_BASE_ZERO;
  CUDSS_CALL_AND_CHECK(
     cudssMatrixCreateCsr(&A, num_row, num_row, nnz, row_ptr, nullptr, col_idx, p_a,
                          CUDA_R_32I, CUDA_R_64F, mtype, mview, base),
     cudss_status, "Failed to create cuDSS matrix A"
  );

  /* Symbolic factorization */
  CUDSS_CALL_AND_CHECK(cudssExecute(handle, CUDSS_PHASE_ANALYSIS, solver_config, solver_data, A, x, b),
                       cudss_status, "Failed to execute cuDSS analysis phase");
  /* Factorization */
  CUDSS_CALL_AND_CHECK(cudssExecute(handle, CUDSS_PHASE_FACTORIZATION, solver_config, solver_data, A, x, b),
                       cudss_status, "Failed to execute cuDSS factorization phase");
  /* Solve */                  
  CUDSS_CALL_AND_CHECK(cudssExecute(handle, CUDSS_PHASE_SOLVE, solver_config, solver_data, A, x, b),
                       cudss_status, "Failed to execute cuDSS solve phase");

  /* Destroying objects */
  CUDSS_CALL_AND_CHECK(cudssMatrixDestroy(A), cudss_status, "Failed to destroy cuDSS matrix A");
  CUDSS_CALL_AND_CHECK(cudssMatrixDestroy(x), cudss_status, "Failed to destroy cuDSS matrix x");
  CUDSS_CALL_AND_CHECK(cudssMatrixDestroy(b), cudss_status, "Failed to destroy cuDSS matrix b");
  CUDSS_CALL_AND_CHECK(cudssDataDestroy(handle, solver_data), cudss_status, "Failed to destroy cuDSS data");
  CUDSS_CALL_AND_CHECK(cudssConfigDestroy(solver_config), cudss_status, "Failed to destroy cuDSS config");
//  CUDSS_CALL_AND_CHECK(cudssDestroy(handle), cudss_status, "Failed to destroy cuDSS handle");
//  CUDA_CALL_AND_CHECK(cudaStreamSynchronize(stream), "Failed to synchronize CUDA stream");

  return 0;           
}

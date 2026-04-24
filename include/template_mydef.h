#pragma once

#define DEV 0
#define CHECK_TYPE(t1, t2)((t1 & t2) == t2)

#define DEG_P 3
#define DEG_Q 3
#define DEG_R 3
#define MAX_PQ ((DEG_P) > (DEG_Q) ? (DEG_P) : (DEG_Q))
#define MAX_PR ((DEG_P) > (DEG_R) ? (DEG_P) : (DEG_R))
#define MAX_QR ((DEG_Q) > (DEG_R) ? (DEG_Q) : (DEG_R))

#define MAX_PQR ((MAX_PQ) > (DEG_R) ? (MAX_PQ) : (DEG_R))

#include <iostream>

#include "vec3.h"
using Double3 = vec3<double>;
#include "managed.h"
#include "json.h"
#include "xdmf3.h"
#include "domain.h"

#include "spline.h"
#include "mycudss.h"
#include "myfunc.h"
#include "iga.h"
#include "init.h"
#include "output.h"
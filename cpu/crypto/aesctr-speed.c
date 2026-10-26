#include "aesctr.h"

#define NEXT aesctr()

#define INIT aesctr_seed(0);

#include "harness.c"

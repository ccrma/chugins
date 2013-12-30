#if !defined(__objdefs_h)
#define __objdefs_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
// #define NDEBUG     /* define to disable asserts */

#define DEFAULT_CONTROL_RATE  200

#if (!defined(M_PI))
  #define M_PI 3.14159265358979323846264338327
#endif
#if (!defined(PI))
  #define PI M_PI
#endif
#if (!defined(TWO_PI))
  #define TWO_PI (2.0 * M_PI)
#endif
#define ONE_OVER_TWO_PI (1.0 / TWO_PI)
#define SQRT_TWO 1.4142135623730950488

#endif


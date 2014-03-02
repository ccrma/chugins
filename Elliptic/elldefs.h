/* max number of filter sections */
#define MAX_SECTIONS      12

/* defines one section of an elliptical filter */
typedef struct {
   float c0, c1, c2, c3;    /* coefficients */
   float x1, x2, y1, y2;    /* signal history */
} EllSect;


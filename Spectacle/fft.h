#define FORWARD 1
#define INVERSE 0

#if (!defined(M_PI))
  #define M_PI 3.14159265358979323846264338327
#endif
#if (!defined(PI))
  #define PI M_PI
#endif
#if (!defined(TWO_PI))
  #define TWO_PI (2.0 * M_PI)
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void JGrfft(float x[], int N, int forward);
void JGcfft(float x[], int NC, int forward);

#ifdef __cplusplus
} /* extern "C" */
#endif




class Filters
{
public:
    // 2nd order
    static void bilinearTranform(double acoefs[], double dcoefs[], double fs);
    // 2nd order parametric section
    static void designParametric(double* dcoefs, double center, double gain, double qval, double fs);
};


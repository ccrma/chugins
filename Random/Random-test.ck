@import "Random.chug"


// will sample RVs, calc these stats, and compare to expected

fun float calc_mean(int nums[]) {
    0 => float sum;
    for (int num : nums) sum + num => sum;
    return sum / nums.size();
}

fun float calc_fmean(float nums[]) {
    0 => float sum;
    for (float num : nums) sum + num => sum;
    return sum / nums.size();
}

fun float calc_fstd_dev(float nums[]) {
    calc_fmean(nums) => float mean;

    0 => float sum_sq_diff;
    for (float num : nums) Math.pow(num - mean, 2) +=> sum_sq_diff;
    return Math.sqrt(sum_sq_diff/nums.size());
}

fun float calc_std_dev(int nums[]) {
    calc_mean(nums) => float mean;

    0 => float sum_sq_diff;
    for (int num : nums) Math.pow(num - mean, 2) +=> sum_sq_diff;
    return Math.sqrt(sum_sq_diff/nums.size());
}



// sampling functions

fun float[] sample_gaussian(float mean, float std_dev, int num_samples) {
    float samples[num_samples];
    for (int i; i < num_samples; i++) Random.gaussian(mean, std_dev) => samples[i];
    return samples;
}

fun int[] sample_binom(int n, float p, int num_samples) {
    int samples[num_samples];
    for (int i; i < num_samples; i++) Random.binomial(n, p) => samples[i];
    return samples;
}

fun float[] sample_exp(float scale, int num_samples) {
    float samples[num_samples];
    for (int i; i < num_samples; i++) Random.exponential(scale) => samples[i];
    return samples;
}

fun int[] sample_geom(float p, int num_samples) {
    int samples[num_samples];
    for (int i; i < num_samples; i++) Random.geometric(p) => samples[i];
    return samples;
}

fun int[] sample_poisson(float p, int num_samples) {
    int samples[num_samples];
    for (int i; i < num_samples; i++) Random.poisson(p) => samples[i];
    return samples;
}



// unit testing

fun void compare_stats_gaussian() {
    <<< "Gaussian Test", "" >>>;

    1. => float mean;
    5. => float std_dev;
    100000 => int num_samples;

    sample_gaussian(mean, std_dev, num_samples) @=> float samples[];

    calc_fmean(samples) => float sample_mean;
    calc_fstd_dev(samples) => float sample_std_dev;

    chout <= "True mean: " <= mean <= IO.nl();
    chout <= "Obs. mean: " <= sample_mean <= IO.nl();
    
    chout <= "True std. dev.: " <= std_dev <= IO.nl();
    chout <= "Obs. std. dev.: " <= sample_std_dev <= IO.nl();

    Math.fabs((mean - sample_mean) / mean) => float mean_off;
    Math.fabs((std_dev - sample_std_dev) / std_dev) => float std_dev_off;

    if (mean_off < 0.05 && std_dev_off < 0.05) {
        <<< "=== Test: PASS ===", "\n" >>>;
    } else {
        <<< "=== Test: FAIL ===", "\n" >>>;
    }
}


fun void compare_stats_binom() {
    <<< "Binomial Test", "" >>>;

    20 => int n;
    0.4 => float p;
    100000 => int num_samples;

    sample_binom(n, p, num_samples) @=> int samples[];

    calc_mean(samples) => float sample_mean;
    calc_std_dev(samples) => float sample_std_dev;

    n * p => float mean;
    Math.pow(n * p * (1-p), 0.5) => float std_dev;

    chout <= "True mean: " <= mean <= IO.nl();
    chout <= "Obs. mean: " <= sample_mean <= IO.nl();
    
    chout <= "True std. dev.: " <= std_dev <= IO.nl();
    chout <= "Obs. std. dev.: " <= sample_std_dev <= IO.nl();

    Math.fabs((mean - sample_mean) / mean) => float mean_off;
    Math.fabs((std_dev - sample_std_dev) / std_dev) => float std_dev_off;

    if (mean_off < 0.05 && std_dev_off < 0.05) {
        <<< "=== Test: PASS ===", "\n" >>>;
    } else {
        <<< "=== Test: FAIL ===", "\n" >>>;
    }
}


fun void compare_stats_exp() {
    <<< "Exponential Test", "" >>>;

    3. => float scale; // 1 / lambda, not lambda
    100000 => int num_samples;

    sample_exp(scale, num_samples) @=> float samples[];

    calc_fmean(samples) => float sample_mean;
    calc_fstd_dev(samples) => float sample_std_dev;

    scale => float mean;
    scale => float std_dev;

    chout <= "True mean: " <= mean <= IO.nl();
    chout <= "Obs. mean: " <= sample_mean <= IO.nl();
    
    chout <= "True std. dev.: " <= std_dev <= IO.nl();
    chout <= "Obs. std. dev.: " <= sample_std_dev <= IO.nl();

    Math.fabs((mean - sample_mean) / mean) => float mean_off;
    Math.fabs((std_dev - sample_std_dev) / std_dev) => float std_dev_off;

    if (mean_off < 0.05 && std_dev_off < 0.05) {
        <<< "=== Test: PASS ===", "\n" >>>;
    } else {
        <<< "=== Test: FAIL ===", "\n" >>>;
    }
}



fun void compare_stats_geom() {
    <<< "Geometric Test", "" >>>;

    0.2 => float p;
    100000 => int num_samples;

    sample_geom(p, num_samples) @=> int samples[];

    calc_mean(samples) => float sample_mean;
    calc_std_dev(samples) => float sample_std_dev;

    1. / p => float mean;
    Math.pow(1 - p, 0.5) / p => float std_dev;

    chout <= "True mean: " <= mean <= IO.nl();
    chout <= "Obs. mean: " <= sample_mean <= IO.nl();
    
    chout <= "True std. dev.: " <= std_dev <= IO.nl();
    chout <= "Obs. std. dev.: " <= sample_std_dev <= IO.nl();

    Math.fabs((mean - sample_mean) / mean) => float mean_off;
    Math.fabs((std_dev - sample_std_dev) / std_dev) => float std_dev_off;

    if (mean_off < 0.05 && std_dev_off < 0.05) {
        <<< "=== Test: PASS ===", "\n" >>>;
    } else {
        <<< "=== Test: FAIL ===", "\n" >>>;
    }
}


fun void compare_stats_poisson() {
    <<< "Poisson Test", "" >>>;

    3 => float lambda;
    100000 => int num_samples;

    sample_poisson(lambda, num_samples) @=> int samples[];

    calc_mean(samples) => float sample_mean;
    calc_std_dev(samples) => float sample_std_dev;

    lambda => float mean;
    Math.pow(lambda, 0.5) => float std_dev;

    chout <= "True mean: " <= mean <= IO.nl();
    chout <= "Obs. mean: " <= sample_mean <= IO.nl();
    
    chout <= "True std. dev.: " <= std_dev <= IO.nl();
    chout <= "Obs. std. dev.: " <= sample_std_dev <= IO.nl();

    Math.fabs((mean - sample_mean) / mean) => float mean_off;
    Math.fabs((std_dev - sample_std_dev) / std_dev) => float std_dev_off;

    if (mean_off < 0.05 && std_dev_off < 0.05) {
        <<< "=== Test: PASS ===", "\n" >>>;
    } else {
        <<< "=== Test: FAIL ===", "\n" >>>;
    }
}



// If these all print "PASS", you're good-ish

compare_stats_gaussian();
compare_stats_binom();
compare_stats_exp();
compare_stats_geom();
compare_stats_poisson();
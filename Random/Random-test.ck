
0 => float sum;
float nums[0];
10000 => int count;

repeat(count)
{
    Random.gaussian(0, 10) => float r;
    chout <= r <= IO.nl();
    r +=> sum;
    nums << r;
}

sum/count => float mean;
0 => float sum_sq_diff;
// re-iterate through sequence and calculate std deviation
for(0 => int i; i < nums.size(); i++)
    (nums[i]-mean)*(nums[i]-mean) +=> sum_sq_diff;

chout <= "mean value: " <= mean <= IO.nl();
chout <= "std deviation: " <= Math.sqrt(sum_sq_diff/count) <= IO.nl();


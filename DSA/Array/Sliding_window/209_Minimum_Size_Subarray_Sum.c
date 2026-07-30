int minSubArrayLen(int target, int *nums, int numsSize)
{
    int min = numsSize + 1;
    int l = 0;
    int sum = 0;

    for (int r = 0; r < numsSize; r++)
    {
        sum += nums[r];

        while (sum >= target)
        {
            min = fmin(min, r - l + 1);
            sum -= nums[l];
            l++;
        }
    }
    return (min == numsSize + 1) ? 0 : min;
}
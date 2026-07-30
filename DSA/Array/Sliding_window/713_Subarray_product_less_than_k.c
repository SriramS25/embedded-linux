int numSubarrayProductLessThanK(int *nums, int numsSize, int k)
{
    int count = 0;
    int l = 0;
    int product = 1;

    for (int r = 0; r < numsSize; r++)
    {

        product = product * nums[r];

        while ((product >= k) && (l <= r))
        {
            product /= nums[l];
            l++;
        }

        count += r - l + 1;
    }

    return count;
}
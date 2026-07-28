int findMaxConsecutiveOnes(int *nums, int numsSize)
{
    int max = 0, one = 0;
    int *l = nums;

    for (int i = 0; i < numsSize; i++)
    {
        if (*l == 1)
        {
            one++;
            l++;
            if (one > max)
            {
                max = one;
            }
        }
        else
        {
            l = l + 1;
            one = 0;
        }
    }
    return max;
}
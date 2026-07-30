int longestOnes(int *nums, int numsSize, int k)
{
    int zero_c = 0;
    int max_c = 0;
    int l = 0;
    int r = 0;

    for (r = 0; r < numsSize; r++)
    {

        if (nums[r] == 0)
        {
            zero_c++;
        }

        while (zero_c > k)
        {
            if (nums[l] == 0)
            {
                zero_c--;
            }
            l++;
        }

        int window = r - l + 1;
        if (window > max_c)
        {
            max_c = window;
        }
    }
    return max_c;
}
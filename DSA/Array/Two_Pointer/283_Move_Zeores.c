void moveZeroes(int *nums, int numsSize)
{
    int *ptr = nums;
    int *end = nums + numsSize;

    for (int i = 0; i < numsSize; i++)
    {
        if (nums[i] != 0)
        {
            *ptr = nums[i];
            ptr++;
        }
    }

    while (ptr < end)
    {
        *ptr = 0;
        ptr++;
    }
}
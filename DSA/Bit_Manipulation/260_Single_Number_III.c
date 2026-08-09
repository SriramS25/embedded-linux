/**
 * Note: The returned array must be malloced, assume caller calls free().
 */
int *singleNumber(int *nums, int numsSize, int *returnSize)
{
    int *res = (int *)malloc(sizeof(int) * 2);
    *returnSize = 2;

    unsigned int xorall = 0;
    for (int i = 0; i < numsSize; i++)
    {
        xorall ^= nums[i];
    }

    unsigned int bit = xorall & (-xorall);
    int a = 0, b = 0;
    for (int i = 0; i < numsSize; i++)
    {
        if (nums[i] & bit)
        {
            a ^= nums[i];
        }
        else
        {
            b ^= nums[i];
        }
    }
    res[0] = a;
    res[1] = b;

    return res;
}
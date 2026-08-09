// Method 1 using O(1) space
/**
 * Note: The returned array must be malloced, assume caller calls free().
 */
int *productExceptSelf(int *nums, int numsSize, int *returnSize)
{

    int *ans = (int *)malloc(sizeof(int) * numsSize);
    *returnSize = numsSize;

    int prefix = 1;
    for (int i = 0; i < numsSize; i++)
    {
        ans[i] = prefix;
        prefix *= nums[i];
    }

    int suffix = 1;
    for (int i = numsSize - 1; i >= 0; i--)
    {
        ans[i] *= suffix;
        suffix *= nums[i];
    }

    return ans;
}

// Method 2 using O(n) space
/**
 * Note: The returned array must be malloced, assume caller calls free().
 */
int *productExceptSelf(int *nums, int numsSize, int *returnSize)
{

    int *ans = (int *)malloc(sizeof(int) * numsSize);
    *returnSize = numsSize;

    int right[numsSize] = {0};
    int left[numsSize] = {0};

    int prefix = 1;
    for (int i = 0; i < numsSize; i++)
    {
        left[i] = prefix;
        prefix *= nums[i];
    }

    int suffix = 1;
    for (int i = numsSize - 1; i >= 0; i--)
    {
        right[i] = suffix;
        suffix *= nums[i];
    }

    for (int i = 0; i < numsSize; i++)
    {
        ans[i] = left[i] * right[i];
    }

    return ans;
}
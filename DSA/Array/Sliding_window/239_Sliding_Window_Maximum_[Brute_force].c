// Brute Force
/**
 * Note: The returned array must be malloced, assume caller calls free().
 */
int *maxSlidingWindow(int *nums, int numsSize, int k, int *returnSize)
{

    if (numsSize == 0 || k == 0)
    {
        *returnSize = 0;
        return NULL;
    }

    int output_size = numsSize - k + 1;
    *returnSize = output_size;

    int *arr = (int *)malloc(sizeof(int) * output_size);

    int l = 0;
    int r = 0;
    int a_i = 0;

    int max = INT_MIN;
    int count = 0;

    while (r < numsSize)
    {

        if (count < k)
        {
            max = fmax(nums[r], max);
            r++;
            count++;
        }
        else
        {
            arr[a_i] = max;
            a_i++;

            max = INT_MIN;
            count = 0;
            l++;
            r = l;
        }
    }
    arr[a_i] = max;

    return arr;
}
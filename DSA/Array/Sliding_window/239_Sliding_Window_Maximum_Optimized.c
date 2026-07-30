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
    int a_i = 0;

    int deque[numsSize];
    int front = 0;
    int back = -1;

    for (int r = 0; r < numsSize; r++)
    {
        // STEP 1: Remove indices that are outside current window
        // Current window is [r-k+1, i]
        // If front index < r-k+1, it's no longer in window
        if (front <= back && deque[front] < r - k + 1)
        {
            front++;
        }

        // STEP 2: Remove smaller elements from back
        // We want deque in decreasing order
        // If current element is bigger than elements at back, remove them
        while (front <= back && nums[deque[back]] <= nums[r])
        {
            back--;
        }

        // STEP 3: Add index to the deque
        back++;
        deque[back] = r;

        // STEP 4: When first window is complete , start storing results
        //  r > = k-1 means we've processed at least k elements
        if (r >= k - 1)
        {
            arr[a_i] = nums[deque[front]];
            a_i++;
        }
    }

    return arr;
}
/**
 * Return an array of arrays of size *returnSize.
 * The sizes of the arrays are returned as *returncol_sizeumnSizes array.
 * Note: Both returned array and *col_sizeumnSizes array must be malloced, assume caller calls free().
 */
int compare(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

int **threeSum(int *nums, int numsSize, int *returnSize, int **returncol_sizeumnSizes)
{
    int capacity = 16;
    int **res = malloc(capacity * sizeof(int *));
    int *col_size = malloc(capacity * sizeof(int));

    int count = 0;

    qsort(nums, numsSize, sizeof(int), compare);

    for (int i = 0; i < numsSize - 2; i++)
    {
        if (i > 0 && nums[i] == nums[i - 1])
            continue;

        int left = i + 1;
        int right = numsSize - 1;

        while (left < right)
        {
            int sum = nums[i] + nums[left] + nums[right];

            if (sum < 0)
            {
                left++;
            }
            else if (sum > 0)
            {
                right--;
            }
            else
            {
                if (count == capacity)
                {
                    capacity *= 2;

                    res = realloc(res, capacity * sizeof(int *));
                    col_size = realloc(col_size, capacity * sizeof(int));
                }
                res[count] = malloc(sizeof(int) * 3);
                res[count][0] = nums[i];
                res[count][1] = nums[left];
                res[count][2] = nums[right];

                col_size[count] = 3;
                count++;

                left++;
                right--;

                // skip duplicate left values
                while (left < right && nums[left] == nums[left - 1])
                {
                    left++;
                }

                // skip duplicate left values
                while (left < right && nums[right] == nums[right + 1])
                {
                    right--;
                }
            }
        }
    }

    *returnSize = count;
    *returncol_sizeumnSizes = col_size;
    return res;
}
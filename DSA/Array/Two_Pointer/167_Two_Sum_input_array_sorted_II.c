/**
 * Note: The returned array must be malloced, assume caller calls free().
 */
int *twoSum(int *numbers, int numbersSize, int target, int *returnSize)
{
    *returnSize = 2;
    int *l = numbers;
    int *r = numbers + numbersSize - 1;

    int *ans = (int *)malloc(sizeof(int) * 2);

    while (l < r)
    {
        int sum = *l + *r;
        if (sum == target)
        {

            ans[0] = l - numbers + 1;
            ans[1] = r - numbers + 1;
            return ans;
        }
        else if (sum > target)
        {
            r--;
        }
        else
        {
            l++;
        }
    }

    free(ans);
    *returnSize = 0;
    return NULL;
}
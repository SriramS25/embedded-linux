long long maximumSubarraySum(int *nums, int numsSize, int k)
{
    long long max = 0;
    int freq[100001] = {0};
    int distinct_count = 0;
    long long sum = 0;

    for (int i = 0; i < k; i++)
    {
        freq[nums[i]]++;
        if (freq[nums[i]] == 1)
        {
            distinct_count++;
        }
        sum += nums[i];
    }
    if (distinct_count == k)
    {
        max = sum;
    }
    printf("first window sum = %d\n", max);

    for (int i = k; i < numsSize; i++)
    {

        freq[nums[i - k]]--;
        sum = sum - nums[i - k];
        if (freq[nums[i - k]] == 0)
        {
            distinct_count--;
        }

        freq[nums[i]]++;
        sum = sum + nums[i];
        if (freq[nums[i]] == 1)
        {
            distinct_count++;
        }

        if (distinct_count == k)
        {
            if (sum > max)
            {
                max = sum;
            }
        }
    }

    printf("max  = %d", max);
    return max;
}
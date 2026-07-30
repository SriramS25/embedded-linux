int atMostKDistinct(int *nums, int numsSize, int k)
{

    // Edge case:
    // No subarray can have at most -1 distinct elements.
    if (k < 0)
        return 0;

    int l = 0;
    int r = 0;
    int distinct = 0;
    int answer = 0;
    int freq[numsSize + 1];
    memset(freq, 0, sizeof(freq));

    for (; r < numsSize; r++)
    {

        if (freq[nums[r]] == 0)
        {
            distinct++;
        }
        freq[nums[r]]++;

        while (distinct > k)
        {
            freq[nums[l]]--;
            if (freq[nums[l]] == 0)
            {
                distinct--;
            }
            l++;
        }

        answer += (r - l + 1);
    }
    return answer;
}

int subarraysWithKDistinct(int *nums, int numsSize, int k)
{
    return atMostKDistinct(nums, numsSize, k) - atMostKDistinct(nums, numsSize, k - 1);
}
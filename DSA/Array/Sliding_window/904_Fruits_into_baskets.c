int totalFruit(int *fruits, int fruitsSize)
{
    int distinct = 0;
    int l = 0;
    int k = 2;
    int max = 0;
    int freq[100001] = {0};

    for (int r = 0; r < fruitsSize; r++)
    {
        freq[fruits[r]]++;
        if (freq[fruits[r]] == 1)
        {
            distinct++;
        }

        while (distinct > k)
        {
            freq[fruits[l]]--;
            if (freq[fruits[l]] == 0)
            {
                distinct--;
            }
            l++;
        }

        max = fmax(max, r - l + 1);
    }

    return max;
}
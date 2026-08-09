int singleNumber(int *nums, int numsSize)
{
    int ans = 0;
    for (int bit = 0; bit < 32; bit++)
    {
        //checking all the bits of the numbers in the array
        //counting the number of 1's in the current bit position
        int count = 0;
        for (int i = 0; i < numsSize; i++)
        {
            int curr_bit = (nums[i] >> bit) & 1;
            if (curr_bit == 1)
            {
                count++;
            }
        }
        //if the count of 1's in the current bit position is not a multiple of 3, then the unique number
        // has a 1 in that bit position
        
        if (count % 3 == 1)
        {
            ans = ans | (1U << bit);
        }
    }
    return ans;
}
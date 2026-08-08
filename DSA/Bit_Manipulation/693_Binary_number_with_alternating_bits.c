//Method 1: Using bit manipulation
bool hasAlternatingBits(int n)
{
    int prev = n & 1;
    n = n >> 1;
    int curr = 0;
    while (n)
    {
        curr = n & 1;
        if (curr == prev)
        {
            return false;
        }

        n = n >> 1;
        prev = curr;
    }
    return true;
}


//Method 2: Using bit manipulation
bool hasAlternatingBits(int n)
{
    uint x = n ^ (n >> 1);
    return (x & (x + 1)) == 0;
}
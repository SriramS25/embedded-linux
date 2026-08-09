class Solution
{
public:
    vector<int> singleNum(vector<int> &arr)
    {
        // Code here.
        vector<int> res;
        int xorAll = 0;

        for (int x : arr)
        {
            xorAll ^= x;
        }

        int bit = xorAll & (-xorAll);

        int a = 0;
        int b = 0;

        for (int x : arr)
        {
            if (x & bit)
            {
                a ^= x;
            }
            else
            {
                b ^= x;
            }
        }

        if (a > b)
        {
            res.push_back(b);
            res.push_back(a);
        }
        else
        {
            res.push_back(a);
            res.push_back(b);
        }

        return res;
    }
};
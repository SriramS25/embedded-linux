int trap(int *height, int heightSize)
{
    int l = 0;
    int r = heightSize - 1;

    int left_max = height[l];
    int right_max = height[r];

    int water = 0;

    while (l < r)
    {
        if (left_max < right_max)
        {
            l++;
            left_max = fmax(left_max, height[l]);
            water += left_max - height[l];
        }
        else
        {
            r--;
            right_max = fmax(right_max, height[r]);
            water += right_max - height[r];
        }
    }

    return water;
}
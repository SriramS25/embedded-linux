int maxArea(int *height, int heightSize)
{
    int *l = height;
    int *r = height + heightSize - 1;

    int max_area = 0;
    int min = 0;
    int width = 0;
    int area = 0;
    while (l < r)
    {
        min = fmin(*l, *r);
        area = min * (r - l);
        max_area = fmax(area, max_area);

        if (*l < *r)
        {
            l++;
        }
        else
        {
            r--;
        }
    }
    return max_area;
}
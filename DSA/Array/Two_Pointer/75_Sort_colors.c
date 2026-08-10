void sortColors(int* nums, int numsSize) {

    int *s = nums;
    int *m = nums;
    int *h = nums + numsSize - 1;

    while(m <= h){
        if(*m == 0){
            *m = *s;
            *s = 0;
            s++;
            m++;
        }else if(*m == 1){
            m++;
        }else{
            *m = *h;
            *h = 2;
            h--;
        }
    }
}
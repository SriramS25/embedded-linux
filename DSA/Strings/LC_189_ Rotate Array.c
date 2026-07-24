/* 
Critical Edge Cases
k > numsSize: Normalize with k %= numsSize

k = 0 or array size ≤ 1: No rotation needed

k is multiple of numsSize: Array returns to original

Time & Space Complexity
Time: O(n) - Three passes

Space: O(1) - In-place, just three pointers and a temp variable 

Reverse First K → Reverse Remaining → Full Reverse

*/
void rotate(int* nums, int numsSize, int k) {
    int *l = nums;
    int *r = nums + numsSize - 1;
    int swap = 0;
    k = k % numsSize;

    while(l < r){
        swap = *l;
        *l = *r;
        *r = swap;
        l++;
        r--;
    }

    l = nums;
    r = nums + k - 1;

    while(l < r){
        swap = *l;
        *l = *r;
        *r = swap;
        l++;
        r--;
    }

    l = nums + k;
    r = nums + numsSize -1;

    while(l < r){
        swap = *l;
        *l = *r;
        *r = swap;
        l++;
        r--;
    }

}
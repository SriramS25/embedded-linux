#include <stdio.h>
//Input : 1234
//Output: 1     2      3       4
void printnum(int x){
    if(x == 0){
        return;
    }
    printnum(x/10);
    printf("%d\n", x%10);
}

int main(){
    int x = 1234;
    printnum(x);
    return 0;
}
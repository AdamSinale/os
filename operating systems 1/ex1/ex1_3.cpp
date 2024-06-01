
int main(){

    int* iptr;
    iptr = (int*)0xdeadbeef;

    int a = *iptr;
    return 0;
}
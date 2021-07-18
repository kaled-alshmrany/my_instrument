//./FuSeBMC_inustrment --input examples/a.c --output examples/a_out.c --add-labels --add-else --add-label-after-loop
int main()
{
    int x;
    if(x)x++;else x--;
    if(x){x++;}
    while(x)x++;
    for(;;)x--;
    return 0;
}
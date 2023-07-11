#include <cstdlib>
#include <list>
#include <unistd.h>
#include <cstring>
#include <stdlib.h>
#include <cmath>
#include <stdio.h>

int main () {
	
	//void a[1000];
	void *a;
	a = malloc(100*sizeof(int));
	
    for (int i = 0; i < 1000; i++) {
		a[i] = i;
	}


    printf ("address of a = 0x%x\n", a);
	
	int temp_address_adder = 128;
	printf ("after a+= %d, address of a = 0x%x\n", temp_address_adder, a + temp_address_adder);
	printf ("and *a = %d\n", *(a + temp_address_adder));
	
	return 0;
}
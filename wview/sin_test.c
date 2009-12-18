#include <math.h>
#include <stdio.h>

int main(int argc, char **argv) {
	float ofs=0;

	for(ofs=0; ofs<10000; ofs+=0.10) {
		float val = (sin(ofs)*127)+128;
		putchar((unsigned char )val);
	}
	return 0;

}

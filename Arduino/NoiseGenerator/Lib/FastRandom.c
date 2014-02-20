/** Base du code récupéré sur http://www.electro-tech-online.com/threads/ultra-fast-pseudorandom-number-generator-for-8-bit.124249/ **/

#include "FastRandom.h"

unsigned char a,b,c;
unsigned char x=0;

void init_rng(unsigned char s1,unsigned char s2,unsigned char s3) //Can also be used to seed the rng with more entropy during use.
{
	//XOR new entropy into key state
	a ^=s1;
	b ^=s2;
	c ^=s3;

	x++;
	a = (a^c^x);
	b = (b+a);
	c = (c+(b>>1)^a);
}

unsigned char randomize()
{
	x++;               //x is incremented every round and is not affected by any other variable
	a = (a^c^x);       //note the mix of addition and XOR
	b = (b+a);         //And the use of very few instructions
	c = (c+(b>>1)^a);  //the right shift is to ensure that high-order bits from b can affect  
	return c;         //low order bits of other variables
}

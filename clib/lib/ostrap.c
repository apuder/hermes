
#include <cinclude.h>


volatile int k_p1;
volatile int k_p2;
volatile int k_p3;
volatile int k_p4;

volatile int k_r1;
volatile int k_r2;
volatile int k_r3;
volatile int k_r4;



void kernel_trap( int op )
{
	asm( "pushl %esi;pushl %eax;pushl %ebx;pushl %ecx;pushl %edx" );
	asm( "movl %0,%%esi" : : "m" (op) );
	asm( "movl %0,%%eax" : : "m" (k_p1) );
	asm( "movl %0,%%ebx" : : "m" (k_p2) );
	asm( "movl %0,%%ecx" : : "m" (k_p3) );
	asm( "movl %0,%%edx" : : "m" (k_p4) );
	asm( "int $111" );
	asm( "movl %%eax,%0" : "=m" (k_r1) : );
	asm( "movl %%ebx,%0" : "=m" (k_r2) : );
	asm( "movl %%ecx,%0" : "=m" (k_r3) : );
	asm( "movl %%edx,%0" : "=m" (k_r4) : );
	asm( "popl %edx;popl %ecx;popl %ebx;popl %eax;popl %esi" );
}

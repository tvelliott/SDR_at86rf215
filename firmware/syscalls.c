

//MIT License
//
//Copyright (c) 2018 tvelliott
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.



/* Support files for GNU libc.  Files in the system namespace go here.
   Files in the C namespace (ie those that do not start with an
   underscore) go in .c.  */

#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>
#include <reent.h>
#include <unistd.h>
#include <sys/wait.h>



#define FreeRTOS
#define MAX_STACK_SIZE 0x2000

extern int __io_putchar( int ch ) __attribute__( ( weak ) );
extern int __io_getchar( void ) __attribute__( ( weak ) );

volatile uint32_t r0;
volatile uint32_t r1;
volatile uint32_t r2;
volatile uint32_t r3;
volatile uint32_t r12;
volatile uint32_t lr; /* Link register. */
volatile uint32_t pc; /* Program counter. */
volatile uint32_t psr;/* Program status register. */

/* The prototype shows it is a naked function - in effect this is just an
  * assembly function. */
void HardFault_Handler( void ) __attribute__( ( naked ) );

/* The fault handler implementation calls a function called
  * prvGetRegistersFromStack(). */
void HardFault_Handler( void )
{
  __asm volatile
  (
    " tst lr, #4                                                \n"
    " ite eq                                                    \n"
    " mrseq r0, msp                                             \n"
    " mrsne r0, psp                                             \n"
    " ldr r1, [r0, #24]                                         \n"
    " ldr r2, handler2_address_const                            \n"
    " bx r2                                                     \n"
    " handler2_address_const: .word prvGetRegistersFromStack    \n"
  );
}

void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
  /* These are volatile to try and prevent the compiler/linker optimising them
    * away as the variables never actually get used.  If the debugger won't show the
    * values of the variables, make them global my moving their declaration outside
    * of this function. */

  r0 = pulFaultStackAddress[ 0 ];
  r1 = pulFaultStackAddress[ 1 ];
  r2 = pulFaultStackAddress[ 2 ];
  r3 = pulFaultStackAddress[ 3 ];

  r12 = pulFaultStackAddress[ 4 ];
  lr = pulFaultStackAddress[ 5 ];
  pc = pulFaultStackAddress[ 6 ];
  psr = pulFaultStackAddress[ 7 ];

  /* When the following line is hit, the variables contain the register values. */
  for( ;; );
}


register char *stack_ptr __asm__( "sp" );
extern char end __asm__( "end" );
static char *heap_end = &end;
static char *prev_heap_end = &end;

int _mem_free( void )
{
  return ( int ) 0x2407ffff - ( int ) heap_end;
}

void print_stackptr( void )
{
  //printf("\r\nstack_ptr %08x", stack_ptr);
}

caddr_t _sbrk( int incr )
{

  prev_heap_end = heap_end;

  heap_end += incr;

  //printf("\r\nheap_incr %d, addr %08x, sp = %08x", incr, (int) prev_heap_end, (int) stack_ptr);

  return ( caddr_t ) prev_heap_end;
}


void initialise_monitor_handles()
{
}

int _getpid( void )
{
  return 1;
}

int _kill( int pid, int sig )
{
  errno = EINVAL;
  return -1;
}

void _exit( int status )
{
  //printf("\r\nexit called, status=%d", status);
  _kill( status, -1 );
  while( 1 ) {}
}

int _write( int file, char *ptr, int len )
{
  int DataIdx;

  for( DataIdx = 0; DataIdx < len; DataIdx++ ) {
    __io_putchar( *ptr++ );
  }
  return len;
}

int _close( int file )
{
  return -1;
}

int _fstat( int file, struct stat *st )
{
  st->st_mode = S_IFCHR;
  return 0;
}

int _isatty( int file )
{
  return 1;
}

int _lseek( int file, int ptr, int dir )
{
  return 0;
}

int _read( int file, char *ptr, int len )
{
  int DataIdx;

  for( DataIdx = 0; DataIdx < len; DataIdx++ ) {
    *ptr++ = __io_getchar();
  }

  return len;
}

int _open( char *path, int flags, ... )
{
  /* Pretend like we always fail */
  return -1;
}

int _wait( int *status )
{
  errno = ECHILD;
  return -1;
}

int _unlink( char *name )
{
  errno = ENOENT;
  return -1;
}

int _times( struct tms *buf )
{
  return -1;
}

int _stat( char *file, struct stat *st )
{
  st->st_mode = S_IFCHR;
  return 0;
}

int _link( char *old, char *new )
{
  errno = EMLINK;
  return -1;
}

int _fork( void )
{
  errno = EAGAIN;
  return -1;
}

int _execve( char *name, char **argv, char **env )
{
  errno = ENOMEM;
  return -1;
}

/* mycode3.c: your portion of the kernel
 *
 *   	Below are functions that are called by other parts of the kernel. 
 * 	Your ability to modify the kernel is via these functions.  You may
 *  	modify the bodies of these functions (and add code outside of them)
 * 	in any way you wish, but you must not change their interfaces. 
 */

#include "aux.h"
#include "sys.h"
#include "mycode3.h"

#define FALSE 0
#define TRUE 1

/*  	A sample semaphore table.  You may change this in any way you wish.  
 */


static struct {
	int valid;	// Is this a valid entry (was sem allocated)?
	int value;	// value of semaphore
	int waitingProcs[MAXPROCS];
	int headIndex;
	int tailIndex;

} semtab[MAXSEMS];


/*  	InitSem() is called when the kernel starts up. Initialize data
 * 	structures (such as the semaphore table) and call any initialization
 *   	functions here. 
 */

void InitSem()
{
	int s;

	/* modify or add code any way you wish */

	for (s = 0; s < MAXSEMS; s++) {		// mark all sems free
		semtab[s].valid = FALSE;
		semtab[s].headIndex = 0;
		semtab[s].tailIndex = 0;
	}
}

/* 	MySeminit(v) is called by the kernel whenever the system call
 *  	Seminit(v) is called.  The kernel passes the initial value v. 
 * 	MySeminit should allocate a semaphore (find a free entry in
 *  	semtab and allocate), initialize that semaphore's value to v,
 *  	and then return the ID (i.e., index of the allocated entry).  
 * 	Should return -1 if it fails (e.g., no free semaphores).  
 */

int MySeminit(int v)
	// v: initial value of semaphore
{
	int s;

	/* modify or add code any way you wish */

	for (s = 0; s < MAXSEMS; s++) {
		if (semtab[s].valid == FALSE) {
			break;
		}
	}
	if (s == MAXSEMS) {
		DPrintf("No free semaphores\n");
		return(-1);
	}

	semtab[s].valid = TRUE;
	semtab[s].value = v;	

	return(s);
}

/*   	MyWait(s) is called by the kernel whenever the system call
 * 	Wait(s) is called. Return 0 if successful, else -1 if failed. 
 */

int MyWait(int s)
	// s: semaphore ID
{
	/* modify or add code any way you wish */

	semtab[s].value--;

	if(semtab[s].value < 0){
		int pid = GetCurProc();
		if(semtab[s].tailIndex == MAXPROCS){
			semtab[s].tailIndex = 0;
		}
		semtab[s].waitingProcs[semtab[s].tailIndex++] = pid;
		Block();
	}
	return (0);
}

/*  	MySignal(s) is called by the kernel whenever the system call
 * 	Signal(s) is called.  Return 0 if successful, else -1 if failed. 
 */

int MySignal(int s)
	// s: semaphore ID
{
	/* modify or add code any way you wish */

	semtab[s].value++;

	if(semtab[s].value <= 0){
		if(semtab[s].headIndex == MAXPROCS){
			semtab[s].headIndex = 0;
		}
		int pid = semtab[s].waitingProcs[semtab[s].headIndex++];
		Unblock(pid);
	}

	return (0);
}

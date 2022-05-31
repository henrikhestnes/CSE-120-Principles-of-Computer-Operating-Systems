/* mycode2.c: your portion of the kernel
 *
 *   	Below are functions that are called by other parts of the kernel. 
 * 	Your ability to modify the kernel is via these functions.  You may
 *  	modify the bodies of these functions (and add code outside of them)
 * 	in any way you wish, but you must not change their interfaces. 
 */

#include "aux.h"
#include "sys.h"
#include "mycode2.h"

#define TIMERINTERVAL 		1	// in ticks (tick = 10 msec)
#define NULL 			0
#define L 			100000
#define SCHEDPOLICY 		PROPORTIONAL	 //one of: ARBITRARY, FIFO, LIFO, ROUNDROBIN or PROPORTIONAL
/*  	A sample process table.  You can change this any way you wish.  
 */

struct Node {
	int valid;	// is this entry valid: 0 = no, 1 = yes 
	int pid;	// process ID (as provided by kernel)
	
	struct Node* next;
	struct Node* prev;

	int pass;
	int stride;
	int requested_rate;
	int active_request;

} proctab[MAXPROCS];

struct Node* HEAD = NULL;
struct Node* TAIL = NULL;
struct Node* ACTIVE = NULL;

int CPU_allocated;
int num_requests;
int num_procs;

/*  	InitSched() is called when the kernel starts up. First, set the
 * 	scheduling policy (see sys.h). Make sure you follow the rules below
 *   	on where and how to set it.  Next, initialize all your data structures
 * 	(such as the process table). Finally, set the timer to interrupt after
 *  	a specified number of ticks.  
 */

void InitSched()
{
	int i;

	/* First, set the scheduling policy.  You should only set it from
	 * within this conditional statement. While you are working on
	 * this assignment, GetSchedPolicy() will return NOSCHEDPOLICY. 
	 * Thus, the condition will be true and you may set the scheduling
	 * policy to whatever you choose (i.e., you may replace ARBITRARY).  
	 * After the assignment is over, during the testing phase, we will
	 * have GetSchedPolicy() return the policy we wish to test (and
	 * the policy WILL NOT CHANGE during the entirety of a test). Thus
	 * the condition will be false and SetSchedPolicy(p) will not be
	 * called, leaving the policy to whatever we chose to test
	 * (and so it is important that you NOT put any critical code in
	 * the body of the conditional statement, as it will not execute
	 * when we test your program).  
	 */
	if (GetSchedPolicy() == NOSCHEDPOLICY) {	// leave as is
							// no other code here
		SetSchedPolicy(SCHEDPOLICY);		// set the policy here
							// no other code here
	}
		
	/* Initialize your data structures here */
	HEAD = &proctab[0];
	TAIL = &proctab[0];
	ACTIVE = &proctab[0];
	
	for (i = 0; i < MAXPROCS; i++) {
		proctab[i].valid = 0;
		proctab[i].next = NULL;
		proctab[i].prev = NULL;
	}

	CPU_allocated = 0;
	num_requests = 0;
	num_procs = 0;

	/* Set the timer last */
	SetTimer(TIMERINTERVAL);
}

void AllocateRemainingCPU();

/* 	StartingProc(p) is called by the kernel when the process
 *  	identified by PID p is starting.  This allows you to record the
 *  	arrival of a new process in the process table and allocate any
 * 	resources (if necessary). Returns 1 if successful, 0 otherwise. 
 */

int StartingProc(int p)
	// p: process that is starting
{
	int i;
	for (i = 0; i < MAXPROCS; i++) {
		if (!proctab[i].valid) {
			num_procs++;
			proctab[i].valid = 1;
			proctab[i].pid = p;
			proctab[i].pass = 0;
			proctab[i].stride = 0;
			proctab[i].requested_rate = 0;
			proctab[i].active_request = 0;
			if(&proctab[i] != HEAD){
				proctab[i].prev = TAIL;
				(proctab[i].prev)->next = &proctab[i];
			}
			TAIL = &proctab[i];
			if(GetSchedPolicy() == FIFO || GetSchedPolicy() == LIFO) DoSched();
			AllocateRemainingCPU();
			return (1);
		}
	}

	DPrintf("Error in StartingProc: no free table entries\n");
	return(0);
}
			

/*   	EndingProc(p) is called by the kernel when the process
 * 	identified by PID p is ending.  This allows you to update the
 *  	process table accordingly, and deallocate any resources (if
 * 	necessary). Returns 1 if successful, 0 otherwise.  
 */


int EndingProc(int p)
	// p: process that is ending
{
	int i;
	for (i = 0; i < MAXPROCS; i++) {
		if (proctab[i].valid && proctab[i].pid == p) {
			proctab[i].valid = 0;
			num_procs--;
			if(proctab[i].active_request){
				num_requests--;
				CPU_allocated -= proctab[i].requested_rate;

			}
			if(&proctab[i] == HEAD && &proctab[i] == TAIL){ //Only element in proctab, next inserted element will be at proctab[0]
				proctab[i].next = NULL;
				proctab[i].prev = NULL;

				HEAD = &proctab[0];
				TAIL = &proctab[0];
				ACTIVE = &proctab[0];
			}
			else if(&proctab[i] == HEAD){ //First element in proctab ending
				HEAD = proctab[i].next;
				ACTIVE = TAIL; //Workaround such that next running process will be the new HEAD

				(proctab[i].next)->prev = NULL;
				proctab[i].next = NULL;
			}
			else if(&proctab[i] == TAIL){ //Last element in proctab ending
				TAIL = proctab[i].prev;
				ACTIVE = TAIL; // Next process will be HEAD

				(proctab[i].prev)->next = NULL;
				proctab[i].prev = NULL;
			}
			else{ //Node in the middle removed
				(proctab[i].prev)->next = proctab[i].next;
				(proctab[i].next)->prev = proctab[i].prev;
				ACTIVE = proctab[i].prev; //Next process will be the ending process' next

				proctab[i].next = NULL;
				proctab[i].prev = NULL;
			}
			AllocateRemainingCPU();
			return(1);
		}
	}

	DPrintf("Error in EndingProc: can't find process %d\n", p);
	return(0);
}


/*  	SchedProc() is called by the kernel when it needs a decision
 *  	for which process to run next.  It will call the kernel function
 * 	GetSchedPolicy() which will return the current scheduling policy
 *   	which was previously set via SetSchedPolicy(policy). SchedProc()
 * 	should return a process PID, or 0 if there are no processes to run. 
 */

int SchedProc()
{
	int i;

	switch(GetSchedPolicy()) {

	case ARBITRARY:

		for (i = 0; i < MAXPROCS; i++) {
			if (proctab[i].valid) {
				return(proctab[i].pid);
			}
		}
		break;

	case FIFO:
		if(HEAD->valid){
			return HEAD->pid;
		}
		break;

	case LIFO:
		if(TAIL->valid){
			return TAIL->pid;
		}
		break;

	case ROUNDROBIN:
		if(ACTIVE->next != NULL){
			ACTIVE = ACTIVE->next;
			return ACTIVE->pid;
		}
		else if(ACTIVE == TAIL && HEAD->valid){
			ACTIVE = HEAD;
			return ACTIVE->pid;
		}
		break;

	case PROPORTIONAL:

		if(!HEAD->valid) break;

		struct Node* iter = HEAD;
		struct Node* next_proc = HEAD;
		while(iter != TAIL){
			iter = iter->next;
			if(iter->pass < next_proc->pass){
				next_proc = iter;
			}
		}
		if(next_proc->pass + next_proc->stride < 0){//Overflow
			iter = HEAD;
			if(iter->active_request) iter->pass -= next_proc->pass;
			while(iter != TAIL){
				iter = iter->next;
				if(iter->active_request) iter->pass -= next_proc->pass;
			}
		}

		next_proc->pass += next_proc->stride;

		return next_proc->pid;

		break;

	}
	
	return(0);
}


/*  	HandleTimerIntr() is called by the kernel whenever a timer
 * 	interrupt occurs.  Timer interrupts should occur on a fixed
 *  	periodic basis.
 */

void HandleTimerIntr()
{
	SetTimer(TIMERINTERVAL);

	switch(GetSchedPolicy()) {	// is the policy preemptive?

	case ROUNDROBIN:		// ROUNDROBIN is preemptive
		DoSched();
		break;
	case PROPORTIONAL:		// PROPORTIONAL is preemptive
		DoSched();		// make a scheduling decision
		break;

	default:			// if non-preemptive, do nothing
		break;
	}
}

/*  	MyRequestCPUrate(p,n) is called by the kernel whenever a process
 * 	identified by PID p calls RequestCPUrate(n). This is a request for
 *   	n% of CPU time if n > 0, i.e., roughly n out of every 100 quantums
 * 	should be allocated to the calling process, or to run at no fixed
 *  	allocation (the default) if n = 0.  MyRequestCPUrate(p,n) should
 * 	return 0 if successful, i.e., if such a request can be satisfied,
 *  	otherwise it should return -1, i.e., error (including if n < 0
 *  	or n > 100).  If MyRequestCPUrate(p,n) fails, it should have NO
 * 	EFFECT on the scheduling of this or any other process, i.e., AS
 *   	IF IT WERE NEVER CALLED. 
 */

int MyRequestCPUrate(int p, int n)
	// p: process whose rate to change
	// n: percent of CPU time
{
	if(n < 0 || n > 100) return -1;

	int i;

	for(i = 0; i < MAXPROCS; i++){
		if(proctab[i].valid && proctab[i].pid == p){
			if(n == 0){
				if(proctab[i].active_request){
					proctab[i].active_request = 0;
					proctab[i].pass = 0;
					CPU_allocated -= proctab[i].requested_rate;
					proctab[i].requested_rate = 0;
					proctab[i].stride = 0;
					num_requests--;
					AllocateRemainingCPU();
				}
				return 0;
			}
			
			if((n - proctab[i].requested_rate) + CPU_allocated > 100) return -1;
			
			CPU_allocated += n - proctab[i].requested_rate;
			proctab[i].requested_rate = n;
			proctab[i].stride = L/n;
			
			if(!proctab[i].active_request) num_requests++;

			proctab[i].active_request = 1;
			
		}
		proctab[i].pass = 0;
	}
	AllocateRemainingCPU();
	return(0);
}


void AllocateRemainingCPU(){
	if(num_procs == num_requests) return;

	int available_CPU = 100 - CPU_allocated;
	int CPU_per_proc = available_CPU/(num_procs - num_requests);
	

	struct Node* iter = HEAD;
	if(!iter->active_request){
		if(CPU_per_proc == 0){
			iter->stride = L;
		}
		else{
			iter->stride = L/CPU_per_proc;
		}
	}
	while(iter != TAIL){
		iter = iter->next;
		if(!iter->active_request){
			if(CPU_per_proc == 0){
				iter->stride = L;
			}
			else{
				iter->stride = L/CPU_per_proc;
			}
		}
	}
}
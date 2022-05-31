/* mycode1.c: your portion of the kernel
 *
 *   	Below are functions that are called by other parts of the kernel. 
 * 	Your ability to modify the kernel is via these functions.  You may
 *  	modify the bodies of these functions (and add code outside of them)
 * 	in any way you wish (however, you cannot change their interfaces). 
 */

#include <string.h>
#include "aux.h"
#include "sys.h"
#include "mycode1.h"

/*  	NewContext(p,c) will be called by the kernel whenever a new
 *  	process	is created.  This is essentially a notification (which you
 * 	will make use of) that this newly created process has an ID of p,
 *   	and that its initial context is pointed to by c.  Make sure you
 * 	make a copy of the contents of this context (i.e., don't copy the
 *  	pointer, but the contents pointed to by the pointer), as the pointer
 * 	will become useless after this function returns.
 */

CONTEXT contexts[MAXPROCS];

void NewContext(int p, CONTEXT *c)
	// p: ID of the new process just created
	// c: initial context for this process
{
	memcpy(&contexts[p-1], c, sizeof(CONTEXT));
}

/*  	MySwitchContext(p) should cause a context switch from the calling
 *  	process to process p. It should return the ID of the calling
 * 	process. The ID of the calling process can be determined by calling
 *   	GetCurProc(), which returns the currently running process's ID.  The
 * 	initial given implementation of MySwitchContext(p) makes use of
 *  	SwitchContext(p), which is a reference/working version of the
 * 	kernel context switching function. It is provided so that the kernel
 *  	can initially work without modification to allow the other exercises
 *  	to execute, and to illustrate proper behavior.  For Exercise F, the
 * 	call to SwitchContext(p) MUST be removed, as your solution code
 *   	will effectively replace its functionality.  Furthermore, it will
 * 	be deactivated during grading, so if you leave it in, your code will
 *  	fail and you will receive no credit. So, don't forget to remove it!
 */
 
int MySwitchContext(int p)
	// p: ID of process to switch to
{
	int magic = 0;
	
	static int cur_ID; 
	cur_ID = GetCurProc();
	
	SaveContext(&contexts[cur_ID-1]);

	if(magic==1) return cur_ID;
	else magic=1;
	
	RestoreContext(&contexts[p-1]);

	return cur_ID;
}

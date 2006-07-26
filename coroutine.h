/*
 *	Co-routine library
 *	Copyright
 *		(C) 2006 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */

#define STACK_SIZE (sizeof(void *) * 32768)

struct stack {
	struct stack *next;			/* Next free stack */
	Coroutine *caller;			/* Calling co-routine */
	int (*func)(va_list args);		/* Function to call */
	va_list args;

#ifdef USE_UCONTEXT
	ucontext_t uc[1];
#else
	jmp_buf go;
	char *stk;
#endif
};

extern struct stack *current_stack;

struct coroutine {
	struct stack *stack;		/* To restore current stack */
	Obj saved_obj_stack;		/* To restore obj stack */
#ifdef USE_UCONTEXT
	ucontext_t uc[1];
#else
	jmp_buf cont;
#endif
};

/* Suspend current co-routine and return to calling co-routine with
 * specified return value.  t points to a Coroutine structure which gets
 * filled in with data about the suspended Coroutine.  The address of this
 * structure is used by co_resume to resume the suspended co-routine. */

int co_yield(Coroutine *t, int val);

/* Suspend the current co-routine and resume a previously suspended
 * co-routine.  co_resume() returns (and the co-routine which called
 * co_resume() is resumed) when the specified co-routine either completes
 * (the function specified in co_call() returns) or yields (it calls
 * co_yield).  The return value is the second argument to co_yield or the
 * return value of the function specified in co_call().
 */

int co_resume(Coroutine *t, int val);

/* Call a function as a co-routine (it runs with its own stack).  co_call
 * returns when the specified function returns (in which case the function's
 * return value is returned) or when the co-routine yields (in which case
 * the second argument to co_yield() is returned.
 */

int co_call(int (*func)(va_list args), ...);

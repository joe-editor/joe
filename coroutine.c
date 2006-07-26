/*
 *	Co-routine library
 *	Copyright
 *		(C) 2006 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#include "types.h"

struct stack *current_stack;	/* Current stack */
struct stack *free_stacks;	/* Free stacks */

#ifndef USE_UCONTEXT

jmp_buf create_stk;	/* longjmp here to create another stack */
jmp_buf rtn_create_stk;	/* Return address for stack creation longjmp call */

/* Record current stack location */

void make_space();

void record_stack()
{
	int create = 1;
	int rtn;
	for (;;) {
		/* Save PC and SP address in jmp_buf */
		if (setjmp(current_stack->go)) {
			/* Run scheduler in this stack.  It will return with a task to continue */
			rtn = current_stack->func(current_stack->args); /* Call function... */
		} else if (!create) {
			/* Free this stack */
			Coroutine *t = current_stack->caller;
			current_stack->next = free_stacks;
			free_stacks = current_stack;
			/* Continue task */
			current_stack = t->stack;
			/* Adding 10 because we want to be able to pass 0 and
			   -1 to user, but 0 has special meaning for setjmp */
			longjmp(t->cont, rtn + 10);
		} else {
			create = 0;
			/* Create the stack (doesn't return) */
			make_space();
		}
	}
}

/* Make space on the stack */

void make_space()
{
	char buf[STACK_SIZE];
	/* Make sure compiler does not optimize out buf */
	current_stack->stk = buf;
	/* We've created the space, jump back to caller. */
	if (!setjmp(create_stk)) {
		longjmp(rtn_create_stk, 1);
	} else {
		/* When someone longjmps to create_stk, create another stack... */
		record_stack();
	}
}

#else

/* Execute function and resume calling co-routine */

int rtval;

void call_it()
{
	Coroutine *t;
	/* Execute function */
	rtval = current_stack->func(current_stack->args);
	/* Free stack and resume caller */
	t = current_stack->caller;
	current_stack->next = free_stacks;
	free_stacks = current_stack;
	current_stack = t->stack;
	setcontext(t->uc);
}

#endif

/* Allocate a stack */

struct stack *mkstack()
{
	struct stack *stack;
	if (free_stacks) {
		stack = free_stacks;
		free_stacks = stack->next;
		stack->caller = 0;
#ifdef USE_UCONTEXT
		makecontext(stack->uc, call_it, 0);
#endif
		return stack;
	}
	stack = (struct stack *)malloc(sizeof(struct stack));
	stack->caller = 0;
#ifdef USE_UCONTEXT
	getcontext(stack->uc);
	stack->uc->uc_link = 0;
	stack->uc->uc_stack.ss_size = STACK_SIZE;
	stack->uc->uc_stack.ss_sp = malloc(STACK_SIZE);
#ifdef __sgi
	stack->uc->uc_stack.ss_sp = (char *)stack->uc->uc_stack.ss_sp + STACK_SIZE - 8;
#endif
	stack->uc->uc_stack.ss_flags = 0;
	makecontext(stack->uc, call_it, 0);
	return stack;
#else
	current_stack = stack;
	if (!setjmp(rtn_create_stk))
		longjmp(create_stk, 1);
	else {
		return stack;
	}
#endif
}

/* Suspend current co-routine and return to caller */

int co_yield(Coroutine *t, int val)
{
#ifdef USE_UCONTEXT
	Coroutine *n;

	/* Save current stack */
	t->stack = current_stack;

	/* Save object stack */
	t->saved_obj_stack = obj_stack[0];
	izque(Obj,link,obj_stack);

	/* Return to creator */
	n = current_stack->caller;
	current_stack->caller = 0;
	current_stack = n->stack;

	/* Give return value to creator */
	rtval = val;

	/* Switch */
	swapcontext(t->uc, n->uc);

	/* Somebody continued us... */
	obj_stack[0] = t->saved_obj_stack;

	return rtval;
#else
	int rtn;

	/* Save current stack */
	t->stack = current_stack;

	/* Save object stack */
	t->saved_obj_stack = obj_stack[0];
	izque(Obj,link,obj_stack);

	/* Save current context */
	if (rtn = setjmp(t->cont)) {
		/* Somebody continued us... */
		obj_stack[0] = t->saved_obj_stack;
		return rtn - 10; /* 10 added to longjmp */
	} else {
		/* Return to creator */
		t = current_stack->caller;
		/* This co-routine can not jump back ever again */
		current_stack->caller = 0;

		current_stack = t->stack;

		/* Go */
		longjmp(t->cont, rtn + 10);
	}
#endif
}

/* Suspend current co-routine and resume another */

int co_resume(Coroutine *t,int val)
{
	Coroutine self[1];
#ifdef USE_UCONTEXT
	Coroutine *n;

	/* Save current stack */
	self->stack = current_stack;

	/* Save object stack */
	self->saved_obj_stack = obj_stack[0];
	izque(Obj,link,obj_stack);

	/* Resume specified coroutine */
	current_stack = t->stack;

	/* Who to resume when coroutine returns */
	current_stack->caller = self;

	/* Give return value to co_yield() */
	rtval = val;

	/* Switch */
	swapcontext(self->uc, t->uc);

	/* Somebody continued us... */
	obj_stack[0] = self->saved_obj_stack;

	return rtval;
#else
	int rtn;

	/* Save current stack */
	self->stack = current_stack;

	/* Save object stack */
	self->saved_obj_stack = obj_stack[0];
	izque(Obj,link,obj_stack);

	/* Save current context */
	if (rtn = setjmp(self->cont)) {
		/* Somebody continued us... */
		obj_stack[0] = self->saved_obj_stack;
		return rtn - 10; /* 10 added to longjmp */
	} else {
		/* Continue specified */
		current_stack = t->stack;
		
		/* Who to return to */
		current_stack->caller = self;

		/* Go */
		longjmp(t->cont, rtn + 10);
	}
#endif
}

/* Call a function as a co-routine */

int co_call(int (*func)(va_list args), ...)
{
	Coroutine self[1];
	va_list ap;

#ifdef USE_UCONTEXT
	va_start(ap, func);

	if (!current_stack) {
		current_stack = (struct stack *)malloc(sizeof(struct stack));
		current_stack->caller = 0;
	}

	/* Save current stack */
	self->stack = current_stack;

	/* Save object stack */
	self->saved_obj_stack = obj_stack[0];
	izque(Obj,link,obj_stack);

	/* Allocate stack for co-routine */
	current_stack = mkstack();

	/* Set function to call in stack */
	current_stack->func = func;
	current_stack->args = ap;

	/* Who to resume when function returns */
	current_stack->caller = self;

	/* Switch */
	swapcontext(self->uc, current_stack->uc);

	/* Somebody continued us... */
	obj_stack[0] = self->saved_obj_stack;

	va_end(ap);

	return rtval;
#else
	int rtn;
	va_start(ap, func);

	if (!current_stack) {
		/* Startup... record main stack */
		current_stack = (struct stack *)malloc(sizeof(struct stack));
		current_stack->caller = 0;
		if (!setjmp(rtn_create_stk))
			record_stack();
	}

	self->stack = current_stack;
	self->saved_obj_stack = obj_stack[0];
	izque(Obj, link, obj_stack);

	/* Allocate stack for co-routine */
	current_stack = mkstack();

	/* Set function to call in stack */
	current_stack->func = func;
	current_stack->args = ap;

	/* Set where we return to */
	current_stack->caller = self;

	/* Save current context */
	if (rtn = setjmp(self->cont)) {
		/* Somebody continued us... */
		obj_stack[0] = self->saved_obj_stack;
		va_end(ap);
		return rtn - 10; /* 10 added to longjmp */
	} else {
		/* Go */
		longjmp(current_stack->go, 1);
	}
#endif
}

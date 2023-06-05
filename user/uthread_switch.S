	.text

	/*
         * save the old thread's registers,
         * restore the new thread's registers.
         */

	.globl thread_switch
thread_switch:
	/* YOUR CODE HERE */
	// save all the registers 
    // thread_switch(t, next_thread);
	// save the registers of t, restore the registers of next_thread
	//struct thread {
	//  uint64 ra;
	//  uint64 pc;
	//
	//  // callee-saved
	//  uint64 s0;
	//  uint64 s1;
	//  uint64 s2;
	//  uint64 s3;
	//  uint64 s4;
	//  uint64 s5;
	//  uint64 s6;
	//  uint64 s7;
	//  uint64 s8;
	//  uint64 s9;
	//  uint64 s10;
	//  uint64 s11;
	//  char       stack[STACK_SIZE]; /* the thread's stack */
	//  int        state;             /* FREE, RUNNING, RUNNABLE */
	//  void       (*handler)();
	//};
    # save the user registers of t in its thread struct
	# user registers are 64 bits, so 8 bytes
	// t is in a0, next_thread is in a1
	// can't modify program counter, how do we resume where we left off?
	# this code wont work as is because its not resuming where the thread left off when it gets scheduled again
	# maybe we are not allowed to save the return address?
    sd ra, 0(a0)
	#sd pc, 8(a0)
	sd s0, 16(a0)
	sd s1, 24(a0)
	sd s2, 32(a0)
	sd s3, 40(a0)
	sd s4, 48(a0)
	sd s5, 56(a0)
	sd s6, 64(a0)
	sd s7, 72(a0)
	sd s8, 80(a0)
	sd s9, 88(a0)
	sd s10, 96(a0)
	sd s11, 104(a0)

	# restore tha user registers of next_thread
    ld ra, 0(a1)
	#ld pc, 16(a1)
	ld s0, 16(a1)
	ld s1, 24(a1)
	ld s2, 32(a1)
	ld s3, 40(a1)
	ld s4, 48(a1)
	ld s5, 56(a1)
	ld s6, 64(a1)
	ld s7, 72(a1)
	ld s8, 80(a1)
	ld s9, 88(a1)
	ld s10, 96(a1)
	ld s11, 104(a1)
	# the saved sp should be the ->stack field of a0
	ld sp, 112(a1)
	ret    /* return to ra */

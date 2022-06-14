	.text
	.file	"mas_blk_mq_tag.c"
	.globl	ufs_tagset_tag_busy_iter // -- Begin function ufs_tagset_tag_busy_iter
	.p2align	2
	.type	ufs_tagset_tag_busy_iter,@function
ufs_tagset_tag_busy_iter:               // @ufs_tagset_tag_busy_iter
// %bb.0:
	str	x27, [sp, #-96]!        // 8-byte Folded Spill
	stp	x26, x25, [sp, #16]     // 16-byte Folded Spill
	stp	x24, x23, [sp, #32]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #48]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #64]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #80]     // 16-byte Folded Spill
	ldr	x24, [x0, #304]
	mov	x19, x2
	mov	x20, x0
	mov	x21, x1
	ldr	w8, [x24, #48]
	add	x29, sp, #80            // =80
	cbz	w8, .LBB0_11
// %bb.1:
	mov	x25, xzr
.LBB0_2:                                // =>This Loop Header: Depth=1
                                        //     Child Loop BB0_5 Depth 2
	ldr	x8, [x24, #56]
	add	x22, x8, x25, lsl #6
	ldr	x8, [x22]
	cbz	x8, .LBB0_10
// %bb.3:                               //   in Loop: Header=BB0_2 Depth=1
	mov	x26, x22
	ldr	w27, [x24, #44]
	ldr	x1, [x26, #8]!
	mov	x0, x22
	mov	x2, xzr
	bl	find_next_bit
	ldr	x1, [x26]
	mov	x23, x0
	cmp	x1, w23, uxtw
	b.ls	.LBB0_10
// %bb.4:                               //   in Loop: Header=BB0_2 Depth=1
	lsl	w27, w25, w27
.LBB0_5:                                //   Parent Loop BB0_2 Depth=1
                                        // =>  This Inner Loop Header: Depth=2
	ldr	x8, [x20, #304]
	add	w9, w27, w23
	ldr	x8, [x8, #208]
	ldr	x8, [x8, w9, uxtw #3]
	cbz	x8, .LBB0_9
// %bb.6:                               //   in Loop: Header=BB0_5 Depth=2
	cbz	x21, .LBB0_9
// %bb.7:                               //   in Loop: Header=BB0_5 Depth=2
	ldr	x9, [x8, #64]
	ldr	x10, [x20, #192]
	cmp	x9, x10
	b.ne	.LBB0_9
// %bb.8:                               //   in Loop: Header=BB0_5 Depth=2
	mov	x0, x20
	mov	x1, x8
	mov	x2, x19
	mov	w3, wzr
	blr	x21
	ldr	x1, [x26]
.LBB0_9:                                //   in Loop: Header=BB0_5 Depth=2
	add	w2, w23, #1             // =1
	mov	x0, x22
	bl	find_next_bit
	ldr	x1, [x26]
	mov	x23, x0
	cmp	x1, w23, uxtw
	b.hi	.LBB0_5
.LBB0_10:                               //   in Loop: Header=BB0_2 Depth=1
	ldr	w8, [x24, #48]
	add	x25, x25, #1            // =1
	cmp	x25, x8
	b.lo	.LBB0_2
.LBB0_11:
	ldr	w8, [x24, #8]
	cbz	w8, .LBB0_23
// %bb.12:
	ldr	w8, [x24, #160]
	cbz	w8, .LBB0_23
// %bb.13:
	mov	x25, xzr
.LBB0_14:                               // =>This Loop Header: Depth=1
                                        //     Child Loop BB0_17 Depth 2
	ldr	x8, [x24, #168]
	add	x22, x8, x25, lsl #6
	ldr	x8, [x22]
	cbz	x8, .LBB0_22
// %bb.15:                              //   in Loop: Header=BB0_14 Depth=1
	mov	x26, x22
	ldr	w27, [x24, #156]
	ldr	x1, [x26, #8]!
	mov	x0, x22
	mov	x2, xzr
	bl	find_next_bit
	ldr	x1, [x26]
	mov	x23, x0
	cmp	x1, w23, uxtw
	b.ls	.LBB0_22
// %bb.16:                              //   in Loop: Header=BB0_14 Depth=1
	lsl	w27, w25, w27
.LBB0_17:                               //   Parent Loop BB0_14 Depth=1
                                        // =>  This Inner Loop Header: Depth=2
	ldr	x8, [x20, #304]
	add	w10, w27, w23
	ldr	w9, [x8, #20]
	ldr	x8, [x8, #208]
	add	w9, w10, w9
	ldr	x8, [x8, w9, uxtw #3]
	cbz	x8, .LBB0_21
// %bb.18:                              //   in Loop: Header=BB0_17 Depth=2
	cbz	x21, .LBB0_21
// %bb.19:                              //   in Loop: Header=BB0_17 Depth=2
	ldr	x9, [x8, #64]
	ldr	x10, [x20, #192]
	cmp	x9, x10
	b.ne	.LBB0_21
// %bb.20:                              //   in Loop: Header=BB0_17 Depth=2
	mov	x0, x20
	mov	x1, x8
	mov	x2, x19
	mov	w3, wzr
	blr	x21
	ldr	x1, [x26]
.LBB0_21:                               //   in Loop: Header=BB0_17 Depth=2
	add	w2, w23, #1             // =1
	mov	x0, x22
	bl	find_next_bit
	ldr	x1, [x26]
	mov	x23, x0
	cmp	x1, w23, uxtw
	b.hi	.LBB0_17
.LBB0_22:                               //   in Loop: Header=BB0_14 Depth=1
	ldr	w8, [x24, #160]
	add	x25, x25, #1            // =1
	cmp	x25, x8
	b.lo	.LBB0_14
.LBB0_23:
	ldr	w8, [x24, #4]
	cbz	w8, .LBB0_35
// %bb.24:
	ldr	w8, [x24, #104]
	cbz	w8, .LBB0_35
// %bb.25:
	mov	x25, xzr
.LBB0_26:                               // =>This Loop Header: Depth=1
                                        //     Child Loop BB0_29 Depth 2
	ldr	x8, [x24, #112]
	add	x22, x8, x25, lsl #6
	ldr	x8, [x22]
	cbz	x8, .LBB0_34
// %bb.27:                              //   in Loop: Header=BB0_26 Depth=1
	mov	x26, x22
	ldr	w27, [x24, #100]
	ldr	x1, [x26, #8]!
	mov	x0, x22
	mov	x2, xzr
	bl	find_next_bit
	ldr	x1, [x26]
	mov	x23, x0
	cmp	x1, w23, uxtw
	b.ls	.LBB0_34
// %bb.28:                              //   in Loop: Header=BB0_26 Depth=1
	lsl	w27, w25, w27
.LBB0_29:                               //   Parent Loop BB0_26 Depth=1
                                        // =>  This Inner Loop Header: Depth=2
	ldr	x8, [x20, #304]
	add	w10, w27, w23
	ldr	w9, [x8, #16]
	ldr	x8, [x8, #208]
	add	w9, w10, w9
	ldr	x8, [x8, w9, uxtw #3]
	cbz	x8, .LBB0_33
// %bb.30:                              //   in Loop: Header=BB0_29 Depth=2
	cbz	x21, .LBB0_33
// %bb.31:                              //   in Loop: Header=BB0_29 Depth=2
	ldr	x9, [x8, #64]
	ldr	x10, [x20, #192]
	cmp	x9, x10
	b.ne	.LBB0_33
// %bb.32:                              //   in Loop: Header=BB0_29 Depth=2
	orr	w3, wzr, #0x1
	mov	x0, x20
	mov	x1, x8
	mov	x2, x19
	blr	x21
	ldr	x1, [x26]
.LBB0_33:                               //   in Loop: Header=BB0_29 Depth=2
	add	w2, w23, #1             // =1
	mov	x0, x22
	bl	find_next_bit
	ldr	x1, [x26]
	mov	x23, x0
	cmp	x1, w23, uxtw
	b.hi	.LBB0_29
.LBB0_34:                               //   in Loop: Header=BB0_26 Depth=1
	ldr	w8, [x24, #104]
	add	x25, x25, #1            // =1
	cmp	x25, x8
	b.lo	.LBB0_26
.LBB0_35:
	ldp	x29, x30, [sp, #80]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #64]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #48]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #32]     // 16-byte Folded Reload
	ldp	x26, x25, [sp, #16]     // 16-byte Folded Reload
	ldr	x27, [sp], #96          // 8-byte Folded Reload
	ret
.Lfunc_end0:
	.size	ufs_tagset_tag_busy_iter, .Lfunc_end0-ufs_tagset_tag_busy_iter
                                        // -- End function
	.globl	ufs_tagset_bt_get       // -- Begin function ufs_tagset_bt_get
	.p2align	2
	.type	ufs_tagset_bt_get,@function
ufs_tagset_bt_get:                      // @ufs_tagset_bt_get
// %bb.0:
	sub	sp, sp, #144            // =144
	adrp	x8, __stack_chk_guard
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	str	x27, [sp, #48]          // 8-byte Folded Spill
	stp	x26, x25, [sp, #64]     // 16-byte Folded Spill
	stp	x24, x23, [sp, #80]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #96]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #112]    // 16-byte Folded Spill
	stp	x29, x30, [sp, #128]    // 16-byte Folded Spill
	add	x29, sp, #128           // =128
	str	x8, [sp, #40]
	cbz	x2, .LBB1_36
// %bb.1:
	ldr	x8, [x0, #8]
	mov	w24, #4096
	mov	x21, x2
	mov	x20, x1
	mov	x19, x0
	movk	w24, #64, lsl #16
	tbnz	w8, #23, .LBB1_6
// %bb.2:
	and	x8, x8, x24
	cbz	x8, .LBB1_6
// %bb.3:
	mov	x0, x19
	bl	ufs_mq_vip_tag_wait_cnt
	cbz	w0, .LBB1_6
// %bb.4:
	ldr	x8, [x19, #32]
	cbz	x8, .LBB1_7
// %bb.5:
	bl	ktime_get
	ldr	x8, [x19, #32]
	mov	w9, #25856
	movk	w9, #7629, lsl #16
	add	x8, x8, x9
	cmp	x0, x8
	b.le	.LBB1_7
.LBB1_6:
	mov	x0, x20
	bl	__sbitmap_queue_get
	mov	w23, w0
	cmn	w0, #1                  // =1
	b.ne	.LBB1_37
.LBB1_7:
	ldr	x0, [x19]
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB1_9
// %bb.8:
	ldrb	w8, [x19, #16]
	tbnz	w8, #2, .LBB1_36
.LBB1_9:
	adrp	x8, autoremove_wake_function
	add	x8, x8, :lo12:autoremove_wake_function
	mov	x9, sp
	//APP
	mrs x25, sp_el0
	//NO_APP
	stp	x25, x8, [sp, #8]
	add	x8, x9, #24             // =24
	str	wzr, [sp]
	stp	x8, x8, [sp, #24]
	ldr	x8, [x19, #8]
	tbz	w8, #23, .LBB1_11
// %bb.10:
	mov	x0, x19
	bl	ufs_mq_inc_vip_wait_cnt
	ldr	x8, [x19, #8]
.LBB1_11:
	tbnz	w8, #23, .LBB1_14
// %bb.12:
	and	x8, x8, x24
	cbz	x8, .LBB1_14
// %bb.13:
	bl	ktime_get
	str	x0, [x19, #32]
.LBB1_14:
	ldr	x8, [x20, #40]
	mov	x1, sp
	orr	w2, wzr, #0x2
	add	x22, x8, #8             // =8
	mov	x0, x22
	bl	prepare_to_wait
	mov	x0, x19
	mov	x1, x20
	mov	x2, x21
	bl	ufs_try_get_tag
	mov	w23, w0
	cmn	w0, #1                  // =1
	b.ne	.LBB1_41
// %bb.15:
	adrp	x26, cpu_number
	adrp	x27, __per_cpu_offset
	add	x26, x26, :lo12:cpu_number
	add	x27, x27, :lo12:__per_cpu_offset
	//APP
	//NO_APP
.LBB1_16:                               // =>This Inner Loop Header: Depth=1
	ldr	x8, [x19, #48]
	ldr	x8, [x8, #144]
	ldr	x8, [x8, #1816]
	cbz	x8, .LBB1_22
// %bb.17:                              //   in Loop: Header=BB1_16 Depth=1
	ldr	x8, [x8, #80]
	cbz	x8, .LBB1_22
// %bb.18:                              //   in Loop: Header=BB1_16 Depth=1
	blr	x8
	ldr	x8, [x19, #8]
	tbnz	w8, #23, .LBB1_25
.LBB1_19:                               //   in Loop: Header=BB1_16 Depth=1
	and	x8, x8, x24
	cbz	x8, .LBB1_25
// %bb.20:                              //   in Loop: Header=BB1_16 Depth=1
	orr	w0, wzr, #0x1
	bl	io_schedule_timeout
	cbnz	x0, .LBB1_26
// %bb.21:                              //   in Loop: Header=BB1_16 Depth=1
	mov	x0, x19
	bl	reset_vip_wait_cnt
	b	.LBB1_26
.LBB1_22:                               //   in Loop: Header=BB1_16 Depth=1
	//APP
	//NO_APP
	ldr	w8, [x25, #16]
	subs	w8, w8, #1              // =1
	str	w8, [x25, #16]
	b.ne	.LBB1_24
// %bb.23:                              //   in Loop: Header=BB1_16 Depth=1
	ldr	x8, [x25]
	tbnz	w8, #1, .LBB1_33
.LBB1_24:                               //   in Loop: Header=BB1_16 Depth=1
	ldr	x8, [x19, #8]
	tbz	w8, #23, .LBB1_19
.LBB1_25:                               //   in Loop: Header=BB1_16 Depth=1
	bl	io_schedule
.LBB1_26:                               //   in Loop: Header=BB1_16 Depth=1
	ldr	x8, [x19]
	ldr	w9, [x25, #16]
	add	w9, w9, #1              // =1
	str	w9, [x25, #16]
	//APP
	//NO_APP
	//APP
	.if 1 == 1
661:
	mrs x9, tpidr_el1
662:
.pushsection .altinstructions,"a"
 .word 661b - .
 .if 0 == 0
 .word 663f - .
 .else
 .word 0- .
 .endif
 .hword 11
 .byte 662b-661b
 .byte 664f-663f
.popsection
 .if 0 == 0
.pushsection .altinstr_replacement, "a"
663:
	mrs x9, tpidr_el2
664:
	.popsection
	.org	. - (664b-663b) + (662b-661b)
	.org	. - (662b-661b) + (664b-663b)
.else
	663:
	664:
	.endif
.endif

	//NO_APP
	ldr	w9, [x9, x26]
	ldr	x8, [x8, #328]
	ldr	x10, [x19]
	//APP
	//NO_APP
	ldr	x9, [x27, x9, lsl #3]
	add	x8, x9, x8
	str	x8, [x19, #48]
	ldrsw	x8, [x8, #64]
	ldr	x9, [x10, #320]
	ldr	x10, [x10, #344]
	ldr	w8, [x9, x8, lsl #2]
	ldr	x8, [x10, x8, lsl #3]
	str	x8, [x19, #56]
	cbz	x8, .LBB1_39
// %bb.27:                              //   in Loop: Header=BB1_16 Depth=1
	cmp	x21, x8
	b.ne	.LBB1_29
.LBB1_28:                               //   in Loop: Header=BB1_16 Depth=1
	mov	x1, sp
	mov	x0, x22
	bl	finish_wait
	mov	x1, sp
	orr	w2, wzr, #0x2
	mov	x0, x22
	bl	prepare_to_wait
	mov	x0, x19
	mov	x1, x20
	mov	x2, x21
	bl	ufs_try_get_tag
	cmn	w0, #1                  // =1
	b.eq	.LBB1_16
	b	.LBB1_40
.LBB1_29:                               //   in Loop: Header=BB1_16 Depth=1
	ldr	x9, [x19, #8]
	tbz	w9, #11, .LBB1_34
// %bb.30:                              //   in Loop: Header=BB1_16 Depth=1
	ldr	x10, [x8, #304]
	tst	x9, x24
	b.ne	.LBB1_35
// %bb.31:                              //   in Loop: Header=BB1_16 Depth=1
	add	x20, x10, #40           // =40
.LBB1_32:                               //   in Loop: Header=BB1_16 Depth=1
	mov	x21, x8
	b	.LBB1_28
.LBB1_33:                               //   in Loop: Header=BB1_16 Depth=1
	bl	preempt_schedule
	ldr	x8, [x19, #8]
	tbz	w8, #23, .LBB1_19
	b	.LBB1_25
.LBB1_34:                               //   in Loop: Header=BB1_16 Depth=1
	ldr	x9, [x8, #304]
	add	x20, x9, #96            // =96
	b	.LBB1_32
.LBB1_35:                               //   in Loop: Header=BB1_16 Depth=1
	add	x20, x10, #152          // =152
	b	.LBB1_32
.LBB1_36:
	mov	w23, #-1
.LBB1_37:
	adrp	x9, __stack_chk_guard
	ldr	x8, [sp, #40]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB1_44
// %bb.38:
	mov	w0, w23
	ldp	x29, x30, [sp, #128]    // 16-byte Folded Reload
	ldp	x20, x19, [sp, #112]    // 16-byte Folded Reload
	ldp	x22, x21, [sp, #96]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #80]     // 16-byte Folded Reload
	ldp	x26, x25, [sp, #64]     // 16-byte Folded Reload
	ldr	x27, [sp, #48]          // 8-byte Folded Reload
	add	sp, sp, #144            // =144
	ret
.LBB1_39:
	mov	w23, #-1
	ldrb	w8, [x19, #10]
	tbz	w8, #7, .LBB1_43
	b	.LBB1_42
.LBB1_40:
	mov	w23, w0
.LBB1_41:
	ldrb	w8, [x19, #10]
	tbz	w8, #7, .LBB1_43
.LBB1_42:
	mov	x0, x19
	bl	ufs_mq_dec_vip_wait_cnt
.LBB1_43:
	mov	x1, sp
	mov	x0, x22
	bl	finish_wait
	b	.LBB1_37
.LBB1_44:
	bl	__stack_chk_fail
.Lfunc_end1:
	.size	ufs_tagset_bt_get, .Lfunc_end1-ufs_tagset_bt_get
                                        // -- End function
	.globl	ufs_tagset_bt_get_unistore // -- Begin function ufs_tagset_bt_get_unistore
	.p2align	2
	.type	ufs_tagset_bt_get_unistore,@function
ufs_tagset_bt_get_unistore:             // @ufs_tagset_bt_get_unistore
// %bb.0:
	sub	sp, sp, #144            // =144
	adrp	x8, __stack_chk_guard
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	stp	x28, x27, [sp, #48]     // 16-byte Folded Spill
	stp	x26, x25, [sp, #64]     // 16-byte Folded Spill
	stp	x24, x23, [sp, #80]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #96]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #112]    // 16-byte Folded Spill
	stp	x29, x30, [sp, #128]    // 16-byte Folded Spill
	str	x8, [sp, #40]
	ldr	x21, [x0, #56]
	add	x29, sp, #128           // =128
	cbz	x21, .LBB2_26
// %bb.1:
	ldr	x23, [x21, #304]
	ldr	x8, [x0, #8]
	mov	w25, #4096
	mov	x19, x0
	mov	x20, x1
	add	x22, x23, #40           // =40
	movk	w25, #64, lsl #16
	tbnz	w8, #23, .LBB2_6
// %bb.2:
	and	x8, x8, x25
	cbz	x8, .LBB2_6
// %bb.3:
	mov	x0, x19
	bl	ufs_mq_vip_tag_wait_cnt
	cbz	w0, .LBB2_6
// %bb.4:
	ldr	x8, [x19, #32]
	cbz	x8, .LBB2_7
// %bb.5:
	bl	ktime_get
	ldr	x8, [x19, #32]
	mov	w9, #25856
	movk	w9, #7629, lsl #16
	add	x8, x8, x9
	cmp	x0, x8
	b.le	.LBB2_7
.LBB2_6:
	mov	x0, x22
	bl	__sbitmap_queue_get
	mov	w24, w0
	cmn	w0, #1                  // =1
	b.ne	.LBB2_27
.LBB2_7:
	adrp	x8, autoremove_wake_function
	add	x8, x8, :lo12:autoremove_wake_function
	mov	x9, sp
	//APP
	mrs x26, sp_el0
	//NO_APP
	stp	x26, x8, [sp, #8]
	add	x8, x9, #24             // =24
	str	wzr, [sp]
	stp	x8, x8, [sp, #24]
	ldr	x8, [x23, #80]
	adrp	x28, cpu_number
	adrp	x27, __per_cpu_offset
	add	x28, x28, :lo12:cpu_number
	add	x23, x8, #8             // =8
	add	x27, x27, :lo12:__per_cpu_offset
	//APP
	//NO_APP
	b	.LBB2_9
.LBB2_8:                                //   in Loop: Header=BB2_9 Depth=1
	mov	x1, sp
	mov	x0, x23
	bl	finish_wait
.LBB2_9:                                // =>This Inner Loop Header: Depth=1
	mov	x1, sp
	orr	w2, wzr, #0x2
	mov	x0, x23
	bl	prepare_to_wait
	ldrb	w8, [x19, #16]
	tbnz	w8, #1, .LBB2_12
// %bb.10:                              //   in Loop: Header=BB2_9 Depth=1
	ldr	x0, [x19]
	ldr	x1, [x19, #24]
	bl	mas_blk_match_expected_lba
	tbz	w0, #0, .LBB2_12
// %bb.11:                              //   in Loop: Header=BB2_9 Depth=1
	ldr	x8, [x19, #16]
	ldr	x9, [x19, #56]
	orr	x8, x8, #0x2
	str	x8, [x19, #16]
	ldr	x8, [x9, #304]
	add	x22, x8, #152           // =152
	orr	w8, wzr, #0x1
	strb	w8, [x20]
.LBB2_12:                               //   in Loop: Header=BB2_9 Depth=1
	mov	x0, x19
	mov	x1, x22
	mov	x2, x21
	bl	ufs_try_get_tag
	mov	w24, w0
	cmn	w0, #1                  // =1
	b.ne	.LBB2_29
// %bb.13:                              //   in Loop: Header=BB2_9 Depth=1
	ldr	x8, [x19, #48]
	ldr	x8, [x8, #144]
	ldr	x8, [x8, #1816]
	cbz	x8, .LBB2_16
// %bb.14:                              //   in Loop: Header=BB2_9 Depth=1
	ldr	x8, [x8, #80]
	cbz	x8, .LBB2_16
// %bb.15:                              //   in Loop: Header=BB2_9 Depth=1
	blr	x8
	b	.LBB2_18
.LBB2_16:                               //   in Loop: Header=BB2_9 Depth=1
	//APP
	//NO_APP
	ldr	w8, [x26, #16]
	subs	w8, w8, #1              // =1
	str	w8, [x26, #16]
	b.ne	.LBB2_18
// %bb.17:                              //   in Loop: Header=BB2_9 Depth=1
	ldr	x8, [x26]
	tbnz	w8, #1, .LBB2_25
.LBB2_18:                               //   in Loop: Header=BB2_9 Depth=1
	mov	w0, #25
	bl	io_schedule_timeout
	ldr	x8, [x19]
	ldr	w9, [x26, #16]
	add	w9, w9, #1              // =1
	str	w9, [x26, #16]
	//APP
	//NO_APP
	//APP
	.if 1 == 1
661:
	mrs x9, tpidr_el1
662:
.pushsection .altinstructions,"a"
 .word 661b - .
 .if 0 == 0
 .word 663f - .
 .else
 .word 0- .
 .endif
 .hword 11
 .byte 662b-661b
 .byte 664f-663f
.popsection
 .if 0 == 0
.pushsection .altinstr_replacement, "a"
663:
	mrs x9, tpidr_el2
664:
	.popsection
	.org	. - (664b-663b) + (662b-661b)
	.org	. - (662b-661b) + (664b-663b)
.else
	663:
	664:
	.endif
.endif

	//NO_APP
	ldr	w9, [x9, x28]
	ldr	x8, [x8, #328]
	ldr	x10, [x19]
	//APP
	//NO_APP
	ldr	x9, [x27, x9, lsl #3]
	add	x8, x9, x8
	str	x8, [x19, #48]
	ldrsw	x8, [x8, #64]
	ldr	x9, [x10, #320]
	ldr	x10, [x10, #344]
	ldr	w8, [x9, x8, lsl #2]
	ldr	x8, [x10, x8, lsl #3]
	str	x8, [x19, #56]
	cbz	x8, .LBB2_29
// %bb.19:                              //   in Loop: Header=BB2_9 Depth=1
	cmp	x21, x8
	b.eq	.LBB2_8
// %bb.20:                              //   in Loop: Header=BB2_9 Depth=1
	ldr	x9, [x19, #8]
	tst	x9, x25
	b.ne	.LBB2_23
// %bb.21:                              //   in Loop: Header=BB2_9 Depth=1
	ldrb	w9, [x19, #16]
	tbnz	w9, #1, .LBB2_23
// %bb.22:                              //   in Loop: Header=BB2_9 Depth=1
	ldr	x9, [x8, #304]
	add	x22, x9, #40            // =40
	b	.LBB2_24
.LBB2_23:                               //   in Loop: Header=BB2_9 Depth=1
	ldr	x9, [x8, #304]
	add	x22, x9, #152           // =152
.LBB2_24:                               //   in Loop: Header=BB2_9 Depth=1
	mov	x21, x8
	b	.LBB2_8
.LBB2_25:                               //   in Loop: Header=BB2_9 Depth=1
	bl	preempt_schedule
	b	.LBB2_18
.LBB2_26:
	mov	w24, #-1
.LBB2_27:
	adrp	x9, __stack_chk_guard
	ldr	x8, [sp, #40]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB2_30
// %bb.28:
	mov	w0, w24
	ldp	x29, x30, [sp, #128]    // 16-byte Folded Reload
	ldp	x20, x19, [sp, #112]    // 16-byte Folded Reload
	ldp	x22, x21, [sp, #96]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #80]     // 16-byte Folded Reload
	ldp	x26, x25, [sp, #64]     // 16-byte Folded Reload
	ldp	x28, x27, [sp, #48]     // 16-byte Folded Reload
	add	sp, sp, #144            // =144
	ret
.LBB2_29:
	mov	x1, sp
	mov	x0, x23
	bl	finish_wait
	b	.LBB2_27
.LBB2_30:
	bl	__stack_chk_fail
.Lfunc_end2:
	.size	ufs_tagset_bt_get_unistore, .Lfunc_end2-ufs_tagset_bt_get_unistore
                                        // -- End function
	.globl	ufs_tagset_all_tag_busy_iter // -- Begin function ufs_tagset_all_tag_busy_iter
	.p2align	2
	.type	ufs_tagset_all_tag_busy_iter,@function
ufs_tagset_all_tag_busy_iter:           // @ufs_tagset_all_tag_busy_iter
// %bb.0:
	stp	x26, x25, [sp, #-80]!   // 16-byte Folded Spill
	stp	x24, x23, [sp, #16]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #32]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #48]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #64]     // 16-byte Folded Spill
	ldr	x8, [x0, #208]
	mov	x19, x2
	mov	x20, x0
	mov	x21, x1
	add	x29, sp, #64            // =64
	cbz	x8, .LBB3_10
// %bb.1:
	ldr	w8, [x20, #48]
	cbz	w8, .LBB3_10
// %bb.2:
	mov	x24, xzr
.LBB3_3:                                // =>This Loop Header: Depth=1
                                        //     Child Loop BB3_6 Depth 2
	ldr	x8, [x20, #56]
	add	x22, x8, x24, lsl #6
	ldr	x8, [x22]
	cbz	x8, .LBB3_9
// %bb.4:                               //   in Loop: Header=BB3_3 Depth=1
	mov	x25, x22
	ldr	w26, [x20, #44]
	ldr	x1, [x25, #8]!
	mov	x0, x22
	mov	x2, xzr
	bl	find_next_bit
	ldr	x1, [x25]
	mov	x23, x0
	cmp	x1, w23, uxtw
	b.ls	.LBB3_9
// %bb.5:                               //   in Loop: Header=BB3_3 Depth=1
	lsl	w26, w24, w26
.LBB3_6:                                //   Parent Loop BB3_3 Depth=1
                                        // =>  This Inner Loop Header: Depth=2
	cbz	x21, .LBB3_8
// %bb.7:                               //   in Loop: Header=BB3_6 Depth=2
	ldr	x8, [x20, #208]
	add	w9, w26, w23
	mov	x1, x19
	mov	w2, wzr
	ldr	x0, [x8, w9, uxtw #3]
	blr	x21
	ldr	x1, [x25]
.LBB3_8:                                //   in Loop: Header=BB3_6 Depth=2
	add	w2, w23, #1             // =1
	mov	x0, x22
	bl	find_next_bit
	ldr	x1, [x25]
	mov	x23, x0
	cmp	x1, w23, uxtw
	b.hi	.LBB3_6
.LBB3_9:                                //   in Loop: Header=BB3_3 Depth=1
	ldr	w8, [x20, #48]
	add	x24, x24, #1            // =1
	cmp	x24, x8
	b.lo	.LBB3_3
.LBB3_10:
	ldr	w8, [x20, #8]
	cbz	w8, .LBB3_21
// %bb.11:
	ldr	x8, [x20, #208]
	cbz	x8, .LBB3_21
// %bb.12:
	ldr	w8, [x20, #160]
	cbz	w8, .LBB3_21
// %bb.13:
	mov	x24, xzr
.LBB3_14:                               // =>This Loop Header: Depth=1
                                        //     Child Loop BB3_17 Depth 2
	ldr	x8, [x20, #168]
	add	x22, x8, x24, lsl #6
	ldr	x8, [x22]
	cbz	x8, .LBB3_20
// %bb.15:                              //   in Loop: Header=BB3_14 Depth=1
	mov	x25, x22
	ldr	w26, [x20, #156]
	ldr	x1, [x25, #8]!
	mov	x0, x22
	mov	x2, xzr
	bl	find_next_bit
	ldr	x1, [x25]
	mov	x23, x0
	cmp	x1, w23, uxtw
	b.ls	.LBB3_20
// %bb.16:                              //   in Loop: Header=BB3_14 Depth=1
	lsl	w26, w24, w26
.LBB3_17:                               //   Parent Loop BB3_14 Depth=1
                                        // =>  This Inner Loop Header: Depth=2
	cbz	x21, .LBB3_19
// %bb.18:                              //   in Loop: Header=BB3_17 Depth=2
	ldr	w8, [x20, #20]
	ldr	x9, [x20, #208]
	add	w10, w26, w23
	mov	x1, x19
	add	w8, w10, w8
	ldr	x0, [x9, w8, uxtw #3]
	mov	w2, wzr
	blr	x21
	ldr	x1, [x25]
.LBB3_19:                               //   in Loop: Header=BB3_17 Depth=2
	add	w2, w23, #1             // =1
	mov	x0, x22
	bl	find_next_bit
	ldr	x1, [x25]
	mov	x23, x0
	cmp	x1, w23, uxtw
	b.hi	.LBB3_17
.LBB3_20:                               //   in Loop: Header=BB3_14 Depth=1
	ldr	w8, [x20, #160]
	add	x24, x24, #1            // =1
	cmp	x24, x8
	b.lo	.LBB3_14
.LBB3_21:
	ldr	w8, [x20, #4]
	cbz	w8, .LBB3_32
// %bb.22:
	ldr	x8, [x20, #208]
	cbz	x8, .LBB3_32
// %bb.23:
	ldr	w8, [x20, #104]
	cbz	w8, .LBB3_32
// %bb.24:
	mov	x24, xzr
.LBB3_25:                               // =>This Loop Header: Depth=1
                                        //     Child Loop BB3_28 Depth 2
	ldr	x8, [x20, #112]
	add	x22, x8, x24, lsl #6
	ldr	x8, [x22]
	cbz	x8, .LBB3_31
// %bb.26:                              //   in Loop: Header=BB3_25 Depth=1
	mov	x25, x22
	ldr	w26, [x20, #100]
	ldr	x1, [x25, #8]!
	mov	x0, x22
	mov	x2, xzr
	bl	find_next_bit
	ldr	x1, [x25]
	mov	x23, x0
	cmp	x1, w23, uxtw
	b.ls	.LBB3_31
// %bb.27:                              //   in Loop: Header=BB3_25 Depth=1
	lsl	w26, w24, w26
.LBB3_28:                               //   Parent Loop BB3_25 Depth=1
                                        // =>  This Inner Loop Header: Depth=2
	cbz	x21, .LBB3_30
// %bb.29:                              //   in Loop: Header=BB3_28 Depth=2
	ldr	w8, [x20, #16]
	ldr	x9, [x20, #208]
	add	w10, w26, w23
	orr	w2, wzr, #0x1
	add	w8, w10, w8
	ldr	x0, [x9, w8, uxtw #3]
	mov	x1, x19
	blr	x21
	ldr	x1, [x25]
.LBB3_30:                               //   in Loop: Header=BB3_28 Depth=2
	add	w2, w23, #1             // =1
	mov	x0, x22
	bl	find_next_bit
	ldr	x1, [x25]
	mov	x23, x0
	cmp	x1, w23, uxtw
	b.hi	.LBB3_28
.LBB3_31:                               //   in Loop: Header=BB3_25 Depth=1
	ldr	w8, [x20, #104]
	add	x24, x24, #1            // =1
	cmp	x24, x8
	b.lo	.LBB3_25
.LBB3_32:
	ldp	x29, x30, [sp, #64]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #48]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #32]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #16]     // 16-byte Folded Reload
	ldp	x26, x25, [sp], #80     // 16-byte Folded Reload
	ret
.Lfunc_end3:
	.size	ufs_tagset_all_tag_busy_iter, .Lfunc_end3-ufs_tagset_all_tag_busy_iter
                                        // -- End function
	.globl	ufs_tagset_free_tags    // -- Begin function ufs_tagset_free_tags
	.p2align	2
	.type	ufs_tagset_free_tags,@function
ufs_tagset_free_tags:                   // @ufs_tagset_free_tags
// %bb.0:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	mov	x19, x0
	ldr	x0, [x0, #80]
	add	x29, sp, #16            // =16
	bl	kfree
	ldr	x0, [x19, #64]
	bl	free_percpu
	ldr	x0, [x19, #56]
	bl	kfree
	ldr	x0, [x19, #136]
	str	xzr, [x19, #56]
	bl	kfree
	ldr	x0, [x19, #120]
	bl	free_percpu
	ldr	x0, [x19, #112]
	bl	kfree
	ldr	x0, [x19, #192]
	str	xzr, [x19, #112]
	bl	kfree
	ldr	x0, [x19, #176]
	bl	free_percpu
	ldr	x0, [x19, #168]
	bl	kfree
	mov	x0, x19
	str	xzr, [x19, #168]
	bl	kfree
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldr	x19, [sp], #32          // 8-byte Folded Reload
	ret
.Lfunc_end4:
	.size	ufs_tagset_free_tags, .Lfunc_end4-ufs_tagset_free_tags
                                        // -- End function
	.globl	ufs_mq_tag_wakeup_all   // -- Begin function ufs_mq_tag_wakeup_all
	.p2align	2
	.type	ufs_mq_tag_wakeup_all,@function
ufs_mq_tag_wakeup_all:                  // @ufs_mq_tag_wakeup_all
// %bb.0:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	mov	x19, x0
	add	x0, x0, #40             // =40
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	bl	sbitmap_queue_wake_all
	add	x0, x19, #96            // =96
	bl	sbitmap_queue_wake_all
	add	x0, x19, #152           // =152
	bl	sbitmap_queue_wake_all
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	mov	w0, wzr
	ldr	x19, [sp], #32          // 8-byte Folded Reload
	ret
.Lfunc_end5:
	.size	ufs_mq_tag_wakeup_all, .Lfunc_end5-ufs_mq_tag_wakeup_all
                                        // -- End function
	.globl	ufs_tagset_init_tags    // -- Begin function ufs_tagset_init_tags
	.p2align	2
	.type	ufs_tagset_init_tags,@function
ufs_tagset_init_tags:                   // @ufs_tagset_init_tags
// %bb.0:
	str	x25, [sp, #-80]!        // 8-byte Folded Spill
	adrp	x8, kmalloc_caches+64
	stp	x22, x21, [sp, #32]     // 16-byte Folded Spill
	mov	w22, w0
	ldr	x0, [x8, :lo12:kmalloc_caches+64]
	mov	w8, #192
	movk	w8, #320, lsl #16
	stp	x24, x23, [sp, #16]     // 16-byte Folded Spill
	mov	w23, w2
	mov	w24, w1
	add	w1, w8, #8, lsl #12     // =32768
	orr	w2, wzr, #0xf0
	stp	x20, x19, [sp, #48]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #64]     // 16-byte Folded Spill
	add	x29, sp, #64            // =64
	mov	w21, w4
	mov	w20, w3
	bl	kmem_cache_alloc_trace
	mov	x19, x0
	cbz	x0, .LBB6_7
// %bb.1:
	sub	w8, w22, w24
	stp	w22, w24, [x19]
	sub	w22, w8, w23
	adrp	x0, .L.str.1
	cmp	w21, #1                 // =1
	add	x0, x0, :lo12:.L.str.1
	mov	w1, w22
	mov	w2, w24
	mov	w3, w23
	str	w23, [x19, #8]
	cset	w25, eq
	bl	printk
	mov	w4, #192
	add	x0, x19, #40            // =40
	mov	w2, #-1
	movk	w4, #320, lsl #16
	mov	w1, w22
	mov	w3, w25
	mov	w5, w20
	bl	sbitmap_queue_init_node
	cbnz	w0, .LBB6_6
// %bb.2:
	ldr	w1, [x19, #4]
	cmp	w21, #1                 // =1
	mov	w4, #192
	cset	w3, eq
	add	x0, x19, #96            // =96
	mov	w2, #-1
	movk	w4, #320, lsl #16
	mov	w5, w20
	bl	sbitmap_queue_init_node
	cbnz	w0, .LBB6_5
// %bb.3:
	ldr	w1, [x19, #8]
	cmp	w21, #1                 // =1
	mov	w4, #192
	cset	w3, eq
	add	x0, x19, #152           // =152
	mov	w2, #-1
	movk	w4, #320, lsl #16
	mov	w5, w20
	bl	sbitmap_queue_init_node
	cbz	w0, .LBB6_8
// %bb.4:
	ldr	x0, [x19, #136]
	bl	kfree
	ldr	x0, [x19, #120]
	bl	free_percpu
	ldr	x0, [x19, #112]
	bl	kfree
	str	xzr, [x19, #112]
.LBB6_5:
	ldr	x0, [x19, #80]
	bl	kfree
	ldr	x0, [x19, #64]
	bl	free_percpu
	ldr	x0, [x19, #56]
	bl	kfree
	str	xzr, [x19, #56]
.LBB6_6:
	adrp	x0, .L.str.2
	adrp	x1, .L__func__.ufs_tagset_init_bitmap_tags
	add	x0, x0, :lo12:.L.str.2
	add	x1, x1, :lo12:.L__func__.ufs_tagset_init_bitmap_tags
	bl	printk
	adrp	x0, .L.str
	add	x0, x0, :lo12:.L.str
	bl	printk
	mov	x0, x19
	bl	kfree
	mov	x19, xzr
.LBB6_7:
	mov	x0, x19
	ldp	x29, x30, [sp, #64]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #48]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #32]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #16]     // 16-byte Folded Reload
	ldr	x25, [sp], #80          // 8-byte Folded Reload
	ret
.LBB6_8:
	ldr	w8, [x19, #4]
	stp	wzr, w22, [x19, #12]
	add	w8, w8, w22
	str	w8, [x19, #20]
	b	.LBB6_7
.Lfunc_end6:
	.size	ufs_tagset_init_tags, .Lfunc_end6-ufs_tagset_init_tags
                                        // -- End function
	.p2align	2               // -- Begin function ufs_try_get_tag
	.type	ufs_try_get_tag,@function
ufs_try_get_tag:                        // @ufs_try_get_tag
// %bb.0:
	stp	x22, x21, [sp, #-48]!   // 16-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	ldr	x8, [x0, #8]
	mov	w22, #4096
	mov	x21, x2
	mov	x19, x1
	mov	x20, x0
	movk	w22, #64, lsl #16
	add	x29, sp, #32            // =32
	tbnz	w8, #23, .LBB7_5
// %bb.1:
	and	x8, x8, x22
	cbz	x8, .LBB7_5
// %bb.2:
	mov	x0, x20
	bl	ufs_mq_vip_tag_wait_cnt
	cbz	w0, .LBB7_5
// %bb.3:
	ldr	x8, [x20, #32]
	cbz	x8, .LBB7_6
// %bb.4:
	bl	ktime_get
	ldr	x8, [x20, #32]
	mov	w9, #25856
	movk	w9, #7629, lsl #16
	add	x8, x8, x9
	cmp	x0, x8
	b.le	.LBB7_6
.LBB7_5:
	mov	x0, x19
	bl	__sbitmap_queue_get
	cmn	w0, #1                  // =1
	b.ne	.LBB7_12
.LBB7_6:
	mov	x0, x21
	mov	w1, wzr
	bl	blk_mq_run_hw_queue
	ldr	x8, [x20, #8]
	tbnz	w8, #23, .LBB7_11
// %bb.7:
	and	x8, x8, x22
	cbz	x8, .LBB7_11
// %bb.8:
	mov	x0, x20
	bl	ufs_mq_vip_tag_wait_cnt
	cbz	w0, .LBB7_11
// %bb.9:
	ldr	x8, [x20, #32]
	cbz	x8, .LBB7_13
// %bb.10:
	bl	ktime_get
	ldr	x8, [x20, #32]
	mov	w9, #25856
	movk	w9, #7629, lsl #16
	add	x8, x8, x9
	cmp	x0, x8
	b.le	.LBB7_13
.LBB7_11:
	mov	x0, x19
	bl	__sbitmap_queue_get
.LBB7_12:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldp	x22, x21, [sp], #48     // 16-byte Folded Reload
	ret
.LBB7_13:
	mov	w0, #-1
	b	.LBB7_12
.Lfunc_end7:
	.size	ufs_try_get_tag, .Lfunc_end7-ufs_try_get_tag
                                        // -- End function
	.type	.L.str,@object          // @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	"\0013[BLK-IO]Failed to init tagset bitmaps!\n"
	.size	.L.str, 42

	.type	.L.str.1,@object        // @.str.1
.L.str.1:
	.asciz	"\0013[BLK-IO]depth = %d, reserved = %d, high prio = %d \r\n"
	.size	.L.str.1, 55

	.type	.L.str.2,@object        // @.str.2
.L.str.2:
	.asciz	"\0013[BLK-IO]%s: error nomem\n"
	.size	.L.str.2, 27

	.type	.L__func__.ufs_tagset_init_bitmap_tags,@object // @__func__.ufs_tagset_init_bitmap_tags
.L__func__.ufs_tagset_init_bitmap_tags:
	.asciz	"ufs_tagset_init_bitmap_tags"
	.size	.L__func__.ufs_tagset_init_bitmap_tags, 28


	.ident	"Android (27847 based on r353983c) clang version 9.0.3 (https://android.googlesource.com/toolchain/clang dbdb7a2acf33dc76cd6f506c323253b8d892ae05) (https://android.googlesource.com/toolchain/llvm f32df87f6769ededcd9e2a6c3c4c45c1baf361c2) (based on LLVM 9.0.3svn)"
	.section	".note.GNU-stack","",@progbits

	.text
	.file	"mas_blk_latency.c"
	.p2align	2               // -- Begin function mas_blk_latency_bio_submit_func
	.type	mas_blk_latency_bio_submit_func,@function
mas_blk_latency_bio_submit_func:        // @mas_blk_latency_bio_submit_func
// %bb.0:
	str	x25, [sp, #-80]!        // 8-byte Folded Spill
	stp	x24, x23, [sp, #16]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #32]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #48]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #64]     // 16-byte Folded Spill
	ldr	x8, [x0, #8]
	add	x29, sp, #64            // =64
	cbz	x8, .LBB0_17
// %bb.1:
	ldr	x21, [x8, #1120]
	cbz	x21, .LBB0_17
// %bb.2:
	ldr	w8, [x21, #1700]
	cbz	w8, .LBB0_17
// %bb.3:
	ldrb	w8, [x0, #145]
	mov	x19, x0
	cmp	w8, #76                 // =76
	b.eq	.LBB0_16
// %bb.4:
	add	x20, x19, #184          // =184
	orr	w8, wzr, #0x1
	mov	x0, x20
	mov	w1, wzr
	mov	x2, xzr
	mov	x3, xzr
	str	w8, [x19, #232]
	bl	init_timer_key
	adrp	x8, __mas_blk_latency_check_timer_expire
	add	x8, x8, :lo12:__mas_blk_latency_check_timer_expire
	adrp	x9, jiffies
	stp	x8, x19, [x19, #208]
	ldr	x22, [x9, :lo12:jiffies]
	ldr	w8, [x19, #16]
	tst	w8, #0x3000000
	b.eq	.LBB0_6
// %bb.5:
	tst	w8, #0x1000000
	mov	w8, #100
	mov	w9, #500
	csel	w0, w9, w8, eq
	b	.LBB0_9
.LBB0_6:
	mov	w9, #4096
	movk	w9, #64, lsl #16
	tst	w8, w9
	b.eq	.LBB0_8
// %bb.7:
	mov	w0, #1000
	b	.LBB0_9
.LBB0_8:
	ldr	w0, [x21, #1704]
.LBB0_9:
	bl	__msecs_to_jiffies
	ldrh	w8, [x19, #20]
	add	x9, x0, x22
	str	x9, [x19, #200]
	add	x10, x19, #476          // =476
	orr	w8, w8, #0x100
	strh	w8, [x19, #20]
	//APP
	dmb ish
	//NO_APP
	//APP
	// atomic_add
	prfm	pstl1strm, [x10]
1:	ldxr	w8, [x10]
	add	w8, w8, #1
	stxr	w9, w8, [x10]
	cbnz	w9, 1b
	//NO_APP
	adrp	x23, cpu_number
	//APP
	.if 1 == 1
661:
	mrs x8, tpidr_el1
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
	mrs x8, tpidr_el2
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
	add	x23, x23, :lo12:cpu_number
	adrp	x24, __cpu_possible_mask
	//APP
	//NO_APP
	ldr	w8, [x8, x23]
	ldrb	w0, [x24, :lo12:__cpu_possible_mask]
	add	w21, w8, #1             // =1
	bl	__sw_hweight64
	udiv	w8, w21, w0
	msub	w21, w8, w0, w21
	add	w8, w21, #63            // =63
	cmp	w21, #0                 // =0
	csel	w8, w8, w21, lt
	adrp	x9, __cpu_online_mask
	asr	w8, w8, #6
	add	x9, x9, :lo12:__cpu_online_mask
	ldr	x8, [x9, w8, sxtw #3]
	orr	w9, wzr, #0x1
	lsl	x9, x9, x21
	tst	x9, x8
	b.ne	.LBB0_13
// %bb.10:
	adrp	x22, __cpu_online_mask
	mov	w21, #-1
	add	x22, x22, :lo12:__cpu_online_mask
	adrp	x25, nr_cpu_ids
.LBB0_11:                               // =>This Inner Loop Header: Depth=1
	mov	w0, w21
	mov	x1, x22
	bl	cpumask_next
	ldr	w8, [x25, :lo12:nr_cpu_ids]
	mov	w21, w0
	cmp	w0, w8
	b.hs	.LBB0_13
// %bb.12:                              //   in Loop: Header=BB0_11 Depth=1
	//APP
	.if 1 == 1
661:
	mrs x8, tpidr_el1
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
	mrs x8, tpidr_el2
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
	ldr	w8, [x8, x23]
	cmp	w21, w8
	b.eq	.LBB0_11
.LBB0_13:
	ldrb	w0, [x24, :lo12:__cpu_possible_mask]
	bl	__sw_hweight64
	cmp	w21, w0
	b.lo	.LBB0_15
// %bb.14:
	//APP
	.if 1 == 1
661:
	mrs x8, tpidr_el1
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
	mrs x8, tpidr_el2
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
	ldr	w21, [x8, x23]
.LBB0_15:
	mov	x0, x20
	mov	w1, w21
	bl	add_timer_on
	//APP
	mrs x8, sp_el0
	//NO_APP
	str	x8, [x19, #112]
	ldr	w9, [x8, #1976]
	str	w9, [x19, #120]
	ldr	w9, [x8, #1980]
	str	w9, [x19, #124]
	ldr	x9, [x8, #2440]
	ldr	x8, [x8, #2448]
	stp	x9, x8, [x19, #128]
.LBB0_16:
	mov	w8, #76
	str	xzr, [x19, #104]
	stp	xzr, xzr, [x19, #168]
	strb	w8, [x19, #145]
	stp	xzr, xzr, [x19, #152]
.LBB0_17:
	ldp	x29, x30, [sp, #64]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #48]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #32]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #16]     // 16-byte Folded Reload
	ldr	x25, [sp], #80          // 8-byte Folded Reload
	ret
.Lfunc_end0:
	.size	mas_blk_latency_bio_submit_func, .Lfunc_end0-mas_blk_latency_bio_submit_func
                                        // -- End function
	.p2align	2               // -- Begin function mas_blk_latency_bio_endio_func
	.type	mas_blk_latency_bio_endio_func,@function
mas_blk_latency_bio_endio_func:         // @mas_blk_latency_bio_endio_func
// %bb.0:
	str	x23, [sp, #-64]!        // 8-byte Folded Spill
	stp	x22, x21, [sp, #16]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	//APP
	mrs x8, sp_el0
	//NO_APP
	ldr	w8, [x8, #16]
	ldr	x20, [x0, #96]
	mov	w9, #1000
	mov	x19, x0
	tst	w8, #0x1fff00
	mov	w8, #34464
	movk	w8, #1, lsl #16
	csel	x22, x8, x9, eq
	add	x29, sp, #48            // =48
	cbz	x20, .LBB1_8
// %bb.1:
	bl	ktime_get
	ldr	w9, [x19, #16]
	ldr	x8, [x19, #152]
	tst	w9, #0x3000000
	b.eq	.LBB1_3
// %bb.2:
	tst	w9, #0x1000000
	mov	w9, #100
	mov	w10, #500
	csel	x9, x10, x9, eq
	b	.LBB1_6
.LBB1_3:
	mov	w10, #4096
	movk	w10, #64, lsl #16
	tst	w9, w10
	b.eq	.LBB1_5
// %bb.4:
	mov	w9, #1000
	b	.LBB1_6
.LBB1_5:
	ldr	w9, [x20, #1704]
.LBB1_6:
	mov	w10, #16960
	movk	w10, #15, lsl #16
	madd	x8, x9, x10, x8
	cmp	x0, x8
	b.le	.LBB1_8
// %bb.7:
	adrp	x0, latency_endbio_ratelmt_state
	adrp	x1, .L.str.24
	add	x0, x0, :lo12:latency_endbio_ratelmt_state
	add	x1, x1, :lo12:.L.str.24
	bl	___ratelimit
	cbnz	w0, .LBB1_18
.LBB1_8:
	add	x0, x19, #184           // =184
	strb	wzr, [x19, #145]
	bl	del_timer
	cbz	w0, .LBB1_11
// %bb.9:
	mov	x0, x19
	bl	bio_put
.LBB1_10:
	str	wzr, [x19, #232]
	str	xzr, [x19, #104]
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]     // 16-byte Folded Reload
	ldr	x23, [sp], #64          // 8-byte Folded Reload
	ret
.LBB1_11:
	ldr	w8, [x19, #232]
	cbz	w8, .LBB1_10
// %bb.12:
	orr	w9, wzr, #0x1
	mov	w8, wzr
	sub	x9, x9, x22
.LBB1_13:                               // =>This Inner Loop Header: Depth=1
	tbnz	w8, #0, .LBB1_10
// %bb.14:                              //   in Loop: Header=BB1_13 Depth=1
	mov	w0, #42950
	mov	x20, x9
	bl	__const_udelay
	ldr	w10, [x19, #232]
	cmp	x20, #0                 // =0
	cset	w8, eq
	add	x9, x20, #1             // =1
	cbnz	w10, .LBB1_13
// %bb.15:
	cbnz	x20, .LBB1_10
// %bb.16:
	adrp	x0, latency_endbio_ratelmt_state
	adrp	x1, .L.str.25
	add	x0, x0, :lo12:latency_endbio_ratelmt_state
	add	x1, x1, :lo12:.L.str.25
	bl	___ratelimit
	cbz	w0, .LBB1_10
// %bb.17:
	bl	dump_stack
	b	.LBB1_10
.LBB1_18:
	mov	x0, x20
	bl	mas_blk_get_lld
	ldr	x23, [x20, #1816]
	mov	x21, x0
	mov	x0, x19
	mov	w1, wzr
	bl	mas_blk_dump_bio
	mov	x0, x20
	mov	w1, wzr
	bl	mas_blk_dump_queue_status
	ldr	x8, [x19, #312]
	cbz	x8, .LBB1_20
// %bb.19:
	blr	x8
.LBB1_20:
	ldr	x8, [x21, #40]
	cbz	x8, .LBB1_22
// %bb.21:
	mov	x0, x20
	mov	w1, wzr
	blr	x8
.LBB1_22:
	cbz	x23, .LBB1_8
// %bb.23:
	ldr	x8, [x23, #160]
	cbz	x8, .LBB1_8
// %bb.24:
	mov	x0, x20
	mov	w1, wzr
	blr	x8
	b	.LBB1_8
.Lfunc_end1:
	.size	mas_blk_latency_bio_endio_func, .Lfunc_end1-mas_blk_latency_bio_endio_func
                                        // -- End function
	.p2align	2               // -- Begin function mas_blk_latency_req_init_func
	.type	mas_blk_latency_req_init_func,@function
mas_blk_latency_req_init_func:          // @mas_blk_latency_req_init_func
// %bb.0:
	ldr	x8, [x0, #64]
	cbz	x8, .LBB2_3
// %bb.1:
	ldr	w8, [x8, #1700]
	cbz	w8, .LBB2_3
// %bb.2:
	mov	w8, #76
	stp	xzr, xzr, [x0, #416]
	stp	xzr, xzr, [x0, #400]
	stp	xzr, xzr, [x0, #384]
	stp	xzr, xzr, [x0, #368]
	stp	xzr, xzr, [x0, #352]
	stp	xzr, xzr, [x0, #336]
	stp	xzr, xzr, [x0, #320]
	str	xzr, [x0, #312]
	//APP
	mrs x9, sp_el0
	//NO_APP
	strb	w8, [x0, #280]
	str	x9, [x0, #240]
	ldr	w8, [x9, #1976]
	str	w8, [x0, #248]
	ldr	w8, [x9, #1980]
	str	w8, [x0, #252]
	ldr	x8, [x9, #2440]
	ldr	x9, [x9, #2448]
	stp	x8, x9, [x0, #256]
	ret
.LBB2_3:
	strb	wzr, [x0, #280]
	ret
.Lfunc_end2:
	.size	mas_blk_latency_req_init_func, .Lfunc_end2-mas_blk_latency_req_init_func
                                        // -- End function
	.p2align	2               // -- Begin function mas_blk_latency_req_free_func
	.type	mas_blk_latency_req_free_func,@function
mas_blk_latency_req_free_func:          // @mas_blk_latency_req_free_func
// %bb.0:
	ldr	x8, [x0, #120]
	strb	wzr, [x0, #280]
	str	xzr, [x0, #240]
	cbz	x8, .LBB3_2
.LBB3_1:                                // =>This Inner Loop Header: Depth=1
	str	xzr, [x8, #104]
	ldr	x8, [x8]
	str	x8, [x0, #120]
	cbnz	x8, .LBB3_1
.LBB3_2:
	ret
.Lfunc_end3:
	.size	mas_blk_latency_req_free_func, .Lfunc_end3-mas_blk_latency_req_free_func
                                        // -- End function
	.globl	mas_blk_latency_check_timer_expire // -- Begin function mas_blk_latency_check_timer_expire
	.p2align	2
	.type	mas_blk_latency_check_timer_expire,@function
mas_blk_latency_check_timer_expire:     // @mas_blk_latency_check_timer_expire
// %bb.0:
	str	x23, [sp, #-64]!        // 8-byte Folded Spill
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	mov	x19, x0
	adrp	x0, latency_log_protect_lock
	add	x0, x0, :lo12:latency_log_protect_lock
	stp	x22, x21, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	add	x29, sp, #48            // =48
	bl	_raw_spin_lock_irqsave
	ldrb	w3, [x19, #145]
	mov	x20, x0
	cmp	w3, #76                 // =76
	b.ne	.LBB4_17
// %bb.1:
	ldr	x8, [x19, #8]
	cbz	x8, .LBB4_6
// %bb.2:
	ldr	x21, [x8, #1120]
	cbz	x21, .LBB4_6
// %bb.3:
	mov	x0, x21
	bl	mas_blk_get_lld
	adrp	x0, latency_log_ratelmt_state
	adrp	x1, .L.str.21
	add	x0, x0, :lo12:latency_log_ratelmt_state
	add	x1, x1, :lo12:.L.str.21
	bl	___ratelimit
	cbz	w0, .LBB4_16
// %bb.4:
	ldr	w8, [x19, #16]
	mov	w9, #4096
	movk	w9, #64, lsl #16
	tst	w8, w9
	b.eq	.LBB4_7
// %bb.5:
	mov	w0, wzr
	b	.LBB4_8
.LBB4_6:
	adrp	x0, .L.str.20
	adrp	x1, .L__func__.mas_blk_latency_check_timer_expire
	add	x0, x0, :lo12:.L.str.20
	add	x1, x1, :lo12:.L__func__.mas_blk_latency_check_timer_expire
	mov	w2, #298
	bl	printk
	b	.LBB4_16
.LBB4_7:
	orr	w0, wzr, #0x1
.LBB4_8:
	bl	dsm_block_record
	mov	x0, x21
	bl	mas_blk_get_lld
	ldr	x23, [x21, #1816]
	mov	x22, x0
	mov	x0, x19
	mov	w1, wzr
	bl	mas_blk_dump_bio
	mov	x0, x21
	mov	w1, wzr
	bl	mas_blk_dump_queue_status
	ldr	x8, [x19, #312]
	cbz	x8, .LBB4_10
// %bb.9:
	blr	x8
.LBB4_10:
	ldr	x8, [x22, #40]
	cbz	x8, .LBB4_12
// %bb.11:
	mov	x0, x21
	mov	w1, wzr
	blr	x8
.LBB4_12:
	cbz	x23, .LBB4_15
// %bb.13:
	ldr	x8, [x23, #160]
	cbz	x8, .LBB4_15
// %bb.14:
	mov	x0, x21
	mov	w1, wzr
	blr	x8
.LBB4_15:
	ldp	w0, w1, [x19, #120]
	adrp	x2, .L.str.21
	add	x2, x2, :lo12:.L.str.21
	bl	iowp_report
.LBB4_16:
	adrp	x0, latency_log_protect_lock
	add	x0, x0, :lo12:latency_log_protect_lock
	mov	x1, x20
	bl	_raw_spin_unlock_irqrestore
	mov	x0, x19
	str	wzr, [x19, #232]
	bl	bio_put
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]     // 16-byte Folded Reload
	ldr	x23, [sp], #64          // 8-byte Folded Reload
	ret
.LBB4_17:
	adrp	x0, .L.str.19
	adrp	x1, .L__func__.mas_blk_latency_check_timer_expire
	add	x0, x0, :lo12:.L.str.19
	add	x1, x1, :lo12:.L__func__.mas_blk_latency_check_timer_expire
	mov	w2, #293
	bl	printk
	b	.LBB4_16
.Lfunc_end4:
	.size	mas_blk_latency_check_timer_expire, .Lfunc_end4-mas_blk_latency_check_timer_expire
                                        // -- End function
	.globl	mas_blk_latency_bio_check // -- Begin function mas_blk_latency_bio_check
	.p2align	2
	.type	mas_blk_latency_bio_check,@function
mas_blk_latency_bio_check:              // @mas_blk_latency_bio_check
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	mov	x19, x0
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	cbz	w1, .LBB5_3
// %bb.1:
	ldrb	w8, [x19, #145]
	cmp	w8, #76                 // =76
	b.ne	.LBB5_6
// %bb.2:
	cmp	w1, #1                  // =1
	mov	w20, w1
	b.ls	.LBB5_4
	b	.LBB5_5
.LBB5_3:
	mov	x20, xzr
.LBB5_4:
	adrp	x8, bio_stage_cfg
	add	x8, x8, :lo12:bio_stage_cfg
	add	x8, x8, x20, lsl #4
	ldr	x8, [x8, #8]
	mov	x0, x19
	blr	x8
.LBB5_5:
	bl	ktime_get
	add	x8, x19, x20, lsl #3
	str	x0, [x8, #152]
.LBB5_6:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end5:
	.size	mas_blk_latency_bio_check, .Lfunc_end5-mas_blk_latency_bio_check
                                        // -- End function
	.globl	mas_blk_latency_req_check // -- Begin function mas_blk_latency_req_check
	.p2align	2
	.type	mas_blk_latency_req_check,@function
mas_blk_latency_req_check:              // @mas_blk_latency_req_check
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	mov	x19, x0
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	cbz	w1, .LBB6_4
// %bb.1:
	ldrb	w8, [x19, #280]
	cmp	w8, #76                 // =76
	b.ne	.LBB6_8
// %bb.2:
	cmp	w1, #14                 // =14
	b.ne	.LBB6_6
// %bb.3:
	orr	w20, wzr, #0xe
	b	.LBB6_5
.LBB6_4:
	mov	x20, xzr
.LBB6_5:
	adrp	x8, req_stage_cfg
	add	x8, x8, :lo12:req_stage_cfg
	add	x8, x8, x20, lsl #4
	ldr	x8, [x8, #8]
	mov	x0, x19
	blr	x8
	b	.LBB6_7
.LBB6_6:
	mov	w20, w1
.LBB6_7:
	bl	ktime_get
	add	x8, x19, x20, lsl #3
	str	x0, [x8, #312]
.LBB6_8:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end6:
	.size	mas_blk_latency_req_check, .Lfunc_end6-mas_blk_latency_req_check
                                        // -- End function
	.globl	mas_blk_latency_for_merge // -- Begin function mas_blk_latency_for_merge
	.p2align	2
	.type	mas_blk_latency_for_merge,@function
mas_blk_latency_for_merge:              // @mas_blk_latency_for_merge
// %bb.0:
	ldr	x8, [x1, #120]
	cbz	x8, .LBB7_2
.LBB7_1:                                // =>This Inner Loop Header: Depth=1
	str	x0, [x8, #104]
	ldr	x8, [x8]
	cbnz	x8, .LBB7_1
.LBB7_2:
	ret
.Lfunc_end7:
	.size	mas_blk_latency_for_merge, .Lfunc_end7-mas_blk_latency_for_merge
                                        // -- End function
	.globl	mas_blk_queue_latency_init // -- Begin function mas_blk_queue_latency_init
	.p2align	2
	.type	mas_blk_queue_latency_init,@function
mas_blk_queue_latency_init:             // @mas_blk_queue_latency_init
// %bb.0:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	mov	x19, x0
	bl	mas_blk_get_lld
	ldr	w8, [x0, #48]
	cbnz	w8, .LBB8_2
// %bb.1:
	mov	w8, #2000
	str	w8, [x0, #48]
.LBB8_2:
	str	w8, [x19, #1704]
	ldr	w8, [x0, #24]
	ubfx	w8, w8, #1, #1
	str	w8, [x19, #1700]
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldr	x19, [sp], #32          // 8-byte Folded Reload
	ret
.Lfunc_end8:
	.size	mas_blk_queue_latency_init, .Lfunc_end8-mas_blk_queue_latency_init
                                        // -- End function
	.section	.init.text,"ax",@progbits
	.globl	mas_blk_latency_init    // -- Begin function mas_blk_latency_init
	.p2align	2
	.type	mas_blk_latency_init,@function
mas_blk_latency_init:                   // @mas_blk_latency_init
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	adrp	x0, latency_log_protect_lock
	adrp	x1, .L.str.22
	adrp	x2, mas_blk_latency_init.__key
	add	x0, x0, :lo12:latency_log_protect_lock
	add	x1, x1, :lo12:.L.str.22
	add	x2, x2, :lo12:mas_blk_latency_init.__key
	mov	x29, sp
	bl	__raw_spin_lock_init
	bl	iowp_workqueue_init
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end9:
	.size	mas_blk_latency_init, .Lfunc_end9-mas_blk_latency_init
                                        // -- End function
	.text
	.globl	blk_mq_tagset_latency_warning_set // -- Begin function blk_mq_tagset_latency_warning_set
	.p2align	2
	.type	blk_mq_tagset_latency_warning_set,@function
blk_mq_tagset_latency_warning_set:      // @blk_mq_tagset_latency_warning_set
// %bb.0:
	ldr	x8, [x0, #40]
	cmp	w1, #0                  // =0
	mov	w9, #2000
	csel	w9, w9, w1, eq
	orr	x8, x8, #0x2
	str	x8, [x0, #40]
	str	w9, [x0, #64]
	ret
.Lfunc_end10:
	.size	blk_mq_tagset_latency_warning_set, .Lfunc_end10-blk_mq_tagset_latency_warning_set
                                        // -- End function
	.globl	blk_queue_latency_warning_set // -- Begin function blk_queue_latency_warning_set
	.p2align	2
	.type	blk_queue_latency_warning_set,@function
blk_queue_latency_warning_set:          // @blk_queue_latency_warning_set
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	mov	w19, w1
	mov	x20, x0
	bl	mas_blk_get_lld
	ldr	x8, [x0, #24]
	cmp	w19, #0                 // =0
	mov	w9, #2000
	orr	w10, wzr, #0x1
	csel	w9, w9, w19, eq
	orr	x8, x8, #0x2
	str	w9, [x0, #48]
	str	x8, [x0, #24]
	str	w9, [x20, #1704]
	str	w10, [x20, #1700]
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end11:
	.size	blk_queue_latency_warning_set, .Lfunc_end11-blk_queue_latency_warning_set
                                        // -- End function
	.globl	mas_queue_io_latency_warning_threshold_store // -- Begin function mas_queue_io_latency_warning_threshold_store
	.p2align	2
	.type	mas_queue_io_latency_warning_threshold_store,@function
mas_queue_io_latency_warning_threshold_store: // @mas_queue_io_latency_warning_threshold_store
// %bb.0:
	sub	sp, sp, #64             // =64
	adrp	x8, __stack_chk_guard
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	mov	x20, x0
	mov	x0, sp
	stp	x22, x21, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	add	x29, sp, #48            // =48
	mov	x19, x2
	str	x8, [sp, #8]
	bl	queue_var_store
	tbnz	x0, #63, .LBB12_4
// %bb.1:
	ldr	x2, [sp]
	adrp	x0, .L.str.23
	adrp	x1, .L__func__.mas_queue_io_latency_warning_threshold_store
	add	x0, x0, :lo12:.L.str.23
	add	x1, x1, :lo12:.L__func__.mas_queue_io_latency_warning_threshold_store
	bl	printk
	ldr	x21, [sp]
	mov	x0, x20
	cmp	x21, #0                 // =0
	cset	w22, ne
	bl	mas_blk_get_lld
	ldr	w8, [x20, #1700]
	cmp	w8, w22
	b.eq	.LBB12_3
// %bb.2:
	str	w22, [x20, #1700]
	ldr	x8, [x0, #24]
	cmp	x21, #0                 // =0
	and	x9, x8, #0xfffffffffffffffd
	orr	x8, x8, #0x2
	csel	x8, x8, x9, ne
	str	x8, [x0, #24]
.LBB12_3:
	ldr	w21, [sp]
	mov	x0, x20
	bl	mas_blk_get_lld
	cmp	w21, #0                 // =0
	mov	w8, #2000
	csel	w8, w8, w21, eq
	str	w8, [x0, #48]
	str	w8, [x20, #1704]
.LBB12_4:
	adrp	x9, __stack_chk_guard
	ldr	x8, [sp, #8]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB12_6
// %bb.5:
	mov	x0, x19
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]     // 16-byte Folded Reload
	add	sp, sp, #64             // =64
	ret
.LBB12_6:
	bl	__stack_chk_fail
.Lfunc_end12:
	.size	mas_queue_io_latency_warning_threshold_store, .Lfunc_end12-mas_queue_io_latency_warning_threshold_store
                                        // -- End function
	.type	.L.str,@object          // @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	"BIO_SUBMIT"
	.size	.L.str, 11

	.type	.L.str.1,@object        // @.str.1
.L.str.1:
	.asciz	"BIO_END"
	.size	.L.str.1, 8

	.type	.L.str.2,@object        // @.str.2
.L.str.2:
	.asciz	"BIO_MAKE_REQ"
	.size	.L.str.2, 13

	.type	.L.str.3,@object        // @.str.3
.L.str.3:
	.asciz	"BIO_WBT_DONE"
	.size	.L.str.3, 13

	.type	bio_stage_cfg,@object   // @bio_stage_cfg
	.section	.rodata,"a",@progbits
	.globl	bio_stage_cfg
	.p2align	3
bio_stage_cfg:
	.xword	.L.str
	.xword	mas_blk_latency_bio_submit_func
	.xword	.L.str.1
	.xword	mas_blk_latency_bio_endio_func
	.xword	.L.str.2
	.xword	0
	.xword	.L.str.3
	.xword	0
	.size	bio_stage_cfg, 64

	.type	.L.str.4,@object        // @.str.4
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.4:
	.asciz	"REQ_INIT"
	.size	.L.str.4, 9

	.type	.L.str.5,@object        // @.str.5
.L.str.5:
	.asciz	"FSEQ_PREFLUSH"
	.size	.L.str.5, 14

	.type	.L.str.6,@object        // @.str.6
.L.str.6:
	.asciz	"FSEQ_DATA"
	.size	.L.str.6, 10

	.type	.L.str.7,@object        // @.str.7
.L.str.7:
	.asciz	"FSEQ_POSTFLUSH"
	.size	.L.str.7, 15

	.type	.L.str.8,@object        // @.str.8
.L.str.8:
	.asciz	"FSEQ_DONE"
	.size	.L.str.8, 10

	.type	.L.str.9,@object        // @.str.9
.L.str.9:
	.asciz	"REQ_RQ_START"
	.size	.L.str.9, 13

	.type	.L.str.10,@object       // @.str.10
.L.str.10:
	.asciz	"REQ_MQ_ADDTO_PLUGLIST"
	.size	.L.str.10, 22

	.type	.L.str.11,@object       // @.str.11
.L.str.11:
	.asciz	"REQ_MQ_FLUSH_PLUGLIST"
	.size	.L.str.11, 22

	.type	.L.str.12,@object       // @.str.12
.L.str.12:
	.asciz	"REQ_MQ_ADDTO_SYNC_LIST"
	.size	.L.str.12, 23

	.type	.L.str.13,@object       // @.str.13
.L.str.13:
	.asciz	"REQ_MQ_ADDTO_ASYNC_LIST"
	.size	.L.str.13, 24

	.type	.L.str.14,@object       // @.str.14
.L.str.14:
	.asciz	"REQ_MQ_QUEUE_RQ"
	.size	.L.str.14, 16

	.type	.L.str.15,@object       // @.str.15
.L.str.15:
	.asciz	"REQ_SQ_REQUEUE"
	.size	.L.str.15, 15

	.type	.L.str.16,@object       // @.str.16
.L.str.16:
	.asciz	"REQ_SCSI_SFT_IRQ_DONE"
	.size	.L.str.16, 22

	.type	.L.str.17,@object       // @.str.17
.L.str.17:
	.asciz	"REQ_RQ_UPDATE"
	.size	.L.str.17, 14

	.type	.L.str.18,@object       // @.str.18
.L.str.18:
	.asciz	"REQ_RQ_FREE"
	.size	.L.str.18, 12

	.type	req_stage_cfg,@object   // @req_stage_cfg
	.section	.rodata,"a",@progbits
	.globl	req_stage_cfg
	.p2align	3
req_stage_cfg:
	.xword	.L.str.4
	.xword	mas_blk_latency_req_init_func
	.xword	.L.str.5
	.xword	0
	.xword	.L.str.6
	.xword	0
	.xword	.L.str.7
	.xword	0
	.xword	.L.str.8
	.xword	0
	.xword	.L.str.9
	.xword	0
	.xword	.L.str.10
	.xword	0
	.xword	.L.str.11
	.xword	0
	.xword	.L.str.12
	.xword	0
	.xword	.L.str.13
	.xword	0
	.xword	.L.str.14
	.xword	0
	.xword	.L.str.15
	.xword	0
	.xword	.L.str.16
	.xword	0
	.xword	.L.str.17
	.xword	0
	.xword	.L.str.18
	.xword	mas_blk_latency_req_free_func
	.size	req_stage_cfg, 240

	.type	latency_log_protect_lock,@object // @latency_log_protect_lock
	.local	latency_log_protect_lock
	.comm	latency_log_protect_lock,24,8
	.type	.L.str.19,@object       // @.str.19
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.19:
	.asciz	"\0013[BLK-IO]%s %d Invalid fs_io_flag: 0x%x\n"
	.size	.L.str.19, 42

	.type	.L__func__.mas_blk_latency_check_timer_expire,@object // @__func__.mas_blk_latency_check_timer_expire
.L__func__.mas_blk_latency_check_timer_expire:
	.asciz	"mas_blk_latency_check_timer_expire"
	.size	.L__func__.mas_blk_latency_check_timer_expire, 35

	.type	.L.str.20,@object       // @.str.20
.L.str.20:
	.asciz	"\0013[BLK-IO]%s %d Invalid request queue !\n"
	.size	.L.str.20, 41

	.type	latency_log_ratelmt_state,@object // @latency_log_ratelmt_state
	.data
	.p2align	3
latency_log_ratelmt_state:
	.zero	4
	.word	3735899821              // 0xdead4ead
	.word	4294967295              // 0xffffffff
	.zero	4
	.xword	-1
	.word	750                     // 0x2ee
	.word	5                       // 0x5
	.word	0                       // 0x0
	.word	0                       // 0x0
	.xword	0                       // 0x0
	.xword	0                       // 0x0
	.size	latency_log_ratelmt_state, 56

	.type	.L.str.21,@object       // @.str.21
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.21:
	.asciz	"io latency"
	.size	.L.str.21, 11

	.type	mas_blk_latency_init.__key,@object // @mas_blk_latency_init.__key
	.local	mas_blk_latency_init.__key
	.comm	mas_blk_latency_init.__key,1,1
	.type	.L.str.22,@object       // @.str.22
.L.str.22:
	.asciz	"&(&latency_log_protect_lock)->rlock"
	.size	.L.str.22, 36

	.type	.L.str.23,@object       // @.str.23
.L.str.23:
	.asciz	"\0013[BLK-IO]%s: io latency warning threshold is set to %lu\n"
	.size	.L.str.23, 58

	.type	.L__func__.mas_queue_io_latency_warning_threshold_store,@object // @__func__.mas_queue_io_latency_warning_threshold_store
.L__func__.mas_queue_io_latency_warning_threshold_store:
	.asciz	"mas_queue_io_latency_warning_threshold_store"
	.size	.L__func__.mas_queue_io_latency_warning_threshold_store, 45

	.type	latency_endbio_ratelmt_state,@object // @latency_endbio_ratelmt_state
	.data
	.p2align	3
latency_endbio_ratelmt_state:
	.zero	4
	.word	3735899821              // 0xdead4ead
	.word	4294967295              // 0xffffffff
	.zero	4
	.xword	-1
	.word	1250                    // 0x4e2
	.word	10                      // 0xa
	.word	0                       // 0x0
	.word	0                       // 0x0
	.xword	0                       // 0x0
	.xword	0                       // 0x0
	.size	latency_endbio_ratelmt_state, 56

	.type	.L.str.24,@object       // @.str.24
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.24:
	.asciz	"bio timeout"
	.size	.L.str.24, 12

	.type	.L.str.25,@object       // @.str.25
.L.str.25:
	.asciz	"Failed to wait io latency timer"
	.size	.L.str.25, 32


	.ident	"Android (27847 based on r353983c) clang version 9.0.3 (https://android.googlesource.com/toolchain/clang dbdb7a2acf33dc76cd6f506c323253b8d892ae05) (https://android.googlesource.com/toolchain/llvm f32df87f6769ededcd9e2a6c3c4c45c1baf361c2) (based on LLVM 9.0.3svn)"
	.section	".note.GNU-stack","",@progbits

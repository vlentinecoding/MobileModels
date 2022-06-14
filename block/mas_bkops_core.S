	.text
	.file	"mas_bkops_core.c"
	.globl	bkops_idle_work_func    // -- Begin function bkops_idle_work_func
	.p2align	2
	.type	bkops_idle_work_func,@function
bkops_idle_work_func:                   // @bkops_idle_work_func
// %bb.0:
	sub	sp, sp, #64             // =64
	adrp	x8, __stack_chk_guard
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	stp	x22, x21, [sp, #16]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	str	x8, [sp, #8]
	str	wzr, [sp, #4]
	ldur	x20, [x0, #-24]
	mov	x19, x0
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	add	x29, sp, #48            // =48
	bl	mutex_lock
	adrp	x21, bkops_enabled
	ldrb	w8, [x21, :lo12:bkops_enabled]
	cmp	w8, #1                  // =1
	b.ne	.LBB0_2
// %bb.1:
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_unlock
	b	.LBB0_8
.LBB0_2:
	ldr	x8, [x20, #8]
	ldur	x0, [x19, #-16]
	add	x1, sp, #4              // =4
	blr	x8
	adrp	x8, user_manual_gc
	ldrb	w9, [x8, :lo12:user_manual_gc]
	mov	w20, w0
	cbz	w9, .LBB0_5
// %bb.3:
	ldr	w9, [sp, #4]
	cbnz	w9, .LBB0_5
// %bb.4:
	strb	wzr, [x8, :lo12:user_manual_gc]
.LBB0_5:
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_unlock
	cbz	w20, .LBB0_8
// %bb.6:
	adrp	x0, .L.str
	add	x0, x0, :lo12:.L.str
.LBB0_7:
	bl	printk
	b	.LBB0_19
.LBB0_8:
	mov	x20, x19
	ldr	x8, [x20, #96]!
	and	x8, x8, #0xfffffffffffffffd
	str	x8, [x20]
	ldr	w8, [sp, #4]
	ldur	x0, [x20, #-104]
	str	w8, [x20, #72]
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB0_12
// %bb.9:
	ldur	w8, [x19, #-32]
	cmp	w8, #2                  // =2
	b.ne	.LBB0_12
// %bb.10:
	ldr	w8, [sp, #4]
	cbnz	w8, .LBB0_13
// %bb.11:
	orr	w0, wzr, #0x7
	mov	x1, x20
	bl	clear_bit
.LBB0_12:
	ldr	w8, [sp, #4]
	cbz	w8, .LBB0_19
.LBB0_13:
	ldr	x8, [x20]
	tbnz	w8, #0, .LBB0_19
// %bb.14:
	ldur	x22, [x19, #-24]
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_lock
	ldrb	w8, [x21, :lo12:bkops_enabled]
	cmp	w8, #1                  // =1
	b.ne	.LBB0_16
// %bb.15:
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_unlock
	b	.LBB0_18
.LBB0_16:
	ldr	x8, [x22]
	ldur	x0, [x19, #-16]
	orr	w1, wzr, #0x1
	blr	x8
	mov	w19, w0
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_unlock
	cbz	w19, .LBB0_18
// %bb.17:
	adrp	x0, .L.str.1
	add	x0, x0, :lo12:.L.str.1
	b	.LBB0_7
.LBB0_18:
	mov	w0, wzr
	mov	x1, x20
	bl	set_bit
.LBB0_19:
	adrp	x9, __stack_chk_guard
	ldr	x8, [sp, #8]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB0_21
// %bb.20:
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]     // 16-byte Folded Reload
	add	sp, sp, #64             // =64
	ret
.LBB0_21:
	bl	__stack_chk_fail
.Lfunc_end0:
	.size	bkops_idle_work_func, .Lfunc_end0-bkops_idle_work_func
                                        // -- End function
	.globl	bkops_io_busyidle_notify_handler // -- Begin function bkops_io_busyidle_notify_handler
	.p2align	2
	.type	bkops_io_busyidle_notify_handler,@function
bkops_io_busyidle_notify_handler:       // @bkops_io_busyidle_notify_handler
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	sub	w8, w1, #2              // =2
	cmp	w8, #2                  // =2
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	b.lo	.LBB1_22
// %bb.1:
	ldr	x19, [x0, #40]
	mov	w20, w1
	cbz	w1, .LBB1_12
// %bb.2:
	cmp	w20, #1                 // =1
	b.ne	.LBB1_20
// %bb.3:
	ldr	x0, [x19, #24]
	bl	blk_queue_query_unistore_enable
	tbnz	w0, #0, .LBB1_5
// %bb.4:
	mov	x0, x19
	bl	bkops_should_query
	cbz	w0, .LBB1_23
.LBB1_5:
	ldr	x0, [x19, #24]
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB1_11
// %bb.6:
	ldr	w8, [x19]
	cmp	w8, #2                  // =2
	b.ne	.LBB1_9
// %bb.7:
	add	x1, x19, #128           // =128
	orr	w0, wzr, #0x6
	bl	set_bit
	ldr	x8, [x19, #128]
	tbz	w8, #7, .LBB1_22
// %bb.8:
	ldr	w8, [x19]
	cmp	w8, #2                  // =2
	b.eq	.LBB1_10
.LBB1_9:
	mov	x0, x19
	bl	bkops_should_query
	cbz	w0, .LBB1_23
.LBB1_10:
	ldr	x8, [x19, #128]
	tbnz	w8, #5, .LBB1_22
.LBB1_11:
	add	x1, x19, #128           // =128
	mov	w0, #5
	bl	set_bit
	adrp	x8, bkops_workqueue_g
	ldr	w0, [x19, #136]
	ldr	x20, [x8, :lo12:bkops_workqueue_g]
	add	x19, x19, #32           // =32
	bl	__msecs_to_jiffies
	mov	x3, x0
	orr	w0, wzr, #0x8
	mov	x1, x20
	mov	x2, x19
	bl	queue_delayed_work_on
	orr	w0, wzr, #0x1
	b	.LBB1_23
.LBB1_12:
	ldr	x0, [x19, #24]
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB1_15
// %bb.13:
	ldr	w8, [x19]
	cmp	w8, #2                  // =2
	b.ne	.LBB1_15
// %bb.14:
	add	x1, x19, #128           // =128
	orr	w0, wzr, #0x6
	bl	clear_bit
.LBB1_15:
	add	x20, x19, #128          // =128
	mov	w0, #5
	mov	x1, x20
	bl	test_and_clear_bit
	cbz	w0, .LBB1_23
// %bb.16:
	add	x0, x19, #32            // =32
	bl	cancel_delayed_work_sync
	ldr	x8, [x19, #128]
	tbz	w8, #0, .LBB1_22
// %bb.17:
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_lock
	adrp	x8, bkops_enabled
	ldrb	w8, [x8, :lo12:bkops_enabled]
	tbnz	w8, #0, .LBB1_19
// %bb.18:
	ldp	x8, x0, [x19, #8]
	mov	w1, wzr
	ldr	x8, [x8]
	blr	x8
.LBB1_19:
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_unlock
	mov	w0, wzr
	mov	x1, x20
	bl	clear_bit
	b	.LBB1_22
.LBB1_20:
	adrp	x0, bkops_io_busyidle_notify_handler._rs
	adrp	x1, .L__func__.bkops_io_busyidle_notify_handler
	add	x0, x0, :lo12:bkops_io_busyidle_notify_handler._rs
	add	x1, x1, :lo12:.L__func__.bkops_io_busyidle_notify_handler
	bl	___ratelimit
	cbz	w0, .LBB1_23
// %bb.21:
	adrp	x0, .L.str.2
	adrp	x1, .L__func__.bkops_io_busyidle_notify_handler
	add	x0, x0, :lo12:.L.str.2
	add	x1, x1, :lo12:.L__func__.bkops_io_busyidle_notify_handler
	mov	w2, w20
	bl	printk
.LBB1_22:
	mov	w0, wzr
.LBB1_23:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end1:
	.size	bkops_io_busyidle_notify_handler, .Lfunc_end1-bkops_io_busyidle_notify_handler
                                        // -- End function
	.globl	mas_bkops_work_query    // -- Begin function mas_bkops_work_query
	.p2align	2
	.type	mas_bkops_work_query,@function
mas_bkops_work_query:                   // @mas_bkops_work_query
// %bb.0:
	sub	sp, sp, #64             // =64
	adrp	x8, __stack_chk_guard
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	mov	x19, x0
	mov	w0, #22
	orr	w1, wzr, #0x1
	str	x21, [sp, #16]          // 8-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	add	x29, sp, #48            // =48
	str	x8, [sp, #8]
	str	wzr, [sp, #4]
	bl	io_trace_unistore_count
	cbz	x19, .LBB2_8
// %bb.1:
	ldr	x0, [x19, #168]
	bl	mas_blk_get_lld
	cbz	x0, .LBB2_8
// %bb.2:
	ldrb	w8, [x0, #25]
	tbz	w8, #0, .LBB2_7
// %bb.3:
	ldr	x20, [x0, #288]
	cbz	x20, .LBB2_25
// %bb.4:
	ldr	w8, [x20]
	cmp	w8, #2                  // =2
	b.ne	.LBB2_7
// %bb.5:
	mov	x19, x20
	ldr	x8, [x19, #128]!
	tbnz	w8, #6, .LBB2_11
// %bb.6:
	orr	w20, wzr, #0x4
	b	.LBB2_9
.LBB2_7:
	orr	w20, wzr, #0x1
	b	.LBB2_9
.LBB2_8:
	orr	w20, wzr, #0x2
.LBB2_9:
	adrp	x9, __stack_chk_guard
	ldr	x8, [sp, #8]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB2_26
// %bb.10:
	mov	w0, w20
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldr	x21, [sp, #16]          // 8-byte Folded Reload
	add	sp, sp, #64             // =64
	ret
.LBB2_11:
	ldr	x8, [x19]
	tbnz	w8, #0, .LBB2_16
// %bb.12:
	mov	w0, #5
	mov	x1, x19
	bl	test_and_clear_bit
	cbz	w0, .LBB2_14
// %bb.13:
	add	x0, x20, #32            // =32
	bl	cancel_delayed_work_sync
.LBB2_14:
	ldr	x21, [x20, #8]
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_lock
	adrp	x8, bkops_enabled
	ldrb	w8, [x8, :lo12:bkops_enabled]
	cmp	w8, #1                  // =1
	b.ne	.LBB2_17
// %bb.15:
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_unlock
	b	.LBB2_22
.LBB2_16:
	mov	w20, #5
	b	.LBB2_9
.LBB2_17:
	ldr	x8, [x21, #8]
	ldr	x0, [x20, #16]
	add	x1, sp, #4              // =4
	blr	x8
	adrp	x8, user_manual_gc
	ldrb	w9, [x8, :lo12:user_manual_gc]
	mov	w20, w0
	cbz	w9, .LBB2_20
// %bb.18:
	ldr	w9, [sp, #4]
	cbnz	w9, .LBB2_20
// %bb.19:
	strb	wzr, [x8, :lo12:user_manual_gc]
.LBB2_20:
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_unlock
	cbz	w20, .LBB2_22
// %bb.21:
	adrp	x0, .L.str.3
	adrp	x1, .L__func__.mas_bkops_work_query
	add	x0, x0, :lo12:.L.str.3
	add	x1, x1, :lo12:.L__func__.mas_bkops_work_query
	mov	w2, w20
	bl	printk
	orr	w20, wzr, #0x6
	b	.LBB2_9
.LBB2_22:
	ldr	w8, [sp, #4]
	cbz	w8, .LBB2_24
// %bb.23:
	orr	w20, wzr, #0x8
	b	.LBB2_9
.LBB2_24:
	orr	w0, wzr, #0x7
	mov	x1, x19
	orr	w20, wzr, #0x7
	bl	clear_bit
	b	.LBB2_9
.LBB2_25:
	orr	w20, wzr, #0x3
	b	.LBB2_9
.LBB2_26:
	bl	__stack_chk_fail
.Lfunc_end2:
	.size	mas_bkops_work_query, .Lfunc_end2-mas_bkops_work_query
                                        // -- End function
	.globl	mas_bkops_work_start    // -- Begin function mas_bkops_work_start
	.p2align	2
	.type	mas_bkops_work_start,@function
mas_bkops_work_start:                   // @mas_bkops_work_start
// %bb.0:
	sub	sp, sp, #64             // =64
	adrp	x8, __stack_chk_guard
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	mov	x19, x0
	mov	w0, #23
	orr	w1, wzr, #0x1
	stp	x22, x21, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	add	x29, sp, #48            // =48
	str	x8, [sp, #8]
	str	wzr, [sp, #4]
	bl	io_trace_unistore_count
	cbz	x19, .LBB3_8
// %bb.1:
	ldr	x0, [x19, #168]
	bl	mas_blk_get_lld
	cbz	x0, .LBB3_8
// %bb.2:
	ldrb	w8, [x0, #25]
	tbz	w8, #0, .LBB3_7
// %bb.3:
	ldr	x21, [x0, #288]
	cbz	x21, .LBB3_29
// %bb.4:
	ldr	w8, [x21]
	cmp	w8, #2                  // =2
	b.ne	.LBB3_7
// %bb.5:
	mov	x19, x21
	ldr	x8, [x19, #128]!
	tbnz	w8, #6, .LBB3_11
// %bb.6:
	orr	w20, wzr, #0x4
	b	.LBB3_9
.LBB3_7:
	orr	w20, wzr, #0x1
	b	.LBB3_9
.LBB3_8:
	orr	w20, wzr, #0x2
.LBB3_9:
	adrp	x9, __stack_chk_guard
	ldr	x8, [sp, #8]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB3_30
// %bb.10:
	mov	w0, w20
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]     // 16-byte Folded Reload
	add	sp, sp, #64             // =64
	ret
.LBB3_11:
	ldr	x8, [x19]
	tbnz	w8, #0, .LBB3_16
// %bb.12:
	mov	w0, #5
	mov	x1, x19
	bl	test_and_clear_bit
	cbz	w0, .LBB3_14
// %bb.13:
	add	x0, x21, #32            // =32
	bl	cancel_delayed_work_sync
.LBB3_14:
	ldr	x20, [x21, #8]
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_lock
	adrp	x22, bkops_enabled
	ldrb	w8, [x22, :lo12:bkops_enabled]
	cmp	w8, #1                  // =1
	b.ne	.LBB3_17
// %bb.15:
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_unlock
	b	.LBB3_22
.LBB3_16:
	mov	w20, #5
	b	.LBB3_9
.LBB3_17:
	ldr	x8, [x20, #8]
	ldr	x0, [x21, #16]
	add	x1, sp, #4              // =4
	blr	x8
	adrp	x8, user_manual_gc
	ldrb	w9, [x8, :lo12:user_manual_gc]
	mov	w20, w0
	cbz	w9, .LBB3_20
// %bb.18:
	ldr	w9, [sp, #4]
	cbnz	w9, .LBB3_20
// %bb.19:
	strb	wzr, [x8, :lo12:user_manual_gc]
.LBB3_20:
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_unlock
	cbz	w20, .LBB3_22
// %bb.21:
	adrp	x0, .L.str.3
	adrp	x1, .L__func__.mas_bkops_work_start
	add	x0, x0, :lo12:.L.str.3
	add	x1, x1, :lo12:.L__func__.mas_bkops_work_start
	mov	w2, w20
	bl	printk
	orr	w20, wzr, #0x6
	b	.LBB3_9
.LBB3_22:
	ldr	w8, [sp, #4]
	cbz	w8, .LBB3_25
// %bb.23:
	ldr	x20, [x21, #8]
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_lock
	ldrb	w8, [x22, :lo12:bkops_enabled]
	cmp	w8, #1                  // =1
	b.ne	.LBB3_26
// %bb.24:
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_unlock
	b	.LBB3_28
.LBB3_25:
	orr	w0, wzr, #0x7
	mov	x1, x19
	orr	w20, wzr, #0x7
	bl	clear_bit
	b	.LBB3_9
.LBB3_26:
	ldr	x8, [x20]
	ldr	x0, [x21, #16]
	orr	w1, wzr, #0x1
	blr	x8
	mov	w20, w0
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_unlock
	cbz	w20, .LBB3_28
// %bb.27:
	adrp	x0, .L.str.14
	adrp	x1, .L__func__.mas_bkops_work_start_ret
	add	x0, x0, :lo12:.L.str.14
	add	x1, x1, :lo12:.L__func__.mas_bkops_work_start_ret
	mov	w2, w20
	bl	printk
	mov	w20, #9
	b	.LBB3_9
.LBB3_28:
	orr	w0, wzr, #0x7
	mov	x1, x19
	bl	set_bit
	mov	w0, wzr
	mov	x1, x19
	bl	set_bit
	mov	w20, wzr
	b	.LBB3_9
.LBB3_29:
	orr	w20, wzr, #0x3
	b	.LBB3_9
.LBB3_30:
	bl	__stack_chk_fail
.Lfunc_end3:
	.size	mas_bkops_work_start, .Lfunc_end3-mas_bkops_work_start
                                        // -- End function
	.globl	mas_bkops_work_stop     // -- Begin function mas_bkops_work_stop
	.p2align	2
	.type	mas_bkops_work_stop,@function
mas_bkops_work_stop:                    // @mas_bkops_work_stop
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	mov	x19, x0
	orr	w0, wzr, #0x18
	orr	w1, wzr, #0x1
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	bl	io_trace_unistore_count
	cbz	x19, .LBB4_12
// %bb.1:
	ldr	x0, [x19, #168]
	bl	mas_blk_get_lld
	cbz	x0, .LBB4_12
// %bb.2:
	ldr	x20, [x0, #288]
	cbz	x20, .LBB4_12
// %bb.3:
	ldr	x8, [x0, #24]
	tbz	w8, #8, .LBB4_12
// %bb.4:
	ldr	w8, [x20]
	cmp	w8, #2                  // =2
	b.ne	.LBB4_12
// %bb.5:
	add	x19, x20, #128          // =128
	orr	w0, wzr, #0x7
	mov	x1, x19
	bl	test_and_clear_bit
	cbz	w0, .LBB4_12
// %bb.6:
	mov	w0, #5
	mov	x1, x19
	bl	test_and_clear_bit
	cbz	w0, .LBB4_8
// %bb.7:
	add	x0, x20, #32            // =32
	bl	cancel_delayed_work_sync
.LBB4_8:
	ldr	x8, [x19]
	tbz	w8, #0, .LBB4_12
// %bb.9:
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_lock
	adrp	x8, bkops_enabled
	ldrb	w8, [x8, :lo12:bkops_enabled]
	tbnz	w8, #0, .LBB4_11
// %bb.10:
	ldp	x8, x0, [x20, #8]
	mov	w1, wzr
	ldr	x8, [x8]
	blr	x8
.LBB4_11:
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_unlock
	mov	w0, wzr
	mov	x1, x19
	bl	clear_bit
.LBB4_12:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end4:
	.size	mas_bkops_work_stop, .Lfunc_end4-mas_bkops_work_stop
                                        // -- End function
	.globl	bkops_pm_callback       // -- Begin function bkops_pm_callback
	.p2align	2
	.type	bkops_pm_callback,@function
bkops_pm_callback:                      // @bkops_pm_callback
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	sub	x8, x1, #1              // =1
	cmp	x8, #3                  // =3
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	b.hi	.LBB5_4
// %bb.1:
	adrp	x9, .LJTI5_0
	add	x9, x9, :lo12:.LJTI5_0
	adr	x10, .LBB5_2
	ldrb	w11, [x9, x8]
	add	x10, x10, x11, lsl #2
	br	x10
.LBB5_2:
	adrp	x20, bkops_lock
	add	x20, x20, :lo12:bkops_lock
	mov	x0, x20
	bl	mutex_lock
	adrp	x8, bkops_enabled
	orr	w19, wzr, #0x1
	mov	x0, x20
	strb	w19, [x8, :lo12:bkops_enabled]
	bl	mutex_unlock
	b	.LBB5_5
.LBB5_3:
	adrp	x19, bkops_lock
	add	x19, x19, :lo12:bkops_lock
	mov	x0, x19
	bl	mutex_lock
	adrp	x8, bkops_enabled
	mov	x0, x19
	strb	wzr, [x8, :lo12:bkops_enabled]
	bl	mutex_unlock
	orr	w19, wzr, #0x1
	b	.LBB5_5
.LBB5_4:
	mov	w19, wzr
.LBB5_5:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	mov	w0, w19
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end5:
	.size	bkops_pm_callback, .Lfunc_end5-bkops_pm_callback
	.section	.rodata,"a",@progbits
.LJTI5_0:
	.byte	(.LBB5_2-.LBB5_2)>>2
	.byte	(.LBB5_3-.LBB5_2)>>2
	.byte	(.LBB5_2-.LBB5_2)>>2
	.byte	(.LBB5_3-.LBB5_2)>>2
                                        // -- End function
	.text
	.globl	bkops_notify_reboot     // -- Begin function bkops_notify_reboot
	.p2align	2
	.type	bkops_notify_reboot,@function
bkops_notify_reboot:                    // @bkops_notify_reboot
// %bb.0:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	adrp	x19, bkops_lock
	add	x19, x19, :lo12:bkops_lock
	mov	x0, x19
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	bl	mutex_lock
	adrp	x8, bkops_enabled
	orr	w9, wzr, #0x1
	mov	x0, x19
	strb	w9, [x8, :lo12:bkops_enabled]
	bl	mutex_unlock
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	mov	w0, wzr
	ldr	x19, [sp], #32          // 8-byte Folded Reload
	ret
.Lfunc_end6:
	.size	bkops_notify_reboot, .Lfunc_end6-bkops_notify_reboot
                                        // -- End function
	.globl	mas_bkops_alloc         // -- Begin function mas_bkops_alloc
	.p2align	2
	.type	mas_bkops_alloc,@function
mas_bkops_alloc:                        // @mas_bkops_alloc
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	adrp	x8, kmalloc_caches+64
	ldr	x0, [x8, :lo12:kmalloc_caches+64]
	mov	w1, #32960
	movk	w1, #320, lsl #16
	orr	w2, wzr, #0x100
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	bl	kmem_cache_alloc_trace
	mov	x19, x0
	cbz	x0, .LBB7_2
// %bb.1:
	adrp	x10, __ufs_bkops_idle_work_func
	mov	x8, #68719476704
	mov	x20, x19
	add	x9, x19, #40            // =40
	add	x10, x10, :lo12:__ufs_bkops_idle_work_func
	add	x0, x19, #64            // =64
	orr	w1, wzr, #0x200000
	mov	x2, xzr
	mov	x3, xzr
	str	x8, [x20, #32]!
	str	x9, [x19, #40]
	stp	x9, x10, [x19, #48]
	bl	init_timer_key
	adrp	x8, delayed_work_timer_fn
	add	x8, x8, :lo12:delayed_work_timer_fn
	mov	w9, #1000
	mov	w10, #3600
	orr	w11, wzr, #0x20000000
	stp	x8, x20, [x19, #88]
	str	x9, [x19, #136]
	str	x10, [x19, #208]
	str	x11, [x19, #224]
	str	x11, [x19, #240]
	b	.LBB7_3
.LBB7_2:
	adrp	x0, .L.str.4
	add	x0, x0, :lo12:.L.str.4
	bl	printk
.LBB7_3:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	mov	x0, x19
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end7:
	.size	mas_bkops_alloc, .Lfunc_end7-mas_bkops_alloc
                                        // -- End function
	.globl	mas_bkops_manual_gc_proc_show // -- Begin function mas_bkops_manual_gc_proc_show
	.p2align	2
	.type	mas_bkops_manual_gc_proc_show,@function
mas_bkops_manual_gc_proc_show:          // @mas_bkops_manual_gc_proc_show
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	adrp	x8, user_manual_gc
	ldrb	w2, [x8, :lo12:user_manual_gc]
	adrp	x1, .L.str.5
	add	x1, x1, :lo12:.L.str.5
	mov	x29, sp
	bl	seq_printf
	mov	w0, wzr
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end8:
	.size	mas_bkops_manual_gc_proc_show, .Lfunc_end8-mas_bkops_manual_gc_proc_show
                                        // -- End function
	.globl	mas_bkops_manual_gc_proc_open // -- Begin function mas_bkops_manual_gc_proc_open
	.p2align	2
	.type	mas_bkops_manual_gc_proc_open,@function
mas_bkops_manual_gc_proc_open:          // @mas_bkops_manual_gc_proc_open
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	mov	x0, x1
	adrp	x1, __cfi_mas_bkops_manual_gc_proc_show
	add	x1, x1, :lo12:__cfi_mas_bkops_manual_gc_proc_show
	mov	x2, xzr
	mov	x29, sp
	bl	single_open
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end9:
	.size	mas_bkops_manual_gc_proc_open, .Lfunc_end9-mas_bkops_manual_gc_proc_open
                                        // -- End function
	.globl	mas_bkops_manual_gc_proc_write // -- Begin function mas_bkops_manual_gc_proc_write
	.p2align	2
	.type	mas_bkops_manual_gc_proc_write,@function
mas_bkops_manual_gc_proc_write:         // @mas_bkops_manual_gc_proc_write
// %bb.0:
	sub	sp, sp, #48             // =48
	adrp	x8, __stack_chk_guard
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	mov	x0, #-22
	str	x19, [sp, #16]          // 8-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	str	x8, [sp, #8]
	cbz	x1, .LBB10_7
// %bb.1:
	mov	x19, x2
	cbz	x2, .LBB10_7
// %bb.2:
	//APP
	mrs x8, sp_el0
	//NO_APP
	ldr	x9, [x8, #8]
	mov	x10, x1
	//APP
		adds	x10, x10, #1
	csel	x9, xzr, x9, hi
	csinv	x10, x10, xzr, cc
	sbcs	xzr, x10, x9
	cset	x10, ls

	//NO_APP
	cbz	x10, .LBB10_9
// %bb.3:
	ldr	x9, [x8, #8]
	//APP
		bics	xzr, x1, x9
	csel	x8, x1, xzr, eq

	//NO_APP
	//APP
	hint #20
	//NO_APP
	add	x0, sp, #4              // =4
	orr	w2, wzr, #0x1
	mov	x1, x8
	bl	__arch_copy_from_user
	cbnz	x0, .LBB10_10
// %bb.4:
	ldrb	w8, [sp, #4]
	sub	w8, w8, #48             // =48
	cmp	w8, #2                  // =2
	b.hs	.LBB10_6
// %bb.5:
	adrp	x9, user_manual_gc
	mov	x0, x19
	strb	w8, [x9, :lo12:user_manual_gc]
	b	.LBB10_7
.LBB10_6:
	adrp	x0, .L.str.6
	adrp	x1, .L__func__.mas_bkops_manual_gc_proc_write
	add	x0, x0, :lo12:.L.str.6
	add	x1, x1, :lo12:.L__func__.mas_bkops_manual_gc_proc_write
	mov	w2, #738
	bl	printk
	mov	x0, #-22
.LBB10_7:
	adrp	x9, __stack_chk_guard
	ldr	x8, [sp, #8]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB10_12
// %bb.8:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldr	x19, [sp, #16]          // 8-byte Folded Reload
	add	sp, sp, #48             // =48
	ret
.LBB10_9:
	orr	w2, wzr, #0x1
	b	.LBB10_11
.LBB10_10:
	mov	x2, x0
.LBB10_11:
	orr	w8, wzr, #0x1
	sub	x8, x8, x2
	add	x9, sp, #4              // =4
	add	x0, x9, x8
	mov	w1, wzr
	bl	memset
	mov	x0, #-14
	b	.LBB10_7
.LBB10_12:
	bl	__stack_chk_fail
.Lfunc_end10:
	.size	mas_bkops_manual_gc_proc_write, .Lfunc_end10-mas_bkops_manual_gc_proc_write
                                        // -- End function
	.globl	mas_bkops_status_proc_show // -- Begin function mas_bkops_status_proc_show
	.p2align	2
	.type	mas_bkops_status_proc_show,@function
mas_bkops_status_proc_show:             // @mas_bkops_status_proc_show
// %bb.0:
	sub	sp, sp, #64             // =64
	adrp	x8, __stack_chk_guard
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	str	x21, [sp, #16]          // 8-byte Folded Spill
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	str	x8, [sp, #8]
	str	wzr, [sp, #4]
	ldr	x20, [x0, #152]
	mov	x19, x0
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	ldr	x21, [x20, #8]
	add	x29, sp, #48            // =48
	bl	mutex_lock
	adrp	x8, bkops_enabled
	ldrb	w8, [x8, :lo12:bkops_enabled]
	cmp	w8, #1                  // =1
	b.ne	.LBB11_2
// %bb.1:
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_unlock
	b	.LBB11_7
.LBB11_2:
	ldr	x8, [x21, #8]
	ldr	x0, [x20, #16]
	add	x1, sp, #4              // =4
	blr	x8
	adrp	x8, user_manual_gc
	ldrb	w9, [x8, :lo12:user_manual_gc]
	mov	w20, w0
	cbz	w9, .LBB11_5
// %bb.3:
	ldr	w9, [sp, #4]
	cbnz	w9, .LBB11_5
// %bb.4:
	strb	wzr, [x8, :lo12:user_manual_gc]
.LBB11_5:
	adrp	x0, bkops_lock
	add	x0, x0, :lo12:bkops_lock
	bl	mutex_unlock
	cbz	w20, .LBB11_7
// %bb.6:
	adrp	x0, .L.str.3
	adrp	x1, .L__func__.mas_bkops_status_proc_show
	add	x0, x0, :lo12:.L.str.3
	add	x1, x1, :lo12:.L__func__.mas_bkops_status_proc_show
	mov	w2, w20
	bl	printk
	b	.LBB11_8
.LBB11_7:
	ldr	w2, [sp, #4]
	adrp	x1, .L.str.7
	add	x1, x1, :lo12:.L.str.7
	mov	x0, x19
	bl	seq_printf
	mov	w20, wzr
.LBB11_8:
	adrp	x9, __stack_chk_guard
	ldr	x8, [sp, #8]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB11_10
// %bb.9:
	mov	w0, w20
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldr	x21, [sp, #16]          // 8-byte Folded Reload
	add	sp, sp, #64             // =64
	ret
.LBB11_10:
	bl	__stack_chk_fail
.Lfunc_end11:
	.size	mas_bkops_status_proc_show, .Lfunc_end11-mas_bkops_status_proc_show
                                        // -- End function
	.globl	mas_bkops_status_proc_open // -- Begin function mas_bkops_status_proc_open
	.p2align	2
	.type	mas_bkops_status_proc_open,@function
mas_bkops_status_proc_open:             // @mas_bkops_status_proc_open
// %bb.0:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	mov	x19, x1
	bl	PDE_DATA
	adrp	x1, __cfi_mas_bkops_status_proc_show
	mov	x2, x0
	add	x1, x1, :lo12:__cfi_mas_bkops_status_proc_show
	mov	x0, x19
	bl	single_open
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldr	x19, [sp], #32          // 8-byte Folded Reload
	ret
.Lfunc_end12:
	.size	mas_bkops_status_proc_open, .Lfunc_end12-mas_bkops_status_proc_open
                                        // -- End function
	.globl	mas_bkops_enable        // -- Begin function mas_bkops_enable
	.p2align	2
	.type	mas_bkops_enable,@function
mas_bkops_enable:                       // @mas_bkops_enable
// %bb.0:
	stp	x24, x23, [sp, #-64]!   // 16-byte Folded Spill
	stp	x22, x21, [sp, #16]     // 16-byte Folded Spill
	mov	x21, x0
	mov	w0, #-22
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	add	x29, sp, #48            // =48
	cbz	x21, .LBB13_20
// %bb.1:
	mov	x19, x1
	cbz	x1, .LBB13_20
// %bb.2:
	mov	x22, x2
	cbz	x2, .LBB13_20
// %bb.3:
	mov	x0, x21
	bl	mas_blk_get_lld
	ldrb	w8, [x0, #24]
	tbz	w8, #3, .LBB13_22
// %bb.4:
	ldr	w8, [x19]
	sub	w8, w8, #1              // =1
	cmp	w8, #3                  // =3
	b.lo	.LBB13_6
// %bb.5:
	adrp	x0, .L.str.9
	add	x0, x0, :lo12:.L.str.9
	bl	printk
	b	.LBB13_19
.LBB13_6:
	ldr	x8, [x19, #8]
	cbz	x8, .LBB13_15
// %bb.7:
	adrp	x24, mas_bkops_inited
	ldrb	w8, [x24, :lo12:mas_bkops_inited]
	mov	x20, x0
	tbnz	w8, #0, .LBB13_10
// %bb.8:
	adrp	x0, .L.str.16
	add	x0, x0, :lo12:.L.str.16
	orr	w1, wzr, #0x18
	mov	w2, wzr
	mov	x3, xzr
	mov	x4, xzr
	bl	__alloc_workqueue_key
	adrp	x8, bkops_workqueue_g
	str	x0, [x8, :lo12:bkops_workqueue_g]
	cbz	x0, .LBB13_18
// %bb.9:
	adrp	x0, bkops_pm_notifier
	add	x0, x0, :lo12:bkops_pm_notifier
	bl	register_pm_notifier
	adrp	x0, bkops_reboot_notifier
	add	x0, x0, :lo12:bkops_reboot_notifier
	bl	register_reboot_notifier
	orr	w8, wzr, #0x1
	strb	w8, [x24, :lo12:mas_bkops_inited]
.LBB13_10:
	mov	x0, x19
	mov	x1, x22
	add	x23, x19, #144          // =144
	bl	mas_bkops_add_debugfs
	cbz	w0, .LBB13_12
// %bb.11:
	adrp	x0, .L.str.12
	add	x0, x0, :lo12:.L.str.12
	bl	printk
.LBB13_12:
	adrp	x1, .L.str.13
	add	x1, x1, :lo12:.L.str.13
	orr	w2, wzr, #0x20
	mov	x0, x23
	str	x21, [x19, #24]
	bl	strncpy
	adrp	x8, __cfi_bkops_io_busyidle_notify_handler
	add	x8, x8, :lo12:__cfi_bkops_io_busyidle_notify_handler
	mov	x0, x20
	mov	x1, x23
	strb	wzr, [x19, #175]
	stp	x8, x19, [x19, #176]
	bl	blk_lld_busyidle_event_subscribe
	cbz	w0, .LBB13_16
// %bb.13:
	adrp	x19, bkops_workqueue_g
	ldr	x8, [x19, :lo12:bkops_workqueue_g]
	cbz	x8, .LBB13_19
// %bb.14:
	adrp	x0, bkops_reboot_notifier
	add	x0, x0, :lo12:bkops_reboot_notifier
	strb	wzr, [x24, :lo12:mas_bkops_inited]
	bl	unregister_reboot_notifier
	adrp	x0, bkops_pm_notifier
	add	x0, x0, :lo12:bkops_pm_notifier
	bl	unregister_pm_notifier
	ldr	x0, [x19, :lo12:bkops_workqueue_g]
	bl	destroy_workqueue
	str	xzr, [x19, :lo12:bkops_workqueue_g]
	b	.LBB13_19
.LBB13_15:
	adrp	x0, .L.str.10
	add	x0, x0, :lo12:.L.str.10
	bl	printk
	b	.LBB13_19
.LBB13_16:
	adrp	x0, .L.str.17
	add	x0, x0, :lo12:.L.str.17
	mov	x1, xzr
	bl	proc_mkdir
	cbz	x0, .LBB13_21
// %bb.17:
	mov	x20, x0
	adrp	x0, .L.str.19
	adrp	x3, mas_bkops_manual_gc_fops
	add	x0, x0, :lo12:.L.str.19
	add	x3, x3, :lo12:mas_bkops_manual_gc_fops
	mov	w1, #432
	mov	x2, x20
	bl	proc_create
	adrp	x0, .L.str.20
	adrp	x3, mas_bkops_status_fops
	add	x0, x0, :lo12:.L.str.20
	add	x3, x3, :lo12:mas_bkops_status_fops
	mov	w1, #288
	mov	x2, x20
	mov	x4, x19
	bl	proc_create_data
	mov	w0, wzr
	b	.LBB13_20
.LBB13_18:
	adrp	x0, .L.str.11
	adrp	x1, .L__func__.mas_bkops_enable
	add	x0, x0, :lo12:.L.str.11
	add	x1, x1, :lo12:.L__func__.mas_bkops_enable
	mov	w2, #829
	bl	printk
.LBB13_19:
	mov	w0, #-22
.LBB13_20:
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]     // 16-byte Folded Reload
	ldp	x24, x23, [sp], #64     // 16-byte Folded Reload
	ret
.LBB13_21:
	adrp	x0, .L.str.18
	adrp	x1, .L__func__.mas_bkops_manual_gc_proc_init
	add	x0, x0, :lo12:.L.str.18
	add	x1, x1, :lo12:.L__func__.mas_bkops_manual_gc_proc_init
	mov	w2, #791
	bl	printk
	mov	w0, wzr
	b	.LBB13_20
.LBB13_22:
	adrp	x0, .L.str.8
	add	x0, x0, :lo12:.L.str.8
	bl	printk
	b	.LBB13_19
.Lfunc_end13:
	.size	mas_bkops_enable, .Lfunc_end13-mas_bkops_enable
                                        // -- End function
	.p2align	2               // -- Begin function bkops_should_query
	.type	bkops_should_query,@function
bkops_should_query:                     // @bkops_should_query
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	ldr	x8, [x0, #128]
	mov	x19, x0
	add	x29, sp, #16            // =16
	tbnz	w8, #1, .LBB14_15
// %bb.1:
	ldr	w9, [x19, #204]
	cbz	w9, .LBB14_3
// %bb.2:
	ldr	w9, [x19, #200]
	cbnz	w9, .LBB14_15
.LBB14_3:
	ldr	x0, [x19, #24]
	bl	mas_blk_get_lld
	ldr	x8, [x19, #128]
	mov	x20, x0
	tbz	w8, #4, .LBB14_7
// %bb.4:
	orr	w0, wzr, #0x1
	bl	ktime_get_with_offset
	bl	ns_to_timespec
	ldr	x8, [x19, #216]
	cbz	x8, .LBB14_17
// %bb.5:
	ldr	x9, [x19, #208]
	sub	x8, x0, x8
	cmp	x8, x9
	b.ge	.LBB14_17
// %bb.6:
	ldr	x8, [x19, #128]
.LBB14_7:
	tbz	w8, #3, .LBB14_9
// %bb.8:
	ldr	x9, [x20, #1584]
	ldp	x11, x10, [x19, #224]
	sub	x10, x9, x10
	cmp	x10, x11
	b.hs	.LBB14_13
.LBB14_9:
	tbz	w8, #2, .LBB14_11
// %bb.10:
	ldr	x9, [x20, #1576]
	ldp	x11, x10, [x19, #240]
	sub	x10, x9, x10
	cmp	x10, x11
	b.hs	.LBB14_14
.LBB14_11:
	adrp	x9, user_manual_gc
	ldrb	w9, [x9, :lo12:user_manual_gc]
	cbnz	w9, .LBB14_15
// %bb.12:
	mov	w0, wzr
	and	x8, x8, #0xfffffffffffffffd
	b	.LBB14_16
.LBB14_13:
	str	x9, [x19, #232]
	b	.LBB14_15
.LBB14_14:
	str	x9, [x19, #248]
.LBB14_15:
	orr	x8, x8, #0x2
	orr	w0, wzr, #0x1
.LBB14_16:
	str	x8, [x19, #128]
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.LBB14_17:
	ldr	x8, [x19, #128]
	str	x0, [x19, #216]
	b	.LBB14_15
.Lfunc_end14:
	.size	bkops_should_query, .Lfunc_end14-bkops_should_query
                                        // -- End function
	.type	.L.str,@object          // @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	"\0013[BKOPS]mas_bkops_query failed!\n"
	.size	.L.str, 34

	.type	.L.str.1,@object        // @.str.1
.L.str.1:
	.asciz	"\0013[BKOPS]mas_bkops_start failed!\n"
	.size	.L.str.1, 34

	.type	bkops_io_busyidle_notify_handler._rs,@object // @bkops_io_busyidle_notify_handler._rs
	.data
	.p2align	3
bkops_io_busyidle_notify_handler._rs:
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
	.size	bkops_io_busyidle_notify_handler._rs, 56

	.type	.L__func__.bkops_io_busyidle_notify_handler,@object // @__func__.bkops_io_busyidle_notify_handler
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__func__.bkops_io_busyidle_notify_handler:
	.asciz	"bkops_io_busyidle_notify_handler"
	.size	.L__func__.bkops_io_busyidle_notify_handler, 33

	.type	.L.str.2,@object        // @.str.2
.L.str.2:
	.asciz	"\0013[BKOPS]%s unknown state: %d\n"
	.size	.L.str.2, 31

	.type	.L.str.3,@object        // @.str.3
.L.str.3:
	.asciz	"\0013[BKOPS]%s: mas_bkops_query failed, ret = %d!\n"
	.size	.L.str.3, 48

	.type	.L__func__.mas_bkops_work_query,@object // @__func__.mas_bkops_work_query
.L__func__.mas_bkops_work_query:
	.asciz	"mas_bkops_work_query"
	.size	.L__func__.mas_bkops_work_query, 21

	.type	.L__func__.mas_bkops_work_start,@object // @__func__.mas_bkops_work_start
.L__func__.mas_bkops_work_start:
	.asciz	"mas_bkops_work_start"
	.size	.L__func__.mas_bkops_work_start, 21

	.type	.L.str.4,@object        // @.str.4
.L.str.4:
	.asciz	"\0013[BKOPS]Failed to alloc bkops_func\n"
	.size	.L.str.4, 37

	.type	.L.str.5,@object        // @.str.5
.L.str.5:
	.asciz	"%d\n"
	.size	.L.str.5, 4

	.type	user_manual_gc,@object  // @user_manual_gc
	.local	user_manual_gc
	.comm	user_manual_gc,1,4
	.type	.L.str.6,@object        // @.str.6
.L.str.6:
	.asciz	"\0013[BKOPS]%s %d: input error.\n"
	.size	.L.str.6, 30

	.type	.L__func__.mas_bkops_manual_gc_proc_write,@object // @__func__.mas_bkops_manual_gc_proc_write
.L__func__.mas_bkops_manual_gc_proc_write:
	.asciz	"mas_bkops_manual_gc_proc_write"
	.size	.L__func__.mas_bkops_manual_gc_proc_write, 31

	.type	.L__func__.mas_bkops_status_proc_show,@object // @__func__.mas_bkops_status_proc_show
.L__func__.mas_bkops_status_proc_show:
	.asciz	"mas_bkops_status_proc_show"
	.size	.L__func__.mas_bkops_status_proc_show, 27

	.type	.L.str.7,@object        // @.str.7
.L.str.7:
	.asciz	"%u\n"
	.size	.L.str.7, 4

	.type	.L.str.8,@object        // @.str.8
.L.str.8:
	.asciz	"\0013[BKOPS]Busy Idle was not enabled!\n"
	.size	.L.str.8, 37

	.type	.L.str.9,@object        // @.str.9
.L.str.9:
	.asciz	"\0013[BKOPS]bkops dev type not specified!\n"
	.size	.L.str.9, 40

	.type	.L.str.10,@object       // @.str.10
.L.str.10:
	.asciz	"\0013[BKOPS]bkops_ops is NULL!\n"
	.size	.L.str.10, 29

	.type	.L.str.11,@object       // @.str.11
.L.str.11:
	.asciz	"\0013[BKOPS]%s %d mas_bkops_init failed!\n"
	.size	.L.str.11, 39

	.type	.L__func__.mas_bkops_enable,@object // @__func__.mas_bkops_enable
.L__func__.mas_bkops_enable:
	.asciz	"mas_bkops_enable"
	.size	.L__func__.mas_bkops_enable, 17

	.type	.L.str.12,@object       // @.str.12
.L.str.12:
	.asciz	"\0013[BKOPS]Failed to create mmc bkops debugfs!\n"
	.size	.L.str.12, 46

	.type	.L.str.13,@object       // @.str.13
.L.str.13:
	.asciz	"mas_bkops"
	.size	.L.str.13, 10

	.type	bkops_lock,@object      // @bkops_lock
	.data
	.p2align	3
bkops_lock:
	.zero	8
	.zero	4
	.word	3735899821              // 0xdead4ead
	.word	4294967295              // 0xffffffff
	.zero	4
	.xword	-1
	.zero	4
	.zero	4
	.xword	bkops_lock+40
	.xword	bkops_lock+40
	.xword	0
	.size	bkops_lock, 64

	.type	bkops_enabled,@object   // @bkops_enabled
	.local	bkops_enabled
	.comm	bkops_enabled,1,1
	.type	bkops_workqueue_g,@object // @bkops_workqueue_g
	.local	bkops_workqueue_g
	.comm	bkops_workqueue_g,8,8
	.type	.L.str.14,@object       // @.str.14
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.14:
	.asciz	"\0013[BKOPS]%s: mas_bkops_start failed, ret = %d!\n"
	.size	.L.str.14, 48

	.type	.L__func__.mas_bkops_work_start_ret,@object // @__func__.mas_bkops_work_start_ret
.L__func__.mas_bkops_work_start_ret:
	.asciz	"mas_bkops_work_start_ret"
	.size	.L__func__.mas_bkops_work_start_ret, 25

	.type	mas_bkops_inited,@object // @mas_bkops_inited
	.local	mas_bkops_inited
	.comm	mas_bkops_inited,1,4
	.type	.L.str.16,@object       // @.str.16
.L.str.16:
	.asciz	"bkops_workqueue"
	.size	.L.str.16, 16

	.type	bkops_pm_notifier,@object // @bkops_pm_notifier
	.data
	.p2align	3
bkops_pm_notifier:
	.xword	__cfi_bkops_pm_callback
	.xword	0
	.word	0                       // 0x0
	.zero	4
	.size	bkops_pm_notifier, 24

	.type	bkops_reboot_notifier,@object // @bkops_reboot_notifier
	.p2align	3
bkops_reboot_notifier:
	.xword	__cfi_bkops_notify_reboot
	.xword	0
	.word	2147483647              // 0x7fffffff
	.zero	4
	.size	bkops_reboot_notifier, 24

	.type	.L.str.17,@object       // @.str.17
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.17:
	.asciz	"block"
	.size	.L.str.17, 6

	.type	.L.str.18,@object       // @.str.18
.L.str.18:
	.asciz	"\0013[BKOPS]%s %d: proc mk error.\n"
	.size	.L.str.18, 32

	.type	.L__func__.mas_bkops_manual_gc_proc_init,@object // @__func__.mas_bkops_manual_gc_proc_init
.L__func__.mas_bkops_manual_gc_proc_init:
	.asciz	"mas_bkops_manual_gc_proc_init"
	.size	.L__func__.mas_bkops_manual_gc_proc_init, 30

	.type	.L.str.19,@object       // @.str.19
.L.str.19:
	.asciz	"manual_gc"
	.size	.L.str.19, 10

	.type	mas_bkops_manual_gc_fops,@object // @mas_bkops_manual_gc_fops
	.section	.rodata,"a",@progbits
	.p2align	3
mas_bkops_manual_gc_fops:
	.xword	0
	.xword	seq_lseek
	.xword	seq_read
	.xword	__cfi_mas_bkops_manual_gc_proc_write
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	__cfi_mas_bkops_manual_gc_proc_open
	.xword	0
	.xword	single_release
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.size	mas_bkops_manual_gc_fops, 240

	.type	.L.str.20,@object       // @.str.20
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.20:
	.asciz	"status"
	.size	.L.str.20, 7

	.type	mas_bkops_status_fops,@object // @mas_bkops_status_fops
	.section	.rodata,"a",@progbits
	.p2align	3
mas_bkops_status_fops:
	.xword	0
	.xword	seq_lseek
	.xword	seq_read
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	__cfi_mas_bkops_status_proc_open
	.xword	0
	.xword	single_release
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.size	mas_bkops_status_fops, 240


	.ident	"Android (27847 based on r353983c) clang version 9.0.3 (https://android.googlesource.com/toolchain/clang dbdb7a2acf33dc76cd6f506c323253b8d892ae05) (https://android.googlesource.com/toolchain/llvm f32df87f6769ededcd9e2a6c3c4c45c1baf361c2) (based on LLVM 9.0.3svn)"
	.section	".note.GNU-stack","",@progbits

	.text
	.file	"mas_blk_busy_idle_notify.c"
	.globl	mas_blk_busyidle_handler_latency_check_timer_expire // -- Begin function mas_blk_busyidle_handler_latency_check_timer_expire
	.p2align	2
	.type	mas_blk_busyidle_handler_latency_check_timer_expire,@function
mas_blk_busyidle_handler_latency_check_timer_expire: // @mas_blk_busyidle_handler_latency_check_timer_expire
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	add	x2, x0, #88             // =88
	adrp	x0, .L.str
	adrp	x1, .L__func__.mas_blk_busyidle_handler_latency_check_timer_expire
	add	x0, x0, :lo12:.L.str
	add	x1, x1, :lo12:.L__func__.mas_blk_busyidle_handler_latency_check_timer_expire
	mov	w3, #1000
	mov	x29, sp
	bl	printk
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end0:
	.size	mas_blk_busyidle_handler_latency_check_timer_expire, .Lfunc_end0-mas_blk_busyidle_handler_latency_check_timer_expire
                                        // -- End function
	.globl	mas_blk_busyidle_notify_handler // -- Begin function mas_blk_busyidle_notify_handler
	.p2align	2
	.type	mas_blk_busyidle_notify_handler,@function
mas_blk_busyidle_notify_handler:        // @mas_blk_busyidle_notify_handler
// %bb.0:
	str	x23, [sp, #-64]!        // 8-byte Folded Spill
	stp	x22, x21, [sp, #16]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	ldr	x8, [x0, #56]
	add	x19, x0, #24            // =24
	add	x29, sp, #48            // =48
	cbz	x8, .LBB1_19
// %bb.1:
	mov	x22, x1
	sub	x23, x0, #64            // =64
	bl	ktime_get
	adrp	x8, jiffies
	ldr	x8, [x8, :lo12:jiffies]
	add	x21, x23, #16           // =16
	mov	x20, x0
	mov	x0, x21
	add	x1, x8, #250            // =250
	bl	mod_timer
	cmp	x22, #3                 // =3
	b.hi	.LBB1_16
// %bb.2:
	adrp	x8, .LJTI1_0
	add	x8, x8, :lo12:.LJTI1_0
	adr	x9, .LBB1_3
	ldrb	w10, [x8, x22]
	add	x9, x9, x10, lsl #2
	br	x9
.LBB1_3:
	ldr	w8, [x23, #144]
	cmp	w8, #1                  // =1
	b.ne	.LBB1_16
// %bb.4:
	ldr	x8, [x23, #120]
	mov	x0, x19
	mov	w1, wzr
	blr	x8
	str	wzr, [x23, #144]
	cmp	w0, #2                  // =2
	b.ne	.LBB1_16
	b	.LBB1_15
.LBB1_5:
	ldr	w8, [x23, #144]
	cbnz	w8, .LBB1_16
// %bb.6:
	ldr	x8, [x23, #120]
	orr	w1, wzr, #0x1
	mov	x0, x19
	orr	w22, wzr, #0x1
	blr	x8
	cmp	w0, #1                  // =1
	str	w22, [x23, #144]
	b.ne	.LBB1_14
.LBB1_7:
	mov	w22, #32769
	b	.LBB1_17
.LBB1_8:
	ldr	x8, [x23, #136]
	ldrb	w8, [x8, #25]
	tbz	w8, #0, .LBB1_16
// %bb.9:
	ldr	w8, [x23, #148]
	cmp	w8, #1                  // =1
	b.ne	.LBB1_16
// %bb.10:
	ldr	x8, [x23, #120]
	orr	w1, wzr, #0x2
	mov	x0, x19
	blr	x8
	str	wzr, [x23, #148]
	cmp	w0, #2                  // =2
	b.ne	.LBB1_16
	b	.LBB1_15
.LBB1_11:
	ldr	x8, [x23, #136]
	ldrb	w8, [x8, #25]
	tbz	w8, #0, .LBB1_16
// %bb.12:
	ldr	w8, [x23, #148]
	cbnz	w8, .LBB1_16
// %bb.13:
	ldr	x8, [x23, #120]
	orr	w1, wzr, #0x3
	mov	x0, x19
	blr	x8
	orr	w8, wzr, #0x1
	cmp	w0, #1                  // =1
	str	w8, [x23, #148]
	b.eq	.LBB1_7
.LBB1_14:
	cmp	w0, #2                  // =2
	b.ne	.LBB1_16
.LBB1_15:
	adrp	x0, .L.str.2
	adrp	x1, .L__func__.mas_blk_busyidle_notify_handler
	add	x0, x0, :lo12:.L.str.2
	add	x1, x1, :lo12:.L__func__.mas_blk_busyidle_notify_handler
	mov	x2, x19
	bl	printk
.LBB1_16:
	mov	w22, wzr
.LBB1_17:
	mov	x0, x21
	bl	del_timer_sync
	bl	ktime_get
	mov	w8, #38528
	movk	w8, #152, lsl #16
	add	x8, x20, x8
	cmp	x0, x8
	b.gt	.LBB1_20
.LBB1_18:
	mov	w0, w22
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]     // 16-byte Folded Reload
	ldr	x23, [sp], #64          // 8-byte Folded Reload
	ret
.LBB1_19:
	adrp	x0, .L.str.1
	adrp	x1, .L__func__.mas_blk_busyidle_notify_handler
	add	x0, x0, :lo12:.L.str.1
	add	x1, x1, :lo12:.L__func__.mas_blk_busyidle_notify_handler
	mov	x2, x19
	bl	printk
	mov	w22, wzr
	b	.LBB1_18
.LBB1_20:
	bl	ktime_get
	mov	x9, #13531
	movk	x9, #55222, lsl #16
	movk	x9, #56962, lsl #32
	sub	x8, x0, x20
	movk	x9, #17179, lsl #48
	smulh	x8, x8, x9
	asr	x9, x8, #18
	adrp	x0, .L.str.3
	adrp	x1, .L__func__.mas_blk_busyidle_notify_handler
	add	x3, x9, x8, lsr #63
	add	x0, x0, :lo12:.L.str.3
	add	x1, x1, :lo12:.L__func__.mas_blk_busyidle_notify_handler
	mov	x2, x19
	bl	printk
	b	.LBB1_18
.Lfunc_end1:
	.size	mas_blk_busyidle_notify_handler, .Lfunc_end1-mas_blk_busyidle_notify_handler
	.section	.rodata,"a",@progbits
.LJTI1_0:
	.byte	(.LBB1_3-.LBB1_3)>>2
	.byte	(.LBB1_5-.LBB1_3)>>2
	.byte	(.LBB1_8-.LBB1_3)>>2
	.byte	(.LBB1_11-.LBB1_3)>>2
                                        // -- End function
	.text
	.globl	blk_lld_fg_io_idle_notify // -- Begin function blk_lld_fg_io_idle_notify
	.p2align	2
	.type	blk_lld_fg_io_idle_notify,@function
blk_lld_fg_io_idle_notify:              // @blk_lld_fg_io_idle_notify
// %bb.0:
	ldr	w8, [x0]
	cmp	w8, #122                // =122
	b.ne	.LBB2_2
// %bb.1:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	adrp	x8, blk_fg_io_idle_notify_workqueue
	ldr	x19, [x8, :lo12:blk_fg_io_idle_notify_workqueue]
	ldr	w8, [x0, #1424]
	add	x20, x0, #1328          // =1328
	add	x29, sp, #16            // =16
	mov	w0, w8
	bl	__msecs_to_jiffies
	mov	x3, x0
	orr	w0, wzr, #0x8
	mov	x1, x19
	mov	x2, x20
	bl	queue_delayed_work_on
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
.LBB2_2:
	ret
.Lfunc_end2:
	.size	blk_lld_fg_io_idle_notify, .Lfunc_end2-blk_lld_fg_io_idle_notify
                                        // -- End function
	.globl	mas_blk_idle_notify_work // -- Begin function mas_blk_idle_notify_work
	.p2align	2
	.type	mas_blk_idle_notify_work,@function
mas_blk_idle_notify_work:               // @mas_blk_idle_notify_work
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	mov	x20, x0
	ldrb	w8, [x20, #-8]!
	add	x29, sp, #16            // =16
	cbz	w8, .LBB3_2
// %bb.1:
	str	wzr, [x20, #208]
.LBB3_2:
	add	x19, x20, #216          // =216
	mov	x0, x19
	bl	mutex_lock
	ldr	w8, [x20, #208]
	cbnz	w8, .LBB3_5
// %bb.3:
	ldr	w8, [x20, #280]
	cmp	w8, #1                  // =1
	b.eq	.LBB3_5
// %bb.4:
	orr	w8, wzr, #0x1
	add	x0, x20, #128           // =128
	orr	w1, wzr, #0x1
	mov	x2, xzr
	str	w8, [x20, #280]
	bl	blocking_notifier_call_chain
.LBB3_5:
	mov	x0, x19
	bl	mutex_unlock
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end3:
	.size	mas_blk_idle_notify_work, .Lfunc_end3-mas_blk_idle_notify_work
                                        // -- End function
	.globl	blk_lld_idle_notify     // -- Begin function blk_lld_idle_notify
	.p2align	2
	.type	blk_lld_idle_notify,@function
blk_lld_idle_notify:                    // @blk_lld_idle_notify
// %bb.0:
	ldr	w8, [x0]
	cmp	w8, #122                // =122
	b.ne	.LBB4_2
// %bb.1:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	adrp	x8, blk_idle_notify_workqueue
	ldr	x19, [x8, :lo12:blk_idle_notify_workqueue]
	ldr	w8, [x0, #1144]
	add	x20, x0, #1048          // =1048
	add	x29, sp, #16            // =16
	mov	w0, w8
	bl	__msecs_to_jiffies
	mov	x3, x0
	orr	w0, wzr, #0x8
	mov	x1, x19
	mov	x2, x20
	bl	queue_delayed_work_on
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
.LBB4_2:
	ret
.Lfunc_end4:
	.size	blk_lld_idle_notify, .Lfunc_end4-blk_lld_idle_notify
                                        // -- End function
	.globl	mas_blk_busyidle_check_bio // -- Begin function mas_blk_busyidle_check_bio
	.p2align	2
	.type	mas_blk_busyidle_check_bio,@function
mas_blk_busyidle_check_bio:             // @mas_blk_busyidle_check_bio
// %bb.0:
	stp	x22, x21, [sp, #-48]!   // 16-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	mov	x21, x1
	mov	x19, x0
	bl	mas_blk_get_lld
	ldrb	w8, [x21, #144]
	str	x19, [x21, #96]
	tbnz	w8, #0, .LBB5_10
// %bb.1:
	add	x22, x0, #1552          // =1552
	mov	x20, x0
	orr	w8, w8, #0x1
	mov	x0, x22
	strb	w8, [x21, #144]
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x20, #1504]
	ldr	x11, [x20, #1528]
	mov	x1, x0
	add	x9, x21, #296           // =296
	add	x10, x20, #1520         // =1520
	add	x8, x8, #1              // =1
	mov	x0, x22
	str	x9, [x20, #1528]
	str	x8, [x20, #1504]
	stp	x10, x11, [x21, #296]
	str	x9, [x11]
	bl	_raw_spin_unlock_irqrestore
	ldr	x8, [x20, #24]
	tbz	w8, #3, .LBB5_4
// %bb.2:
	ldr	w8, [x20, #1248]
	cbz	w8, .LBB5_11
.LBB5_3:
	add	x8, x20, #1248          // =1248
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	ldr	x8, [x20, #24]
.LBB5_4:
	tbz	w8, #8, .LBB5_10
// %bb.5:
	ldrb	w8, [x21, #290]
	cbz	w8, .LBB5_10
// %bb.6:
	ldrb	w8, [x21, #144]
	tbnz	w8, #5, .LBB5_10
// %bb.7:
	orr	w8, w8, #0x20
	strb	w8, [x21, #144]
	ldr	w8, [x20, #24]
	mov	w9, #264
	mvn	w8, w8
	tst	x8, x9
	b.ne	.LBB5_10
// %bb.8:
	ldr	w8, [x20, #1428]
	add	x22, x20, #1428         // =1428
	cbz	w8, .LBB5_14
.LBB5_9:
	//APP
	// atomic_add
	prfm	pstl1strm, [x22]
1:	ldxr	w8, [x22]
	add	w8, w8, #1
	stxr	w9, w8, [x22]
	cbnz	w9, 1b
	//NO_APP
.LBB5_10:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldp	x22, x21, [sp], #48     // 16-byte Folded Reload
	ret
.LBB5_11:
	add	x22, x20, #1256         // =1256
	mov	x0, x22
	bl	mutex_lock
	add	x0, x20, #1048          // =1048
	bl	cancel_delayed_work
	ldr	w8, [x20, #1320]
	cmp	w8, #1                  // =1
	b.ne	.LBB5_13
// %bb.12:
	add	x0, x20, #1168          // =1168
	mov	x1, xzr
	mov	x2, xzr
	str	wzr, [x20, #1320]
	bl	blocking_notifier_call_chain
.LBB5_13:
	mov	x0, x22
	bl	mutex_unlock
	b	.LBB5_3
.LBB5_14:
	add	x21, x20, #1432         // =1432
	mov	x0, x21
	bl	mutex_lock
	add	x0, x20, #1328          // =1328
	bl	cancel_delayed_work
	ldr	w8, [x20, #1496]
	cmp	w8, #1                  // =1
	b.ne	.LBB5_16
// %bb.15:
	add	x0, x20, #1168          // =1168
	orr	w1, wzr, #0x2
	mov	x2, x19
	str	wzr, [x20, #1496]
	bl	blocking_notifier_call_chain
.LBB5_16:
	mov	x0, x21
	bl	mutex_unlock
	b	.LBB5_9
.Lfunc_end5:
	.size	mas_blk_busyidle_check_bio, .Lfunc_end5-mas_blk_busyidle_check_bio
                                        // -- End function
	.globl	mas_blk_busyidle_check_request_bio // -- Begin function mas_blk_busyidle_check_request_bio
	.p2align	2
	.type	mas_blk_busyidle_check_request_bio,@function
mas_blk_busyidle_check_request_bio:     // @mas_blk_busyidle_check_request_bio
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	ldr	x19, [x1, #120]
	add	x29, sp, #16            // =16
	cbz	x19, .LBB6_4
// %bb.1:
	mov	x20, x0
.LBB6_2:                                // =>This Inner Loop Header: Depth=1
	mov	x0, x20
	mov	x1, x19
	bl	mas_blk_busyidle_check_bio
	ldr	x19, [x19]
	cbnz	x19, .LBB6_2
// %bb.3:
	orr	w0, wzr, #0x1
	b	.LBB6_5
.LBB6_4:
	mov	w0, wzr
.LBB6_5:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end6:
	.size	mas_blk_busyidle_check_request_bio, .Lfunc_end6-mas_blk_busyidle_check_request_bio
                                        // -- End function
	.globl	mas_blk_busyidle_end_rq // -- Begin function mas_blk_busyidle_end_rq
	.p2align	2
	.type	mas_blk_busyidle_end_rq,@function
mas_blk_busyidle_end_rq:                // @mas_blk_busyidle_end_rq
// %bb.0:
	stp	x22, x21, [sp, #-48]!   // 16-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	mov	x19, x0
	ldr	x0, [x0, #64]
	add	x29, sp, #32            // =32
	mov	w20, w1
	bl	mas_blk_get_lld
	ldr	x8, [x0, #1512]
	mov	x21, x0
	cbz	x8, .LBB7_2
// %bb.1:
	add	x22, x21, #1552         // =1552
	mov	x0, x22
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x21, #1512]
	mov	x1, x0
	add	x10, x19, #488          // =488
	mov	x0, x22
	sub	x8, x8, #1              // =1
	str	x8, [x21, #1512]
	ldp	x8, x9, [x19, #488]
	str	x9, [x8, #8]
	str	x8, [x9]
	str	x10, [x19, #488]
	str	x10, [x19, #496]
	bl	_raw_spin_unlock_irqrestore
.LBB7_2:
	ldrb	w8, [x21, #24]
	tbz	w8, #3, .LBB7_6
// %bb.3:
	ldrb	w8, [x21, #1040]
	cbnz	w8, .LBB7_6
// %bb.4:
	ldr	w8, [x21, #1248]
	cbz	w8, .LBB7_9
// %bb.5:
	add	x8, x21, #1248          // =1248
	//APP
	// atomic_sub_return
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stlxr	w10, w9, [x8]
	cbnz	w10, 1b
	dmb ish
	//NO_APP
	cbz	w9, .LBB7_10
.LBB7_6:
	ldr	x8, [x19, #296]
	cbz	x8, .LBB7_8
.LBB7_7:
	mov	x0, x19
	mov	w1, w20
	blr	x8
.LBB7_8:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldp	x22, x21, [sp], #48     // 16-byte Folded Reload
	ret
.LBB7_9:
	adrp	x0, .L.str.12
	adrp	x1, .L__func__.blk_idle_count
	add	x0, x0, :lo12:.L.str.12
	add	x1, x1, :lo12:.L__func__.blk_idle_count
	bl	printk
	ldr	x8, [x19, #296]
	cbnz	x8, .LBB7_7
	b	.LBB7_8
.LBB7_10:
	ldr	w8, [x21]
	cmp	w8, #122                // =122
	b.ne	.LBB7_6
// %bb.11:
	adrp	x8, blk_idle_notify_workqueue
	ldr	w0, [x21, #1144]
	ldr	x22, [x8, :lo12:blk_idle_notify_workqueue]
	add	x21, x21, #1048         // =1048
	bl	__msecs_to_jiffies
	mov	x3, x0
	orr	w0, wzr, #0x8
	mov	x1, x22
	mov	x2, x21
	bl	queue_delayed_work_on
	ldr	x8, [x19, #296]
	cbnz	x8, .LBB7_7
	b	.LBB7_8
.Lfunc_end7:
	.size	mas_blk_busyidle_end_rq, .Lfunc_end7-mas_blk_busyidle_end_rq
                                        // -- End function
	.globl	mas_blk_busyidle_check_execute_request // -- Begin function mas_blk_busyidle_check_execute_request
	.p2align	2
	.type	mas_blk_busyidle_check_execute_request,@function
mas_blk_busyidle_check_execute_request: // @mas_blk_busyidle_check_execute_request
// %bb.0:
	stp	x22, x21, [sp, #-48]!   // 16-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	ldr	x21, [x1, #120]
	mov	x19, x2
	mov	x20, x1
	add	x29, sp, #32            // =32
	cbz	x21, .LBB8_4
// %bb.1:
	mov	x22, x0
.LBB8_2:                                // =>This Inner Loop Header: Depth=1
	mov	x0, x22
	mov	x1, x21
	bl	mas_blk_busyidle_check_bio
	ldr	x21, [x21]
	cbnz	x21, .LBB8_2
// %bb.3:
	add	x8, x20, #616           // =616
	b	.LBB8_8
.LBB8_4:
	ldr	x0, [x20, #64]
	bl	mas_blk_get_lld
	add	x22, x0, #1552          // =1552
	mov	x21, x0
	mov	x0, x22
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x21, #1512]
	ldr	x11, [x21, #1544]
	mov	x1, x0
	add	x9, x20, #488           // =488
	add	x10, x21, #1536         // =1536
	add	x8, x8, #1              // =1
	mov	x0, x22
	str	x9, [x21, #1544]
	str	x8, [x21, #1512]
	stp	x10, x11, [x20, #488]
	str	x9, [x11]
	bl	_raw_spin_unlock_irqrestore
	ldrb	w8, [x21, #24]
	tbz	w8, #3, .LBB8_7
// %bb.5:
	ldr	w8, [x21, #1248]
	cbz	w8, .LBB8_9
.LBB8_6:
	add	x8, x21, #1248          // =1248
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
.LBB8_7:
	adrp	x8, __cfi_mas_blk_busyidle_end_rq
	add	x8, x8, :lo12:__cfi_mas_blk_busyidle_end_rq
	str	x8, [x20, #616]
	add	x8, x20, #296           // =296
.LBB8_8:
	str	x19, [x8]
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldp	x22, x21, [sp], #48     // 16-byte Folded Reload
	ret
.LBB8_9:
	add	x22, x21, #1256         // =1256
	mov	x0, x22
	bl	mutex_lock
	add	x0, x21, #1048          // =1048
	bl	cancel_delayed_work
	ldr	w8, [x21, #1320]
	cmp	w8, #1                  // =1
	b.ne	.LBB8_11
// %bb.10:
	add	x0, x21, #1168          // =1168
	mov	x1, xzr
	mov	x2, xzr
	str	wzr, [x21, #1320]
	bl	blocking_notifier_call_chain
.LBB8_11:
	mov	x0, x22
	bl	mutex_unlock
	b	.LBB8_6
.Lfunc_end8:
	.size	mas_blk_busyidle_check_execute_request, .Lfunc_end8-mas_blk_busyidle_check_execute_request
                                        // -- End function
	.globl	mas_blk_busyidle_check_bio_endio // -- Begin function mas_blk_busyidle_check_bio_endio
	.p2align	2
	.type	mas_blk_busyidle_check_bio_endio,@function
mas_blk_busyidle_check_bio_endio:       // @mas_blk_busyidle_check_bio_endio
// %bb.0:
	stp	x22, x21, [sp, #-48]!   // 16-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	ldr	x8, [x0, #8]
	add	x9, x0, #96             // =96
	mov	x20, x0
	add	x29, sp, #32            // =32
	add	x10, x8, #1120          // =1120
	cmp	x8, #0                  // =0
	csel	x8, x9, x10, eq
	ldr	x0, [x8]
	bl	mas_blk_get_lld
	ldrb	w8, [x0, #25]
	mov	x19, x0
	tbz	w8, #0, .LBB9_6
// %bb.1:
	ldrb	w8, [x20, #290]
	cbz	w8, .LBB9_6
// %bb.2:
	ldrb	w8, [x20, #144]
	tbz	w8, #5, .LBB9_6
// %bb.3:
	and	w8, w8, #0xffffffdf
	strb	w8, [x20, #144]
	ldr	w8, [x19, #24]
	mov	w9, #264
	mvn	w8, w8
	tst	x8, x9
	b.ne	.LBB9_6
// %bb.4:
	ldr	w8, [x19, #1428]
	cbz	w8, .LBB9_17
// %bb.5:
	add	x8, x19, #1428          // =1428
	//APP
	// atomic_sub_return
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stlxr	w10, w9, [x8]
	cbnz	w10, 1b
	dmb ish
	//NO_APP
	cbz	w9, .LBB9_18
.LBB9_6:
	ldrb	w8, [x20, #144]
	tbz	w8, #0, .LBB9_13
.LBB9_7:
	ldr	x8, [x19, #1504]
	cbz	x8, .LBB9_9
// %bb.8:
	add	x21, x19, #1552         // =1552
	mov	x0, x21
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x19, #1504]
	mov	x1, x0
	add	x10, x20, #296          // =296
	mov	x0, x21
	sub	x8, x8, #1              // =1
	str	x8, [x19, #1504]
	ldp	x8, x9, [x20, #296]
	str	x9, [x8, #8]
	str	x8, [x9]
	str	x10, [x20, #296]
	str	x10, [x20, #304]
	bl	_raw_spin_unlock_irqrestore
.LBB9_9:
	strb	wzr, [x20, #144]
	ldrb	w8, [x19, #24]
	tbz	w8, #3, .LBB9_13
// %bb.10:
	ldrb	w8, [x19, #1040]
	cbnz	w8, .LBB9_13
// %bb.11:
	ldr	w8, [x19, #1248]
	cbz	w8, .LBB9_14
// %bb.12:
	add	x8, x19, #1248          // =1248
	//APP
	// atomic_sub_return
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stlxr	w10, w9, [x8]
	cbnz	w10, 1b
	dmb ish
	//NO_APP
	cbz	w9, .LBB9_15
.LBB9_13:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldp	x22, x21, [sp], #48     // 16-byte Folded Reload
	ret
.LBB9_14:
	adrp	x0, .L.str.12
	adrp	x1, .L__func__.blk_idle_count
	add	x0, x0, :lo12:.L.str.12
	add	x1, x1, :lo12:.L__func__.blk_idle_count
	bl	printk
	b	.LBB9_13
.LBB9_15:
	ldr	w8, [x19]
	cmp	w8, #122                // =122
	b.ne	.LBB9_13
// %bb.16:
	adrp	x8, blk_idle_notify_workqueue
	ldr	w0, [x19, #1144]
	ldr	x20, [x8, :lo12:blk_idle_notify_workqueue]
	add	x19, x19, #1048         // =1048
	bl	__msecs_to_jiffies
	mov	x3, x0
	orr	w0, wzr, #0x8
	mov	x1, x20
	mov	x2, x19
	bl	queue_delayed_work_on
	b	.LBB9_13
.LBB9_17:
	adrp	x0, .L.str.13
	adrp	x1, .L__func__.blk_fg_io_idle_count
	add	x0, x0, :lo12:.L.str.13
	add	x1, x1, :lo12:.L__func__.blk_fg_io_idle_count
	bl	printk
	ldrb	w8, [x20, #144]
	tbnz	w8, #0, .LBB9_7
	b	.LBB9_13
.LBB9_18:
	ldr	w8, [x19]
	cmp	w8, #122                // =122
	b.ne	.LBB9_6
// %bb.19:
	adrp	x8, blk_fg_io_idle_notify_workqueue
	ldr	w0, [x19, #1424]
	ldr	x21, [x8, :lo12:blk_fg_io_idle_notify_workqueue]
	add	x22, x19, #1328         // =1328
	bl	__msecs_to_jiffies
	mov	x3, x0
	orr	w0, wzr, #0x8
	mov	x1, x21
	mov	x2, x22
	bl	queue_delayed_work_on
	ldrb	w8, [x20, #144]
	tbnz	w8, #0, .LBB9_7
	b	.LBB9_13
.Lfunc_end9:
	.size	mas_blk_busyidle_check_bio_endio, .Lfunc_end9-mas_blk_busyidle_check_bio_endio
                                        // -- End function
	.globl	blk_busyidle_event_subscribe // -- Begin function blk_busyidle_event_subscribe
	.p2align	2
	.type	blk_busyidle_event_subscribe,@function
blk_busyidle_event_subscribe:           // @blk_busyidle_event_subscribe
// %bb.0:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	mov	x8, x0
	mov	w0, #-22
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	cbz	x8, .LBB10_5
// %bb.1:
	mov	x19, x1
	cbz	x1, .LBB10_5
// %bb.2:
	ldr	x8, [x8, #160]
	ldr	x0, [x8, #1120]
	cbz	x0, .LBB10_4
// %bb.3:
	bl	mas_blk_get_lld
	mov	x1, x19
	bl	blk_lld_busyidle_event_subscribe
	b	.LBB10_5
.LBB10_4:
	mov	w0, #-22
.LBB10_5:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldr	x19, [sp], #32          // 8-byte Folded Reload
	ret
.Lfunc_end10:
	.size	blk_busyidle_event_subscribe, .Lfunc_end10-blk_busyidle_event_subscribe
                                        // -- End function
	.globl	blk_queue_busyidle_event_subscribe // -- Begin function blk_queue_busyidle_event_subscribe
	.p2align	2
	.type	blk_queue_busyidle_event_subscribe,@function
blk_queue_busyidle_event_subscribe:     // @blk_queue_busyidle_event_subscribe
// %bb.0:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	mov	x8, x0
	mov	w0, #-22
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	cbz	x8, .LBB11_3
// %bb.1:
	mov	x19, x1
	cbz	x1, .LBB11_3
// %bb.2:
	mov	x0, x8
	bl	mas_blk_get_lld
	mov	x1, x19
	bl	blk_lld_busyidle_event_subscribe
.LBB11_3:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldr	x19, [sp], #32          // 8-byte Folded Reload
	ret
.Lfunc_end11:
	.size	blk_queue_busyidle_event_subscribe, .Lfunc_end11-blk_queue_busyidle_event_subscribe
                                        // -- End function
	.globl	blk_lld_busyidle_event_subscribe // -- Begin function blk_lld_busyidle_event_subscribe
	.p2align	2
	.type	blk_lld_busyidle_event_subscribe,@function
blk_lld_busyidle_event_subscribe:       // @blk_lld_busyidle_event_subscribe
// %bb.0:
	stp	x24, x23, [sp, #-64]!   // 16-byte Folded Spill
	stp	x22, x21, [sp, #16]     // 16-byte Folded Spill
	mov	w22, #-22
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	add	x29, sp, #48            // =48
	cbz	x0, .LBB12_21
// %bb.1:
	mov	x21, x1
	cbz	x1, .LBB12_21
// %bb.2:
	str	x0, [x21, #48]
	adrp	x8, kmalloc_caches+16
	mov	x20, x0
	ldr	x0, [x8, :lo12:kmalloc_caches+16]
	mov	w1, #32960
	movk	w1, #320, lsl #16
	mov	w2, #152
	bl	kmem_cache_alloc_trace
	cbz	x0, .LBB12_6
// %bb.3:
	ldp	x8, x9, [x21]
	ldr	x10, [x21, #16]
	mov	x22, x0
	mov	x19, x0
	str	x8, [x22, #88]!
	stp	x9, x10, [x22, #8]
	ldp	x8, x9, [x21, #40]
	ldp	x10, x11, [x21, #24]
	stp	x8, x9, [x22, #40]
	stp	x10, x11, [x22, #24]
	cbz	x11, .LBB12_7
// %bb.4:
	mov	x0, x22
	bl	strlen
	sub	x8, x0, #1              // =1
	cmp	x8, #31                 // =31
	b.lo	.LBB12_9
// %bb.5:
	adrp	x0, .L.str.17
	add	x0, x0, :lo12:.L.str.17
	b	.LBB12_8
.LBB12_6:
	mov	w22, #-12
	b	.LBB12_21
.LBB12_7:
	adrp	x0, .L.str.16
	add	x0, x0, :lo12:.L.str.16
.LBB12_8:
	adrp	x1, .L__func__.is_valid_event_node
	add	x1, x1, :lo12:.L__func__.is_valid_event_node
	bl	printk
	bl	dump_stack
	mov	w22, #-22
	b	.LBB12_20
.LBB12_9:
	add	x21, x20, #1256         // =1256
	mov	x0, x21
	bl	mutex_lock
	ldrb	w8, [x20, #25]
	tbz	w8, #0, .LBB12_11
// %bb.10:
	add	x0, x20, #1432          // =1432
	bl	mutex_lock
.LBB12_11:
	ldr	x24, [x20, #1152]
	add	x23, x20, #1152         // =1152
	cmp	x24, x23
	b.ne	.LBB12_13
	b	.LBB12_16
.LBB12_12:                              //   in Loop: Header=BB12_13 Depth=1
	ldr	x24, [x24]
	cmp	x24, x23
	b.eq	.LBB12_16
.LBB12_13:                              // =>This Inner Loop Header: Depth=1
	add	x1, x24, #88            // =88
	mov	x0, x22
	bl	strcmp
	cbnz	w0, .LBB12_12
// %bb.14:                              //   in Loop: Header=BB12_13 Depth=1
	ldr	x8, [x19, #120]
	ldr	x9, [x24, #120]
	cmp	x8, x9
	b.ne	.LBB12_12
// %bb.15:
	adrp	x0, .L.str.14
	adrp	x1, .L__func__.mas_blk_busyidle_event_register
	add	x0, x0, :lo12:.L.str.14
	add	x1, x1, :lo12:.L__func__.mas_blk_busyidle_event_register
	mov	x2, x22
	bl	printk
	bl	dump_stack
	mov	w22, #-1
	ldrb	w8, [x20, #25]
	tbnz	w8, #0, .LBB12_17
	b	.LBB12_18
.LBB12_16:
	add	x0, x19, #16            // =16
	mov	w1, wzr
	mov	x2, xzr
	mov	x3, xzr
	bl	init_timer_key
	adrp	x8, __cfi_mas_blk_busyidle_handler_latency_check_timer_expire
	adrp	x9, __cfi_mas_blk_busyidle_notify_handler
	add	x8, x8, :lo12:__cfi_mas_blk_busyidle_handler_latency_check_timer_expire
	add	x9, x9, :lo12:__cfi_mas_blk_busyidle_notify_handler
	mov	x1, x19
	mov	x10, #4294967297
	stp	x8, x19, [x19, #40]
	str	x9, [x1, #64]!
	str	wzr, [x19, #80]
	str	x10, [x19, #144]
	ldr	x8, [x20, #1160]
	add	x0, x20, #1168          // =1168
	str	x19, [x20, #1160]
	stp	x23, x8, [x19]
	str	x19, [x8]
	bl	blocking_notifier_chain_register
	adrp	x0, .L.str.15
	add	x0, x0, :lo12:.L.str.15
	mov	x1, x22
	bl	printk
	mov	w22, wzr
	ldrb	w8, [x20, #25]
	tbz	w8, #0, .LBB12_18
.LBB12_17:
	add	x0, x20, #1432          // =1432
	bl	mutex_unlock
.LBB12_18:
	mov	x0, x21
	bl	mutex_unlock
	cbz	w22, .LBB12_21
// %bb.19:
	mov	w22, #-1
.LBB12_20:
	mov	x0, x19
	bl	kfree
.LBB12_21:
	mov	w0, w22
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]     // 16-byte Folded Reload
	ldp	x24, x23, [sp], #64     // 16-byte Folded Reload
	ret
.Lfunc_end12:
	.size	blk_lld_busyidle_event_subscribe, .Lfunc_end12-blk_lld_busyidle_event_subscribe
                                        // -- End function
	.globl	blk_busyidle_event_unsubscribe // -- Begin function blk_busyidle_event_unsubscribe
	.p2align	2
	.type	blk_busyidle_event_unsubscribe,@function
blk_busyidle_event_unsubscribe:         // @blk_busyidle_event_unsubscribe
// %bb.0:
	cbz	x0, .LBB13_2
// %bb.1:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	mov	x29, sp
	bl	mas_blk_busyidle_event_unregister
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.LBB13_2:
	mov	w0, #-22
	ret
.Lfunc_end13:
	.size	blk_busyidle_event_unsubscribe, .Lfunc_end13-blk_busyidle_event_unsubscribe
                                        // -- End function
	.p2align	2               // -- Begin function mas_blk_busyidle_event_unregister
	.type	mas_blk_busyidle_event_unregister,@function
mas_blk_busyidle_event_unregister:      // @mas_blk_busyidle_event_unregister
// %bb.0:
	stp	x24, x23, [sp, #-64]!   // 16-byte Folded Spill
	stp	x22, x21, [sp, #16]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	ldr	x23, [x0, #48]
	mov	x21, x0
	add	x29, sp, #48            // =48
	add	x19, x23, #1256         // =1256
	mov	x0, x19
	bl	mutex_lock
	ldrb	w8, [x23, #25]
	tbz	w8, #0, .LBB14_2
// %bb.1:
	add	x0, x23, #1432          // =1432
	bl	mutex_lock
.LBB14_2:
	ldr	x20, [x23, #1152]
	add	x24, x23, #1152         // =1152
	cmp	x20, x24
	b.ne	.LBB14_4
	b	.LBB14_7
.LBB14_3:                               //   in Loop: Header=BB14_4 Depth=1
	ldr	x20, [x20]
	cmp	x20, x24
	b.eq	.LBB14_7
.LBB14_4:                               // =>This Inner Loop Header: Depth=1
	add	x22, x20, #88           // =88
	mov	x0, x21
	mov	x1, x22
	bl	strcmp
	cbnz	w0, .LBB14_3
// %bb.5:                               //   in Loop: Header=BB14_4 Depth=1
	ldr	x8, [x21, #32]
	ldr	x9, [x20, #120]
	cmp	x8, x9
	b.ne	.LBB14_3
// %bb.6:
	add	x21, x20, #64           // =64
	add	x0, x23, #1168          // =1168
	mov	x1, x21
	bl	blocking_notifier_chain_unregister
	adrp	x0, .L.str.19
	add	x0, x0, :lo12:.L.str.19
	mov	x1, x22
	mov	x2, x21
	bl	printk
	ldp	x8, x9, [x20]
	mov	x0, x20
	str	x9, [x8, #8]
	str	x8, [x9]
	str	x20, [x20]
	str	x20, [x20, #8]
	bl	kfree
	mov	w20, wzr
	ldrb	w8, [x23, #25]
	tbnz	w8, #0, .LBB14_8
	b	.LBB14_9
.LBB14_7:
	adrp	x0, .L.str.18
	add	x0, x0, :lo12:.L.str.18
	bl	printk
	bl	dump_stack
	mov	w20, #-22
	ldrb	w8, [x23, #25]
	tbz	w8, #0, .LBB14_9
.LBB14_8:
	add	x0, x23, #1432          // =1432
	bl	mutex_unlock
.LBB14_9:
	mov	x0, x19
	bl	mutex_unlock
	mov	w0, w20
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]     // 16-byte Folded Reload
	ldp	x24, x23, [sp], #64     // 16-byte Folded Reload
	ret
.Lfunc_end14:
	.size	mas_blk_busyidle_event_unregister, .Lfunc_end14-mas_blk_busyidle_event_unregister
                                        // -- End function
	.globl	blk_queue_busyidle_event_unsubscribe // -- Begin function blk_queue_busyidle_event_unsubscribe
	.p2align	2
	.type	blk_queue_busyidle_event_unsubscribe,@function
blk_queue_busyidle_event_unsubscribe:   // @blk_queue_busyidle_event_unsubscribe
// %bb.0:
	cbz	x0, .LBB15_2
// %bb.1:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	mov	x29, sp
	bl	mas_blk_busyidle_event_unregister
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.LBB15_2:
	mov	w0, #-22
	ret
.Lfunc_end15:
	.size	blk_queue_busyidle_event_unsubscribe, .Lfunc_end15-blk_queue_busyidle_event_unsubscribe
                                        // -- End function
	.globl	blk_queue_busyidle_enable // -- Begin function blk_queue_busyidle_enable
	.p2align	2
	.type	blk_queue_busyidle_enable,@function
blk_queue_busyidle_enable:              // @blk_queue_busyidle_enable
// %bb.0:
	cbz	x0, .LBB16_2
// %bb.1:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	mov	w19, w1
	bl	mas_blk_get_lld
	ldr	x8, [x0, #24]
	cmp	w19, #0                 // =0
	and	x9, x8, #0xfffffffffffffff7
	orr	x8, x8, #0x8
	csel	x8, x9, x8, eq
	str	x8, [x0, #24]
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldr	x19, [sp], #32          // 8-byte Folded Reload
.LBB16_2:
	ret
.Lfunc_end16:
	.size	blk_queue_busyidle_enable, .Lfunc_end16-blk_queue_busyidle_enable
                                        // -- End function
	.globl	blk_mq_tagset_busyidle_enable // -- Begin function blk_mq_tagset_busyidle_enable
	.p2align	2
	.type	blk_mq_tagset_busyidle_enable,@function
blk_mq_tagset_busyidle_enable:          // @blk_mq_tagset_busyidle_enable
// %bb.0:
	cbz	x0, .LBB17_2
// %bb.1:
	ldr	x8, [x0, #40]
	cmp	w1, #0                  // =0
	and	x9, x8, #0xfffffffffffffff7
	orr	x8, x8, #0x8
	csel	x8, x9, x8, eq
	str	x8, [x0, #40]
.LBB17_2:
	ret
.Lfunc_end17:
	.size	blk_mq_tagset_busyidle_enable, .Lfunc_end17-blk_mq_tagset_busyidle_enable
                                        // -- End function
	.globl	blk_mq_tagset_hw_idle_notify_enable // -- Begin function blk_mq_tagset_hw_idle_notify_enable
	.p2align	2
	.type	blk_mq_tagset_hw_idle_notify_enable,@function
blk_mq_tagset_hw_idle_notify_enable:    // @blk_mq_tagset_hw_idle_notify_enable
// %bb.0:
	cbz	x0, .LBB18_2
// %bb.1:
	cmp	w1, #0                  // =0
	cset	w8, ne
	strb	w8, [x0, #1056]
.LBB18_2:
	ret
.Lfunc_end18:
	.size	blk_mq_tagset_hw_idle_notify_enable, .Lfunc_end18-blk_mq_tagset_hw_idle_notify_enable
                                        // -- End function
	.globl	mas_blk_busyidle_state_init // -- Begin function mas_blk_busyidle_state_init
	.p2align	2
	.type	mas_blk_busyidle_state_init,@function
mas_blk_busyidle_state_init:            // @mas_blk_busyidle_state_init
// %bb.0:
	str	x23, [sp, #-64]!        // 8-byte Folded Spill
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	adrp	x20, blk_idle_notify_workqueue
	ldr	x8, [x20, :lo12:blk_idle_notify_workqueue]
	mov	x19, x0
	stp	x22, x21, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	add	x29, sp, #48            // =48
	cbnz	x8, .LBB19_2
// %bb.1:
	adrp	x0, .L.str.4
	add	x0, x0, :lo12:.L.str.4
	orr	w1, wzr, #0x18
	mov	w2, wzr
	mov	x3, xzr
	mov	x4, xzr
	bl	__alloc_workqueue_key
	str	x0, [x20, :lo12:blk_idle_notify_workqueue]
	cbz	x0, .LBB19_5
.LBB19_2:
	adrp	x20, blk_fg_io_idle_notify_workqueue
	ldr	x8, [x20, :lo12:blk_fg_io_idle_notify_workqueue]
	cbnz	x8, .LBB19_4
// %bb.3:
	adrp	x0, .L.str.20
	add	x0, x0, :lo12:.L.str.20
	orr	w1, wzr, #0x18
	mov	w2, wzr
	mov	x3, xzr
	mov	x4, xzr
	bl	__alloc_workqueue_key
	str	x0, [x20, :lo12:blk_fg_io_idle_notify_workqueue]
	cbz	x0, .LBB19_6
.LBB19_4:
	mov	x20, #68719476704
	adrp	x8, __cfi_mas_blk_idle_notify_work
	str	x20, [x19, #8]!
	mov	w21, #5
	add	x8, x8, :lo12:__cfi_mas_blk_idle_notify_work
	add	x9, x19, #8             // =8
	add	x0, x19, #32            // =32
	orr	w1, wzr, #0x200000
	mov	x2, xzr
	mov	x3, xzr
	str	w21, [x19, #96]
	str	x9, [x19, #8]
	stp	x9, x8, [x19, #16]
	bl	init_timer_key
	adrp	x22, delayed_work_timer_fn
	adrp	x1, .L.str.6
	adrp	x2, mas_blk_busyidle_state_init.__key
	add	x22, x22, :lo12:delayed_work_timer_fn
	add	x0, x19, #208           // =208
	add	x1, x1, :lo12:.L.str.6
	add	x2, x2, :lo12:mas_blk_busyidle_state_init.__key
	stp	x22, x19, [x19, #56]
	str	wzr, [x19, #200]
	bl	__mutex_init
	adrp	x1, .L.str.8
	adrp	x2, mas_blk_busyidle_state_init.__key.7
	add	x8, x19, #104           // =104
	add	x0, x19, #120           // =120
	add	x1, x1, :lo12:.L.str.8
	add	x2, x2, :lo12:mas_blk_busyidle_state_init.__key.7
	str	x8, [x19, #104]
	str	x8, [x19, #112]
	bl	__init_rwsem
	adrp	x9, mas_blk_fg_io_idle_notify_work
	orr	w23, wzr, #0x1
	add	x8, x19, #288           // =288
	add	x9, x9, :lo12:mas_blk_fg_io_idle_notify_work
	add	x0, x19, #312           // =312
	orr	w1, wzr, #0x200000
	mov	x2, xzr
	mov	x3, xzr
	str	xzr, [x19, #192]
	str	w21, [x19, #376]
	add	x21, x19, #280          // =280
	str	x20, [x19, #280]
	str	w23, [x19, #272]
	str	x8, [x19, #288]
	stp	x8, x9, [x19, #296]
	bl	init_timer_key
	adrp	x1, .L.str.21
	adrp	x2, mas_blk_fg_io_busyidle_state_init.__key
	add	x0, x19, #384           // =384
	add	x1, x1, :lo12:.L.str.21
	add	x2, x2, :lo12:mas_blk_fg_io_busyidle_state_init.__key
	stp	x22, x21, [x19, #336]
	str	wzr, [x19, #380]
	bl	__mutex_init
	adrp	x1, .L.str.10
	adrp	x2, mas_blk_busyidle_state_init.__key.9
	add	x8, x19, #472           // =472
	add	x9, x19, #488           // =488
	add	x0, x19, #504           // =504
	add	x1, x1, :lo12:.L.str.10
	add	x2, x2, :lo12:mas_blk_busyidle_state_init.__key.9
	str	w23, [x19, #448]
	stp	xzr, xzr, [x19, #456]
	str	x8, [x19, #472]
	str	x8, [x19, #480]
	str	x9, [x19, #488]
	str	x9, [x19, #496]
	bl	__raw_spin_lock_init
	mov	w0, wzr
	b	.LBB19_8
.LBB19_5:
	adrp	x0, .L.str.5
	adrp	x1, .L__func__.mas_blk_busyidle_state_init
	add	x0, x0, :lo12:.L.str.5
	add	x1, x1, :lo12:.L__func__.mas_blk_busyidle_state_init
	b	.LBB19_7
.LBB19_6:
	adrp	x0, .L.str.5
	adrp	x1, .L__func__.mas_blk_alloc_fg_io_workqueue
	add	x0, x0, :lo12:.L.str.5
	add	x1, x1, :lo12:.L__func__.mas_blk_alloc_fg_io_workqueue
.LBB19_7:
	bl	printk
	mov	w0, #-12
.LBB19_8:
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]     // 16-byte Folded Reload
	ldr	x23, [sp], #64          // 8-byte Folded Reload
	ret
.Lfunc_end19:
	.size	mas_blk_busyidle_state_init, .Lfunc_end19-mas_blk_busyidle_state_init
                                        // -- End function
	.p2align	2               // -- Begin function mas_blk_fg_io_idle_notify_work
	.type	mas_blk_fg_io_idle_notify_work,@function
mas_blk_fg_io_idle_notify_work:         // @mas_blk_fg_io_idle_notify_work
// %bb.0:
	str	x21, [sp, #-48]!        // 8-byte Folded Spill
	mov	x21, x0
	sub	x0, x0, #1328           // =1328
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	bl	mas_blk_get_queue_by_lld
	add	x19, x21, #104          // =104
	mov	x20, x0
	mov	x0, x19
	bl	mutex_lock
	ldr	w8, [x21, #100]
	cbnz	w8, .LBB20_3
// %bb.1:
	ldr	w8, [x21, #168]
	cmp	w8, #1                  // =1
	b.eq	.LBB20_3
// %bb.2:
	orr	w8, wzr, #0x1
	sub	x0, x21, #160           // =160
	orr	w1, wzr, #0x3
	mov	x2, x20
	str	w8, [x21, #168]
	bl	blocking_notifier_call_chain
.LBB20_3:
	mov	x0, x19
	bl	mutex_unlock
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldr	x21, [sp], #48          // 8-byte Folded Reload
	ret
.Lfunc_end20:
	.size	mas_blk_fg_io_idle_notify_work, .Lfunc_end20-mas_blk_fg_io_idle_notify_work
                                        // -- End function
	.type	.L.str,@object          // @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	"\0013[BLK-IO]%s: %s process time is more than %u ms\n"
	.size	.L.str, 50

	.type	.L__func__.mas_blk_busyidle_handler_latency_check_timer_expire,@object // @__func__.mas_blk_busyidle_handler_latency_check_timer_expire
.L__func__.mas_blk_busyidle_handler_latency_check_timer_expire:
	.asciz	"mas_blk_busyidle_handler_latency_check_timer_expire"
	.size	.L__func__.mas_blk_busyidle_handler_latency_check_timer_expire, 52

	.type	.L.str.1,@object        // @.str.1
.L.str.1:
	.asciz	"\0013[BLK-IO]%s: %s NULL callback\n"
	.size	.L.str.1, 32

	.type	.L__func__.mas_blk_busyidle_notify_handler,@object // @__func__.mas_blk_busyidle_notify_handler
.L__func__.mas_blk_busyidle_notify_handler:
	.asciz	"mas_blk_busyidle_notify_handler"
	.size	.L__func__.mas_blk_busyidle_notify_handler, 32

	.type	.L.str.2,@object        // @.str.2
.L.str.2:
	.asciz	"\0013[BLK-IO]%s: %s error!\n"
	.size	.L.str.2, 25

	.type	.L.str.3,@object        // @.str.3
.L.str.3:
	.asciz	"\0013[BLK-IO]%s: %s busy idle callback cost %lld ms\n"
	.size	.L.str.3, 50

	.type	blk_fg_io_idle_notify_workqueue,@object // @blk_fg_io_idle_notify_workqueue
	.local	blk_fg_io_idle_notify_workqueue
	.comm	blk_fg_io_idle_notify_workqueue,8,8
	.type	blk_idle_notify_workqueue,@object // @blk_idle_notify_workqueue
	.local	blk_idle_notify_workqueue
	.comm	blk_idle_notify_workqueue,8,8
	.type	.L.str.4,@object        // @.str.4
.L.str.4:
	.asciz	"busyidle_notify"
	.size	.L.str.4, 16

	.type	.L.str.5,@object        // @.str.5
.L.str.5:
	.asciz	"\0013[BLK-IO]%s: Failed to alloc workqueue!\n"
	.size	.L.str.5, 42

	.type	.L__func__.mas_blk_busyidle_state_init,@object // @__func__.mas_blk_busyidle_state_init
.L__func__.mas_blk_busyidle_state_init:
	.asciz	"mas_blk_busyidle_state_init"
	.size	.L__func__.mas_blk_busyidle_state_init, 28

	.type	mas_blk_busyidle_state_init.__key,@object // @mas_blk_busyidle_state_init.__key
	.local	mas_blk_busyidle_state_init.__key
	.comm	mas_blk_busyidle_state_init.__key,1,1
	.type	.L.str.6,@object        // @.str.6
.L.str.6:
	.asciz	"&blk_idle->io_count_mutex"
	.size	.L.str.6, 26

	.type	mas_blk_busyidle_state_init.__key.7,@object // @mas_blk_busyidle_state_init.__key.7
	.local	mas_blk_busyidle_state_init.__key.7
	.comm	mas_blk_busyidle_state_init.__key.7,1,1
	.type	.L.str.8,@object        // @.str.8
.L.str.8:
	.asciz	"&(&blk_idle->nh)->rwsem"
	.size	.L.str.8, 24

	.type	mas_blk_busyidle_state_init.__key.9,@object // @mas_blk_busyidle_state_init.__key.9
	.local	mas_blk_busyidle_state_init.__key.9
	.comm	mas_blk_busyidle_state_init.__key.9,1,1
	.type	.L.str.10,@object       // @.str.10
.L.str.10:
	.asciz	"&(&blk_idle->counted_list_lock)->rlock"
	.size	.L.str.10, 39

	.type	.L.str.12,@object       // @.str.12
.L.str.12:
	.asciz	"\0013[BLK-IO]%s: io_count has been zero\n"
	.size	.L.str.12, 38

	.type	.L__func__.blk_idle_count,@object // @__func__.blk_idle_count
.L__func__.blk_idle_count:
	.asciz	"blk_idle_count"
	.size	.L__func__.blk_idle_count, 15

	.type	.L.str.13,@object       // @.str.13
.L.str.13:
	.asciz	"\0013[BLK-IO]<%s> io_count has been zero\n"
	.size	.L.str.13, 39

	.type	.L__func__.blk_fg_io_idle_count,@object // @__func__.blk_fg_io_idle_count
.L__func__.blk_fg_io_idle_count:
	.asciz	"blk_fg_io_idle_count"
	.size	.L__func__.blk_fg_io_idle_count, 21

	.type	.L.str.14,@object       // @.str.14
.L.str.14:
	.asciz	"\0013[BLK-IO]%s: %s has been registered already!\n"
	.size	.L.str.14, 47

	.type	.L__func__.mas_blk_busyidle_event_register,@object // @__func__.mas_blk_busyidle_event_register
.L__func__.mas_blk_busyidle_event_register:
	.asciz	"mas_blk_busyidle_event_register"
	.size	.L__func__.mas_blk_busyidle_event_register, 32

	.type	.L.str.15,@object       // @.str.15
.L.str.15:
	.asciz	"\0013[BLK-IO]%s registered to busy idle\n"
	.size	.L.str.15, 38

	.type	.L.str.16,@object       // @.str.16
.L.str.16:
	.asciz	"\0010[BLK-IO]%s: notifier_callback is NULL\n"
	.size	.L.str.16, 41

	.type	.L__func__.is_valid_event_node,@object // @__func__.is_valid_event_node
.L__func__.is_valid_event_node:
	.asciz	"is_valid_event_node"
	.size	.L__func__.is_valid_event_node, 20

	.type	.L.str.17,@object       // @.str.17
.L.str.17:
	.asciz	"\0010[BLK-IO]%s: Invalid subscriber\n"
	.size	.L.str.17, 34

	.type	.L.str.18,@object       // @.str.18
.L.str.18:
	.asciz	"\0013[BLK-IO]Trying to unregister an unregistered event_node!\n"
	.size	.L.str.18, 60

	.type	.L.str.19,@object       // @.str.19
.L.str.19:
	.asciz	"\0013[BLK-IO]%s unregistered from busy idle module 0x%pK\n"
	.size	.L.str.19, 55

	.type	.L.str.20,@object       // @.str.20
.L.str.20:
	.asciz	"fg_busy_idle_notify"
	.size	.L.str.20, 20

	.type	.L__func__.mas_blk_alloc_fg_io_workqueue,@object // @__func__.mas_blk_alloc_fg_io_workqueue
.L__func__.mas_blk_alloc_fg_io_workqueue:
	.asciz	"mas_blk_alloc_fg_io_workqueue"
	.size	.L__func__.mas_blk_alloc_fg_io_workqueue, 30

	.type	mas_blk_fg_io_busyidle_state_init.__key,@object // @mas_blk_fg_io_busyidle_state_init.__key
	.local	mas_blk_fg_io_busyidle_state_init.__key
	.comm	mas_blk_fg_io_busyidle_state_init.__key,1,1
	.type	.L.str.21,@object       // @.str.21
.L.str.21:
	.asciz	"&blk_idle->fg_io_count_mutex"
	.size	.L.str.21, 29


	.ident	"Android (27847 based on r353983c) clang version 9.0.3 (https://android.googlesource.com/toolchain/clang dbdb7a2acf33dc76cd6f506c323253b8d892ae05) (https://android.googlesource.com/toolchain/llvm f32df87f6769ededcd9e2a6c3c4c45c1baf361c2) (based on LLVM 9.0.3svn)"
	.section	".note.GNU-stack","",@progbits

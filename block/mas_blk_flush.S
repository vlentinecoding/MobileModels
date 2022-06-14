	.text
	.file	"mas_blk_flush.c"
                                        // Start of file scope inline assembly
	.section "___kcrctab+mas_blk_panic_flush", "a"	
	.weak	__crc_mas_blk_panic_flush	
	.long	__crc_mas_blk_panic_flush	
	.previous					

                                        // End of file scope inline assembly
	.globl	mas_blk_flush_work_fn   // -- Begin function mas_blk_flush_work_fn
	.p2align	2
	.type	mas_blk_flush_work_fn,@function
mas_blk_flush_work_fn:                  // @mas_blk_flush_work_fn
// %bb.0:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	ldur	x0, [x0, #-24]
	mov	w1, wzr
	add	x29, sp, #16            // =16
	bl	bdget_disk
	cbz	x0, .LBB0_6
// %bb.1:
	orr	w1, wzr, #0x2
	mov	x2, xzr
	mov	x19, x0
	bl	blkdev_get
	cbz	w0, .LBB0_3
// %bb.2:
	adrp	x0, .L.str
	adrp	x1, .L__func__.mas_blk_flush_work_fn
	add	x0, x0, :lo12:.L.str
	add	x1, x1, :lo12:.L__func__.mas_blk_flush_work_fn
	bl	printk
	b	.LBB0_6
.LBB0_3:
	mov	w1, #192
	movk	w1, #320, lsl #16
	mov	x0, x19
	mov	x2, xzr
	bl	blkdev_issue_flush
	cbz	w0, .LBB0_5
// %bb.4:
	adrp	x0, .L.str.1
	adrp	x1, .L__func__.mas_blk_flush_work_fn
	add	x0, x0, :lo12:.L.str.1
	add	x1, x1, :lo12:.L__func__.mas_blk_flush_work_fn
	bl	printk
.LBB0_5:
	orr	w1, wzr, #0x2
	mov	x0, x19
	bl	blkdev_put
.LBB0_6:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldr	x19, [sp], #32          // 8-byte Folded Reload
	ret
.Lfunc_end0:
	.size	mas_blk_flush_work_fn, .Lfunc_end0-mas_blk_flush_work_fn
                                        // -- End function
	.globl	mas_blk_flush_async_dispatch // -- Begin function mas_blk_flush_async_dispatch
	.p2align	2
	.type	mas_blk_flush_async_dispatch,@function
mas_blk_flush_async_dispatch:           // @mas_blk_flush_async_dispatch
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	mov	x20, x1
	mov	x19, x0
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB1_3
// %bb.1:
	ldrb	w8, [x20, #18]
	tbnz	w8, #7, .LBB1_3
// %bb.2:
	ldr	w8, [x20, #48]
	cbz	w8, .LBB1_8
.LBB1_3:
	ldr	w8, [x19, #1696]
	cbz	w8, .LBB1_8
// %bb.4:
	ldr	w8, [x20, #16]
	tbz	w8, #19, .LBB1_6
// %bb.5:
	ldr	w9, [x20, #48]
	cbz	w9, .LBB1_10
.LBB1_6:
	and	w8, w8, #0xff
	cmp	w8, #1                  // =1
	b.ne	.LBB1_8
// %bb.7:
	add	x8, x19, #1676          // =1676
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
.LBB1_8:
	mov	w0, wzr
.LBB1_9:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.LBB1_10:
	ldrb	w9, [x20, #146]
	cbz	w9, .LBB1_12
// %bb.11:
	add	x19, x19, #1576         // =1576
	mov	x0, x19
	bl	cancel_delayed_work
	adrp	x8, blk_async_flush_workqueue
	ldr	x1, [x8, :lo12:blk_async_flush_workqueue]
	orr	w0, wzr, #0x8
	mov	w3, #5
	mov	x2, x19
	bl	queue_delayed_work_on
	orr	w0, wzr, #0x1
	b	.LBB1_9
.LBB1_12:
	ldr	w9, [x19, #1676]
	cbnz	w9, .LBB1_6
// %bb.13:
	orr	w0, wzr, #0x1
	b	.LBB1_9
.Lfunc_end1:
	.size	mas_blk_flush_async_dispatch, .Lfunc_end1-mas_blk_flush_async_dispatch
                                        // -- End function
	.globl	mas_blk_flush_update    // -- Begin function mas_blk_flush_update
	.p2align	2
	.type	mas_blk_flush_update,@function
mas_blk_flush_update:                   // @mas_blk_flush_update
// %bb.0:
	cbnz	w1, .LBB2_2
// %bb.1:
	ldr	x8, [x0, #64]
	str	wzr, [x8, #1676]
.LBB2_2:
	ret
.Lfunc_end2:
	.size	mas_blk_flush_update, .Lfunc_end2-mas_blk_flush_update
                                        // -- End function
	.globl	blk_power_off_flush     // -- Begin function blk_power_off_flush
	.p2align	2
	.type	blk_power_off_flush,@function
blk_power_off_flush:                    // @blk_power_off_flush
// %bb.0:
	str	x27, [sp, #-96]!        // 8-byte Folded Spill
	mov	w2, w0
	adrp	x0, .L.str.2
	adrp	x1, .L__func__.blk_power_off_flush
	add	x0, x0, :lo12:.L.str.2
	add	x1, x1, :lo12:.L__func__.blk_power_off_flush
	stp	x26, x25, [sp, #16]     // 16-byte Folded Spill
	stp	x24, x23, [sp, #32]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #48]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #64]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #80]     // 16-byte Folded Spill
	add	x29, sp, #80            // =80
	bl	printk
	adrp	x0, all_flush_q_lock
	add	x0, x0, :lo12:all_flush_q_lock
	bl	_raw_spin_lock_irqsave
	adrp	x26, all_flush_q_list
	add	x26, x26, :lo12:all_flush_q_list
	ldr	x27, [x26]
	mov	x19, x0
	cmp	x27, x26
	b.eq	.LBB3_9
// %bb.1:
	adrp	x20, .L.str.3
	adrp	x21, .L__func__.blk_power_off_flush
	adrp	x22, .L.str.4
	add	x20, x20, :lo12:.L.str.3
	add	x21, x21, :lo12:.L__func__.blk_power_off_flush
	add	x22, x22, :lo12:.L.str.4
.LBB3_2:                                // =>This Inner Loop Header: Depth=1
	ldurb	w8, [x27, #-120]
	cbz	w8, .LBB3_8
// %bb.3:                               //   in Loop: Header=BB3_2 Depth=1
	sub	x23, x27, #1680         // =1680
	mov	x0, x23
	bl	mas_blk_get_lld
	cbz	x0, .LBB3_8
// %bb.4:                               //   in Loop: Header=BB3_2 Depth=1
	ldr	x8, [x0, #56]
	mov	x24, x0
	cbz	x8, .LBB3_8
// %bb.5:                               //   in Loop: Header=BB3_2 Depth=1
	ldur	x9, [x27, #-128]
	cbz	x9, .LBB3_7
// %bb.6:                               //   in Loop: Header=BB3_2 Depth=1
	add	x25, x9, #12            // =12
	mov	x0, x20
	mov	x1, x21
	mov	x2, x25
	bl	printk
	ldr	x8, [x24, #56]
	mov	x0, x23
	blr	x8
	mov	x0, x22
	mov	x1, x21
	mov	x2, x25
	bl	printk
	b	.LBB3_8
.LBB3_7:                                //   in Loop: Header=BB3_2 Depth=1
	mov	x0, x23
	blr	x8
.LBB3_8:                                //   in Loop: Header=BB3_2 Depth=1
	ldr	x27, [x27]
	cmp	x27, x26
	b.ne	.LBB3_2
.LBB3_9:
	adrp	x0, all_flush_q_lock
	add	x0, x0, :lo12:all_flush_q_lock
	mov	x1, x19
	bl	_raw_spin_unlock_irqrestore
	adrp	x0, .L.str.5
	adrp	x1, .L__func__.blk_power_off_flush
	add	x0, x0, :lo12:.L.str.5
	add	x1, x1, :lo12:.L__func__.blk_power_off_flush
	bl	printk
	ldp	x29, x30, [sp, #80]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #64]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #48]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #32]     // 16-byte Folded Reload
	ldp	x26, x25, [sp, #16]     // 16-byte Folded Reload
	ldr	x27, [sp], #96          // 8-byte Folded Reload
	ret
.Lfunc_end3:
	.size	blk_power_off_flush, .Lfunc_end3-blk_power_off_flush
                                        // -- End function
	.globl	mas_blk_flush_list_register // -- Begin function mas_blk_flush_list_register
	.p2align	2
	.type	mas_blk_flush_list_register,@function
mas_blk_flush_list_register:            // @mas_blk_flush_list_register
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	adrp	x20, all_object_lock
	add	x20, x20, :lo12:all_object_lock
	mov	x19, x0
	mov	x0, x20
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	bl	_raw_spin_lock_irqsave
	adrp	x8, all_object_list
	add	x8, x8, :lo12:all_object_list
	ldr	x9, [x8]
	mov	x1, x0
	adrp	x10, all_object_list
	mov	x0, x20
	str	x19, [x9, #8]
	stp	x9, x8, [x19]
	str	x19, [x10, :lo12:all_object_list]
	bl	_raw_spin_unlock_irqrestore
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end4:
	.size	mas_blk_flush_list_register, .Lfunc_end4-mas_blk_flush_list_register
                                        // -- End function
	.globl	mas_blk_flush_list_unregister // -- Begin function mas_blk_flush_list_unregister
	.p2align	2
	.type	mas_blk_flush_list_unregister,@function
mas_blk_flush_list_unregister:          // @mas_blk_flush_list_unregister
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	adrp	x20, all_object_lock
	add	x20, x20, :lo12:all_object_lock
	mov	x19, x0
	mov	x0, x20
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	bl	_raw_spin_lock_irqsave
	ldp	x8, x9, [x19]
	mov	x1, x0
	mov	x0, x20
	str	x9, [x8, #8]
	str	x8, [x9]
	str	x19, [x19]
	str	x19, [x19, #8]
	bl	_raw_spin_unlock_irqrestore
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end5:
	.size	mas_blk_flush_list_unregister, .Lfunc_end5-mas_blk_flush_list_unregister
                                        // -- End function
	.globl	mas_blk_order_panic_wait_flush // -- Begin function mas_blk_order_panic_wait_flush
	.p2align	2
	.type	mas_blk_order_panic_wait_flush,@function
mas_blk_order_panic_wait_flush:         // @mas_blk_order_panic_wait_flush
// %bb.0:
	stp	x26, x25, [sp, #-80]!   // 16-byte Folded Spill
	stp	x20, x19, [sp, #48]     // 16-byte Folded Spill
	adrp	x19, .L__func__.mas_blk_order_panic_wait_flush
	adrp	x0, .L.str.6
	add	x19, x19, :lo12:.L__func__.mas_blk_order_panic_wait_flush
	add	x0, x0, :lo12:.L.str.6
	mov	x1, x19
	stp	x24, x23, [sp, #16]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #32]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #64]     // 16-byte Folded Spill
	add	x29, sp, #64            // =64
	bl	printk
	adrp	x20, all_object_lock
	adrp	x25, all_object_list
	adrp	x21, .L.str.7
	mov	w24, #200
	add	x20, x20, :lo12:all_object_lock
	add	x25, x25, :lo12:all_object_list
	add	x21, x21, :lo12:.L.str.7
.LBB6_1:                                // =>This Loop Header: Depth=1
                                        //     Child Loop BB6_3 Depth 2
	mov	w0, #35160
	movk	w0, #65, lsl #16
	bl	__const_udelay
	mov	x0, x20
	bl	_raw_spin_lock_irqsave
	ldr	x26, [x25]
	mov	x22, x0
	cmp	x26, x25
	b.eq	.LBB6_11
// %bb.2:                               //   in Loop: Header=BB6_1 Depth=1
	mov	w23, wzr
.LBB6_3:                                //   Parent Loop BB6_1 Depth=1
                                        // =>  This Inner Loop Header: Depth=2
	sub	x8, x26, #1652          // =1652
	ldr	w2, [x8]
	cmp	w2, #2                  // =2
	b.ne	.LBB6_6
// %bb.4:                               //   in Loop: Header=BB6_3 Depth=2
	sub	x8, x26, #1632          // =1632
	ldr	x8, [x8]
	tst	x8, #0x80
	cset	w9, eq
	orr	w9, w23, w9
	tbz	w9, #0, .LBB6_7
// %bb.5:                               //   in Loop: Header=BB6_3 Depth=2
	ubfx	x8, x8, #7, #1
	orr	w23, w23, w8
	b	.LBB6_8
.LBB6_6:                                //   in Loop: Header=BB6_3 Depth=2
	mov	x0, x21
	mov	x1, x19
	bl	printk
	b	.LBB6_8
.LBB6_7:                                //   in Loop: Header=BB6_3 Depth=2
	sub	x0, x26, #1656          // =1656
	bl	ufs_order_panic_wait_datasync_handle
	mov	w23, w0
.LBB6_8:                                //   in Loop: Header=BB6_3 Depth=2
	ldr	x26, [x26]
	cmp	x26, x25
	b.ne	.LBB6_3
// %bb.9:                               //   in Loop: Header=BB6_1 Depth=1
	mov	x0, x20
	mov	x1, x22
	bl	_raw_spin_unlock_irqrestore
	tbnz	w23, #0, .LBB6_13
// %bb.10:                              //   in Loop: Header=BB6_1 Depth=1
	subs	w24, w24, #1            // =1
	b.ne	.LBB6_1
	b	.LBB6_12
.LBB6_11:                               //   in Loop: Header=BB6_1 Depth=1
	mov	x0, x20
	mov	x1, x22
	bl	_raw_spin_unlock_irqrestore
	subs	w24, w24, #1            // =1
	b.ne	.LBB6_1
.LBB6_12:
	orr	w0, wzr, #0x1
	bl	blk_power_off_flush
	adrp	x0, .L.str.9
	add	x0, x0, :lo12:.L.str.9
	b	.LBB6_14
.LBB6_13:
	orr	w0, wzr, #0x1
	bl	blk_power_off_flush
	adrp	x0, .L.str.8
	add	x0, x0, :lo12:.L.str.8
.LBB6_14:
	adrp	x1, .L__func__.mas_blk_order_panic_wait_flush
	add	x1, x1, :lo12:.L__func__.mas_blk_order_panic_wait_flush
	bl	printk
	ldp	x29, x30, [sp, #64]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #48]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #32]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #16]     // 16-byte Folded Reload
	ldp	x26, x25, [sp], #80     // 16-byte Folded Reload
	ret
.Lfunc_end6:
	.size	mas_blk_order_panic_wait_flush, .Lfunc_end6-mas_blk_order_panic_wait_flush
                                        // -- End function
	.globl	mas_blk_panic_flush     // -- Begin function mas_blk_panic_flush
	.p2align	2
	.type	mas_blk_panic_flush,@function
mas_blk_panic_flush:                    // @mas_blk_panic_flush
// %bb.0:
	stp	x24, x23, [sp, #-64]!   // 16-byte Folded Spill
	mov	w0, wzr
	stp	x22, x21, [sp, #16]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	add	x29, sp, #48            // =48
	bl	blk_power_off_flush
	adrp	x0, all_object_lock
	add	x0, x0, :lo12:all_object_lock
	bl	_raw_spin_lock_irqsave
	adrp	x22, all_object_list
	add	x22, x22, :lo12:all_object_list
	ldr	x23, [x22]
	mov	x19, x0
	cmp	x23, x22
	b.eq	.LBB7_9
// %bb.1:
	adrp	x20, .L.str.7
	adrp	x21, .L__func__.mas_blk_panic_flush
	mov	w24, wzr
	add	x20, x20, :lo12:.L.str.7
	add	x21, x21, :lo12:.L__func__.mas_blk_panic_flush
.LBB7_2:                                // =>This Inner Loop Header: Depth=1
	sub	x8, x23, #1652          // =1652
	ldr	w2, [x8]
	cmp	w2, #2                  // =2
	b.ne	.LBB7_5
// %bb.3:                               //   in Loop: Header=BB7_2 Depth=1
	sub	x8, x23, #1632          // =1632
	ldrb	w8, [x8]
	tbz	w8, #7, .LBB7_6
// %bb.4:                               //   in Loop: Header=BB7_2 Depth=1
	sub	x0, x23, #1656          // =1656
	bl	ufs_order_panic_datasync_handle
	orr	w24, wzr, #0x1
	b	.LBB7_6
.LBB7_5:                                //   in Loop: Header=BB7_2 Depth=1
	mov	x0, x20
	mov	x1, x21
	bl	printk
.LBB7_6:                                //   in Loop: Header=BB7_2 Depth=1
	ldr	x23, [x23]
	cmp	x23, x22
	b.ne	.LBB7_2
// %bb.7:
	adrp	x0, all_object_lock
	add	x0, x0, :lo12:all_object_lock
	mov	x1, x19
	bl	_raw_spin_unlock_irqrestore
	tbz	w24, #0, .LBB7_10
// %bb.8:
	bl	mas_blk_order_panic_wait_flush
	b	.LBB7_10
.LBB7_9:
	adrp	x0, all_object_lock
	add	x0, x0, :lo12:all_object_lock
	mov	x1, x19
	bl	_raw_spin_unlock_irqrestore
.LBB7_10:
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]     // 16-byte Folded Reload
	ldp	x24, x23, [sp], #64     // 16-byte Folded Reload
	ret
.Lfunc_end7:
	.size	mas_blk_panic_flush, .Lfunc_end7-mas_blk_panic_flush
                                        // -- End function
	.globl	mas_blk_poweroff_flush_notifier_call // -- Begin function mas_blk_poweroff_flush_notifier_call
	.p2align	2
	.type	mas_blk_poweroff_flush_notifier_call,@function
mas_blk_poweroff_flush_notifier_call:   // @mas_blk_poweroff_flush_notifier_call
// %bb.0:
	str	x23, [sp, #-64]!        // 8-byte Folded Spill
	cmp	x1, #2                  // =2
	stp	x22, x21, [sp, #16]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	add	x29, sp, #48            // =48
	b.eq	.LBB8_3
// %bb.1:
	cmp	x1, #3                  // =3
	b.ne	.LBB8_14
// %bb.2:
	orr	w0, wzr, #0x1
	bl	blk_power_off_flush
	b	.LBB8_14
.LBB8_3:
	adrp	x0, all_object_lock
	add	x0, x0, :lo12:all_object_lock
	bl	_raw_spin_lock_irqsave
	adrp	x22, all_object_list
	add	x22, x22, :lo12:all_object_list
	ldr	x23, [x22]
	mov	x19, x0
	cmp	x23, x22
	b.eq	.LBB8_13
// %bb.4:
	adrp	x20, .L.str.7
	adrp	x21, .L__func__.mas_blk_order_poweroff_proc
	add	x20, x20, :lo12:.L.str.7
	add	x21, x21, :lo12:.L__func__.mas_blk_order_poweroff_proc
.LBB8_5:                                // =>This Inner Loop Header: Depth=1
	sub	x8, x23, #1652          // =1652
	ldr	w2, [x8]
	cmp	w2, #2                  // =2
	b.ne	.LBB8_11
// %bb.6:                               //   in Loop: Header=BB8_5 Depth=1
	sub	x8, x23, #1632          // =1632
	ldrb	w8, [x8]
	tbz	w8, #7, .LBB8_12
// %bb.7:                               //   in Loop: Header=BB8_5 Depth=1
	sub	x8, x23, #1648          // =1648
	ldr	x8, [x8]
	cbz	x8, .LBB8_12
// %bb.8:                               //   in Loop: Header=BB8_5 Depth=1
	ldr	x8, [x8, #1712]
	cbz	x8, .LBB8_12
// %bb.9:                               //   in Loop: Header=BB8_5 Depth=1
	ldr	x8, [x8, #32]
	cbz	x8, .LBB8_12
// %bb.10:                              //   in Loop: Header=BB8_5 Depth=1
	sub	x0, x23, #1656          // =1656
	blr	x8
	b	.LBB8_12
.LBB8_11:                               //   in Loop: Header=BB8_5 Depth=1
	mov	x0, x20
	mov	x1, x21
	bl	printk
.LBB8_12:                               //   in Loop: Header=BB8_5 Depth=1
	ldr	x23, [x23]
	cmp	x23, x22
	b.ne	.LBB8_5
.LBB8_13:
	adrp	x0, all_object_lock
	add	x0, x0, :lo12:all_object_lock
	mov	x1, x19
	bl	_raw_spin_unlock_irqrestore
.LBB8_14:
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]     // 16-byte Folded Reload
	mov	w0, wzr
	ldr	x23, [sp], #64          // 8-byte Folded Reload
	ret
.Lfunc_end8:
	.size	mas_blk_poweroff_flush_notifier_call, .Lfunc_end8-mas_blk_poweroff_flush_notifier_call
                                        // -- End function
	.section	.init.text,"ax",@progbits
	.globl	mas_blk_flush_init      // -- Begin function mas_blk_flush_init
	.p2align	2
	.type	mas_blk_flush_init,@function
mas_blk_flush_init:                     // @mas_blk_flush_init
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	adrp	x0, mas_blk_poweroff_flush_nb
	add	x0, x0, :lo12:mas_blk_poweroff_flush_nb
	mov	x29, sp
	bl	powerkey_register_notifier
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end9:
	.size	mas_blk_flush_init, .Lfunc_end9-mas_blk_flush_init
                                        // -- End function
	.text
	.globl	mas_blk_flush_reduced_queue_unregister // -- Begin function mas_blk_flush_reduced_queue_unregister
	.p2align	2
	.type	mas_blk_flush_reduced_queue_unregister,@function
mas_blk_flush_reduced_queue_unregister: // @mas_blk_flush_reduced_queue_unregister
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	ldr	w8, [x0, #1696]
	add	x29, sp, #16            // =16
	cbz	w8, .LBB10_4
// %bb.1:
	ldr	x8, [x0, #1680]
	mov	x19, x0
	cbz	x8, .LBB10_4
// %bb.2:
	ldr	x8, [x19, #1688]
	cbz	x8, .LBB10_4
// %bb.3:
	adrp	x20, all_flush_q_lock
	add	x20, x20, :lo12:all_flush_q_lock
	mov	x0, x20
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x19, #1688]
	ldr	x9, [x19, #1680]
	mov	x10, #256
	movk	x10, #57005, lsl #48
	mov	x1, x0
	add	x11, x10, #256          // =256
	mov	x0, x20
	str	x8, [x9, #8]
	str	x9, [x8]
	str	x10, [x19, #1680]
	str	x11, [x19, #1688]
	bl	_raw_spin_unlock_irqrestore
.LBB10_4:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end10:
	.size	mas_blk_flush_reduced_queue_unregister, .Lfunc_end10-mas_blk_flush_reduced_queue_unregister
                                        // -- End function
	.globl	blk_mq_tagset_direct_flush_register // -- Begin function blk_mq_tagset_direct_flush_register
	.p2align	2
	.type	blk_mq_tagset_direct_flush_register,@function
blk_mq_tagset_direct_flush_register:    // @blk_mq_tagset_direct_flush_register
// %bb.0:
	cbz	x0, .LBB11_2
// %bb.1:
	str	x1, [x0, #72]
	ret
.LBB11_2:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	adrp	x0, .L.str.11
	adrp	x1, .L__func__.blk_mq_tagset_direct_flush_register
	add	x0, x0, :lo12:.L.str.11
	add	x1, x1, :lo12:.L__func__.blk_mq_tagset_direct_flush_register
	mov	x29, sp
	bl	printk
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end11:
	.size	blk_mq_tagset_direct_flush_register, .Lfunc_end11-blk_mq_tagset_direct_flush_register
                                        // -- End function
	.globl	mas_blk_queue_async_flush_init // -- Begin function mas_blk_queue_async_flush_init
	.p2align	2
	.type	mas_blk_queue_async_flush_init,@function
mas_blk_queue_async_flush_init:         // @mas_blk_queue_async_flush_init
// %bb.0:
	stp	x22, x21, [sp, #-48]!   // 16-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	mov	x19, x0
	bl	mas_blk_get_lld
	ldrb	w8, [x0, #24]
	tbz	w8, #2, .LBB12_10
// %bb.1:
	adrp	x21, blk_async_flush_workqueue
	ldr	x8, [x21, :lo12:blk_async_flush_workqueue]
	mov	x20, x0
	cbnz	x8, .LBB12_3
// %bb.2:
	adrp	x0, .L.str.12
	add	x0, x0, :lo12:.L.str.12
	orr	w1, wzr, #0x2
	mov	w2, wzr
	mov	x3, xzr
	mov	x4, xzr
	bl	__alloc_workqueue_key
	str	x0, [x21, :lo12:blk_async_flush_workqueue]
	cbz	x0, .LBB12_11
.LBB12_3:
	str	wzr, [x19, #1676]
	ldr	x8, [x19, #1680]
	add	x21, x19, #1680         // =1680
	cbnz	x8, .LBB12_6
// %bb.4:
	ldr	x8, [x19, #1688]
	cbnz	x8, .LBB12_6
// %bb.5:
	str	x21, [x19, #1680]
	str	x21, [x19, #1688]
.LBB12_6:
	adrp	x10, __cfi_mas_blk_flush_work_fn
	mov	x8, #68719476704
	add	x9, x19, #1584          // =1584
	add	x10, x10, :lo12:__cfi_mas_blk_flush_work_fn
	add	x0, x19, #1608          // =1608
	orr	w1, wzr, #0x200000
	mov	x2, xzr
	mov	x3, xzr
	add	x22, x19, #1576         // =1576
	str	x8, [x19, #1576]
	str	x9, [x19, #1584]
	str	x9, [x19, #1592]
	str	x10, [x19, #1600]
	bl	init_timer_key
	adrp	x8, delayed_work_timer_fn
	add	x8, x8, :lo12:delayed_work_timer_fn
	str	x8, [x19, #1632]
	str	x22, [x19, #1640]
	ldr	x8, [x20, #56]
	cbz	x8, .LBB12_9
// %bb.7:
	ldr	x8, [x21]
	cmp	x21, x8
	b.ne	.LBB12_9
// %bb.8:
	adrp	x20, all_flush_q_lock
	add	x20, x20, :lo12:all_flush_q_lock
	mov	x0, x20
	bl	_raw_spin_lock_irqsave
	adrp	x8, all_flush_q_list
	add	x8, x8, :lo12:all_flush_q_list
	ldr	x9, [x8]
	mov	x1, x0
	adrp	x10, all_flush_q_list
	mov	x0, x20
	str	x21, [x9, #8]
	str	x9, [x19, #1680]
	str	x8, [x19, #1688]
	str	x21, [x10, :lo12:all_flush_q_list]
	bl	_raw_spin_unlock_irqrestore
.LBB12_9:
	orr	w8, wzr, #0x1
	str	w8, [x19, #1696]
.LBB12_10:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldp	x22, x21, [sp], #48     // 16-byte Folded Reload
	ret
.LBB12_11:
	adrp	x0, .L.str.13
	adrp	x1, .L__func__.mas_blk_queue_async_flush_init
	add	x0, x0, :lo12:.L.str.13
	add	x1, x1, :lo12:.L__func__.mas_blk_queue_async_flush_init
	bl	printk
	b	.LBB12_10
.Lfunc_end12:
	.size	mas_blk_queue_async_flush_init, .Lfunc_end12-mas_blk_queue_async_flush_init
                                        // -- End function
	.globl	blk_flush_set_async     // -- Begin function blk_flush_set_async
	.p2align	2
	.type	blk_flush_set_async,@function
blk_flush_set_async:                    // @blk_flush_set_async
// %bb.0:
	cbz	x0, .LBB13_2
// %bb.1:
	orr	w8, wzr, #0x1
	strb	w8, [x0, #146]
.LBB13_2:
	ret
.Lfunc_end13:
	.size	blk_flush_set_async, .Lfunc_end13-blk_flush_set_async
                                        // -- End function
	.globl	blk_queue_direct_flush_register // -- Begin function blk_queue_direct_flush_register
	.p2align	2
	.type	blk_queue_direct_flush_register,@function
blk_queue_direct_flush_register:        // @blk_queue_direct_flush_register
// %bb.0:
	str	x21, [sp, #-48]!        // 8-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	cbz	x0, .LBB14_6
// %bb.1:
	mov	x20, x1
	mov	x19, x0
	bl	mas_blk_get_lld
	ldr	x8, [x19, #1680]
	add	x21, x19, #1680         // =1680
	cbnz	x8, .LBB14_3
// %bb.2:
	ldr	x8, [x19, #1688]
	cbz	x8, .LBB14_8
.LBB14_3:
	cbz	x20, .LBB14_9
.LBB14_4:
	str	x20, [x0, #56]
	adrp	x20, all_flush_q_lock
	add	x20, x20, :lo12:all_flush_q_lock
	mov	x0, x20
	bl	_raw_spin_lock_irqsave
	adrp	x8, all_flush_q_list
	add	x8, x8, :lo12:all_flush_q_list
	ldr	x9, [x8]
	adrp	x10, all_flush_q_list
	mov	x1, x0
	str	x21, [x9, #8]
	str	x9, [x19, #1680]
	str	x8, [x19, #1688]
	str	x21, [x10, :lo12:all_flush_q_list]
.LBB14_5:
	mov	x0, x20
	bl	_raw_spin_unlock_irqrestore
	b	.LBB14_7
.LBB14_6:
	adrp	x0, .L.str.14
	adrp	x1, .L__func__.blk_queue_direct_flush_register
	add	x0, x0, :lo12:.L.str.14
	add	x1, x1, :lo12:.L__func__.blk_queue_direct_flush_register
	bl	printk
.LBB14_7:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldr	x21, [sp], #48          // 8-byte Folded Reload
	ret
.LBB14_8:
	str	x21, [x19, #1680]
	str	x21, [x19, #1688]
	cbnz	x20, .LBB14_4
.LBB14_9:
	str	xzr, [x0, #56]
	ldr	w8, [x19, #1696]
	cbz	w8, .LBB14_7
// %bb.10:
	ldr	x8, [x19, #1680]
	cbz	x8, .LBB14_7
// %bb.11:
	ldr	x8, [x19, #1688]
	cbz	x8, .LBB14_7
// %bb.12:
	adrp	x20, all_flush_q_lock
	add	x20, x20, :lo12:all_flush_q_lock
	mov	x0, x20
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x19, #1688]
	ldr	x9, [x19, #1680]
	mov	x10, #256
	movk	x10, #57005, lsl #48
	mov	x1, x0
	add	x11, x10, #256          // =256
	str	x8, [x9, #8]
	str	x9, [x8]
	str	x10, [x19, #1680]
	str	x11, [x19, #1688]
	b	.LBB14_5
.Lfunc_end14:
	.size	blk_queue_direct_flush_register, .Lfunc_end14-blk_queue_direct_flush_register
                                        // -- End function
	.globl	blk_queue_flush_reduce_config // -- Begin function blk_queue_flush_reduce_config
	.p2align	2
	.type	blk_queue_flush_reduce_config,@function
blk_queue_flush_reduce_config:          // @blk_queue_flush_reduce_config
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	cbz	x0, .LBB15_2
// %bb.1:
	mov	w20, w1
	mov	x19, x0
	bl	mas_blk_get_lld
	ldr	x8, [x0, #24]
	tst	w20, #0x1
	and	x9, x8, #0xfffffffffffffffb
	orr	x8, x8, #0x4
	csel	x8, x8, x9, ne
	str	x8, [x0, #24]
	mov	x0, x19
	bl	mas_blk_queue_async_flush_init
	b	.LBB15_3
.LBB15_2:
	adrp	x0, .L.str.14
	adrp	x1, .L__func__.blk_queue_flush_reduce_config
	add	x0, x0, :lo12:.L.str.14
	add	x1, x1, :lo12:.L__func__.blk_queue_flush_reduce_config
	bl	printk
.LBB15_3:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end15:
	.size	blk_queue_flush_reduce_config, .Lfunc_end15-blk_queue_flush_reduce_config
                                        // -- End function
	.globl	blk_mq_tagset_flush_reduce_config // -- Begin function blk_mq_tagset_flush_reduce_config
	.p2align	2
	.type	blk_mq_tagset_flush_reduce_config,@function
blk_mq_tagset_flush_reduce_config:      // @blk_mq_tagset_flush_reduce_config
// %bb.0:
	cbz	x0, .LBB16_3
// %bb.1:
	ldr	x8, [x0, #40]
	tbz	w1, #0, .LBB16_4
// %bb.2:
	orr	x8, x8, #0x4
	str	x8, [x0, #40]
	ret
.LBB16_3:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	adrp	x0, .L.str.11
	adrp	x1, .L__func__.blk_mq_tagset_flush_reduce_config
	add	x0, x0, :lo12:.L.str.11
	add	x1, x1, :lo12:.L__func__.blk_mq_tagset_flush_reduce_config
	mov	x29, sp
	bl	printk
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.LBB16_4:
	and	x8, x8, #0xfffffffffffffffb
	str	x8, [x0, #40]
	ret
.Lfunc_end16:
	.size	blk_mq_tagset_flush_reduce_config, .Lfunc_end16-blk_mq_tagset_flush_reduce_config
                                        // -- End function
	.globl	blk_flush_async_support // -- Begin function blk_flush_async_support
	.p2align	2
	.type	blk_flush_async_support,@function
blk_flush_async_support:                // @blk_flush_async_support
// %bb.0:
	cbz	x0, .LBB17_3
// %bb.1:
	ldr	x8, [x0, #160]
	ldr	x8, [x8, #1120]
	cbz	x8, .LBB17_4
// %bb.2:
	ldr	w0, [x8, #1696]
	ret
.LBB17_3:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	adrp	x0, .L.str.15
	adrp	x1, .L__func__.blk_flush_async_support
	add	x0, x0, :lo12:.L.str.15
	add	x1, x1, :lo12:.L__func__.blk_flush_async_support
	mov	x29, sp
	bl	printk
	mov	w0, wzr
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.LBB17_4:
	mov	w0, wzr
	ret
.Lfunc_end17:
	.size	blk_flush_async_support, .Lfunc_end17-blk_flush_async_support
                                        // -- End function
	.type	.L.str,@object          // @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	"\0013[BLK-IO]%s: blkdev_get fail!\n"
	.size	.L.str, 32

	.type	.L__func__.mas_blk_flush_work_fn,@object // @__func__.mas_blk_flush_work_fn
.L__func__.mas_blk_flush_work_fn:
	.asciz	"mas_blk_flush_work_fn"
	.size	.L__func__.mas_blk_flush_work_fn, 22

	.type	.L.str.1,@object        // @.str.1
.L.str.1:
	.asciz	"\0013[BLK-IO]%s: blkdev_issue_flush fail!\n"
	.size	.L.str.1, 40

	.type	blk_async_flush_workqueue,@object // @blk_async_flush_workqueue
	.local	blk_async_flush_workqueue
	.comm	blk_async_flush_workqueue,8,8
	.type	.L.str.2,@object        // @.str.2
.L.str.2:
	.asciz	"\0013[BLK-IO]%s: emergency = %d\n"
	.size	.L.str.2, 30

	.type	.L__func__.blk_power_off_flush,@object // @__func__.blk_power_off_flush
.L__func__.blk_power_off_flush:
	.asciz	"blk_power_off_flush"
	.size	.L__func__.blk_power_off_flush, 20

	.type	all_flush_q_lock,@object // @all_flush_q_lock
	.data
	.p2align	3
all_flush_q_lock:
	.zero	4
	.word	3735899821              // 0xdead4ead
	.word	4294967295              // 0xffffffff
	.zero	4
	.xword	-1
	.size	all_flush_q_lock, 24

	.type	all_flush_q_list,@object // @all_flush_q_list
	.p2align	3
all_flush_q_list:
	.xword	all_flush_q_list
	.xword	all_flush_q_list
	.size	all_flush_q_list, 16

	.type	.L.str.3,@object        // @.str.3
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.3:
	.asciz	"\0013[BLK-IO]%s: emergency flush on %s\n"
	.size	.L.str.3, 37

	.type	.L.str.4,@object        // @.str.4
.L.str.4:
	.asciz	"\0013[BLK-IO]%s: emergency flush on %s done!\n"
	.size	.L.str.4, 43

	.type	.L.str.5,@object        // @.str.5
.L.str.5:
	.asciz	"\0013[BLK-IO]%s: done!\n"
	.size	.L.str.5, 21

	.type	all_object_lock,@object // @all_object_lock
	.data
	.p2align	3
all_object_lock:
	.zero	4
	.word	3735899821              // 0xdead4ead
	.word	4294967295              // 0xffffffff
	.zero	4
	.xword	-1
	.size	all_object_lock, 24

	.type	all_object_list,@object // @all_object_list
	.p2align	3
all_object_list:
	.xword	all_object_list
	.xword	all_object_list
	.size	all_object_list, 16

	.type	.L.str.6,@object        // @.str.6
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.6:
	.asciz	"\0013[BLK-IO]<%s> start\n"
	.size	.L.str.6, 22

	.type	.L__func__.mas_blk_order_panic_wait_flush,@object // @__func__.mas_blk_order_panic_wait_flush
.L__func__.mas_blk_order_panic_wait_flush:
	.asciz	"mas_blk_order_panic_wait_flush"
	.size	.L__func__.mas_blk_order_panic_wait_flush, 31

	.type	.L.str.7,@object        // @.str.7
.L.str.7:
	.asciz	"\0013[BLK-IO]<%s> type = %d\n"
	.size	.L.str.7, 26

	.type	.L.str.8,@object        // @.str.8
.L.str.8:
	.asciz	"\0013[BLK-IO]<%s> normal end\n"
	.size	.L.str.8, 27

	.type	.L.str.9,@object        // @.str.9
.L.str.9:
	.asciz	"\0013[BLK-IO]<%s> timeout end\n"
	.size	.L.str.9, 28

	.type	.L__func__.mas_blk_panic_flush,@object // @__func__.mas_blk_panic_flush
.L__func__.mas_blk_panic_flush:
	.asciz	"mas_blk_panic_flush"
	.size	.L__func__.mas_blk_panic_flush, 20

	.type	__kstrtab_mas_blk_panic_flush,@object // @__kstrtab_mas_blk_panic_flush
	.section	__ksymtab_strings,"a",@progbits
__kstrtab_mas_blk_panic_flush:
	.asciz	"mas_blk_panic_flush"
	.size	__kstrtab_mas_blk_panic_flush, 20

	.type	__ksymtab_mas_blk_panic_flush,@object // @__ksymtab_mas_blk_panic_flush
	.section	"___ksymtab+mas_blk_panic_flush","a",@progbits
	.p2align	3
__ksymtab_mas_blk_panic_flush:
	.xword	mas_blk_panic_flush
	.xword	__kstrtab_mas_blk_panic_flush
	.size	__ksymtab_mas_blk_panic_flush, 16

	.type	mas_blk_poweroff_flush_nb,@object // @mas_blk_poweroff_flush_nb
	.data
	.p2align	3
mas_blk_poweroff_flush_nb:
	.xword	__cfi_mas_blk_poweroff_flush_notifier_call
	.xword	0
	.word	0                       // 0x0
	.zero	4
	.size	mas_blk_poweroff_flush_nb, 24

	.type	__initcall___cfi_mas_blk_flush_init6,@object // @__initcall___cfi_mas_blk_flush_init6
	.section	.initcall6.init,"aw",@progbits
	.p2align	3
__initcall___cfi_mas_blk_flush_init6:
	.xword	__cfi_mas_blk_flush_init
	.size	__initcall___cfi_mas_blk_flush_init6, 8

	.type	.L.str.11,@object       // @.str.11
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.11:
	.asciz	"\0013[BLK-IO]%s: tag_set is NULL!\n"
	.size	.L.str.11, 32

	.type	.L__func__.blk_mq_tagset_direct_flush_register,@object // @__func__.blk_mq_tagset_direct_flush_register
.L__func__.blk_mq_tagset_direct_flush_register:
	.asciz	"blk_mq_tagset_direct_flush_register"
	.size	.L__func__.blk_mq_tagset_direct_flush_register, 36

	.type	.L.str.12,@object       // @.str.12
.L.str.12:
	.asciz	"async_flush"
	.size	.L.str.12, 12

	.type	.L.str.13,@object       // @.str.13
.L.str.13:
	.asciz	"\0013[BLK-IO]%s: Failed to alloc workqueue!\n"
	.size	.L.str.13, 42

	.type	.L__func__.mas_blk_queue_async_flush_init,@object // @__func__.mas_blk_queue_async_flush_init
.L__func__.mas_blk_queue_async_flush_init:
	.asciz	"mas_blk_queue_async_flush_init"
	.size	.L__func__.mas_blk_queue_async_flush_init, 31

	.type	.L.str.14,@object       // @.str.14
.L.str.14:
	.asciz	"\0013[BLK-IO]%s: q is NULL!\n"
	.size	.L.str.14, 26

	.type	.L__func__.blk_queue_direct_flush_register,@object // @__func__.blk_queue_direct_flush_register
.L__func__.blk_queue_direct_flush_register:
	.asciz	"blk_queue_direct_flush_register"
	.size	.L__func__.blk_queue_direct_flush_register, 32

	.type	.L__func__.blk_queue_flush_reduce_config,@object // @__func__.blk_queue_flush_reduce_config
.L__func__.blk_queue_flush_reduce_config:
	.asciz	"blk_queue_flush_reduce_config"
	.size	.L__func__.blk_queue_flush_reduce_config, 30

	.type	.L__func__.blk_mq_tagset_flush_reduce_config,@object // @__func__.blk_mq_tagset_flush_reduce_config
.L__func__.blk_mq_tagset_flush_reduce_config:
	.asciz	"blk_mq_tagset_flush_reduce_config"
	.size	.L__func__.blk_mq_tagset_flush_reduce_config, 34

	.type	.L.str.15,@object       // @.str.15
.L.str.15:
	.asciz	"\0013[BLK-IO]%s: bi_bdev is NULL!\n"
	.size	.L.str.15, 32

	.type	.L__func__.blk_flush_async_support,@object // @__func__.blk_flush_async_support
.L__func__.blk_flush_async_support:
	.asciz	"blk_flush_async_support"
	.size	.L__func__.blk_flush_async_support, 24

	.type	.L__func__.mas_blk_order_poweroff_proc,@object // @__func__.mas_blk_order_poweroff_proc
.L__func__.mas_blk_order_poweroff_proc:
	.asciz	"mas_blk_order_poweroff_proc"
	.size	.L__func__.mas_blk_order_poweroff_proc, 28


	.ident	"Android (27847 based on r353983c) clang version 9.0.3 (https://android.googlesource.com/toolchain/clang dbdb7a2acf33dc76cd6f506c323253b8d892ae05) (https://android.googlesource.com/toolchain/llvm f32df87f6769ededcd9e2a6c3c4c45c1baf361c2) (based on LLVM 9.0.3svn)"
	.section	".note.GNU-stack","",@progbits

	.text
	.file	"mas_blk_iosched_ufs_mq.c"
	.globl	ufs_mq_inc_vip_wait_cnt // -- Begin function ufs_mq_inc_vip_wait_cnt
	.p2align	2
	.type	ufs_mq_inc_vip_wait_cnt,@function
ufs_mq_inc_vip_wait_cnt:                // @ufs_mq_inc_vip_wait_cnt
// %bb.0:
	ldr	x8, [x0]
	ldr	x8, [x8, #1808]
	cbz	x8, .LBB0_3
// %bb.1:
	ldr	x8, [x8, #280]
	cbz	x8, .LBB0_3
// %bb.2:
	add	x8, x8, #28             // =28
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
.LBB0_3:
	ret
.Lfunc_end0:
	.size	ufs_mq_inc_vip_wait_cnt, .Lfunc_end0-ufs_mq_inc_vip_wait_cnt
                                        // -- End function
	.globl	ufs_mq_dec_vip_wait_cnt // -- Begin function ufs_mq_dec_vip_wait_cnt
	.p2align	2
	.type	ufs_mq_dec_vip_wait_cnt,@function
ufs_mq_dec_vip_wait_cnt:                // @ufs_mq_dec_vip_wait_cnt
// %bb.0:
	ldr	x8, [x0]
	ldr	x8, [x8, #1808]
	cbz	x8, .LBB1_2
// %bb.1:
	ldr	x8, [x8, #280]
.LBB1_2:
	ldr	w9, [x8, #28]
.LBB1_3:                                // =>This Inner Loop Header: Depth=1
	subs	w10, w9, #1             // =1
	b.mi	.LBB1_5
// %bb.4:                               //   in Loop: Header=BB1_3 Depth=1
	sxtw	x11, w9
	sxtw	x10, w10
	add	x12, x8, #28            // =28
	//APP
		prfm	pstl1strm, [x12]
1:	ldxr	w14, [x12]
	eor	w13, w14, w11
	cbnz	w13, 2f
	stlxr	w13, w10, [x12]
	cbnz	w13, 1b
	dmb ish
2:
	//NO_APP
	cmp	w9, w14
	mov	w9, w14
	b.ne	.LBB1_3
.LBB1_5:
	ret
.Lfunc_end1:
	.size	ufs_mq_dec_vip_wait_cnt, .Lfunc_end1-ufs_mq_dec_vip_wait_cnt
                                        // -- End function
	.globl	reset_vip_wait_cnt      // -- Begin function reset_vip_wait_cnt
	.p2align	2
	.type	reset_vip_wait_cnt,@function
reset_vip_wait_cnt:                     // @reset_vip_wait_cnt
// %bb.0:
	ldr	x8, [x0]
	ldr	x8, [x8, #1808]
	cbz	x8, .LBB2_2
// %bb.1:
	ldr	x8, [x8, #280]
.LBB2_2:
	ldr	w9, [x8, #24]
	cbz	w9, .LBB2_4
// %bb.3:
	ret
.LBB2_4:
	str	wzr, [x8, #28]
	ret
.Lfunc_end2:
	.size	reset_vip_wait_cnt, .Lfunc_end2-reset_vip_wait_cnt
                                        // -- End function
	.globl	ufs_mq_vip_tag_wait_cnt // -- Begin function ufs_mq_vip_tag_wait_cnt
	.p2align	2
	.type	ufs_mq_vip_tag_wait_cnt,@function
ufs_mq_vip_tag_wait_cnt:                // @ufs_mq_vip_tag_wait_cnt
// %bb.0:
	ldr	x8, [x0]
	ldr	x8, [x8, #1808]
	cbz	x8, .LBB3_2
// %bb.1:
	ldr	x8, [x8, #280]
.LBB3_2:
	ldr	w0, [x8, #28]
	ret
.Lfunc_end3:
	.size	ufs_mq_vip_tag_wait_cnt, .Lfunc_end3-ufs_mq_vip_tag_wait_cnt
                                        // -- End function
	.globl	get_mq_all_tag_used     // -- Begin function get_mq_all_tag_used
	.p2align	2
	.type	get_mq_all_tag_used,@function
get_mq_all_tag_used:                    // @get_mq_all_tag_used
// %bb.0:
	cbz	x0, .LBB4_4
// %bb.1:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	mov	x29, sp
	bl	mas_blk_get_lld
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	cbz	x0, .LBB4_4
// %bb.2:
	ldr	x8, [x0, #1680]
	cbz	x8, .LBB4_5
// %bb.3:
	ldr	w9, [x8, #24]
	ldr	w10, [x8, #16]
	ldr	w8, [x8, #20]
	add	w9, w10, w9
	add	w0, w9, w8
.LBB4_4:
	ret
.LBB4_5:
	mov	w0, wzr
	ret
.Lfunc_end4:
	.size	get_mq_all_tag_used, .Lfunc_end4-get_mq_all_tag_used
                                        // -- End function
	.globl	get_mq_prio_tag_used    // -- Begin function get_mq_prio_tag_used
	.p2align	2
	.type	get_mq_prio_tag_used,@function
get_mq_prio_tag_used:                   // @get_mq_prio_tag_used
// %bb.0:
	cbz	x0, .LBB5_4
// %bb.1:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	mov	x29, sp
	bl	mas_blk_get_lld
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	cbz	x0, .LBB5_4
// %bb.2:
	ldr	x8, [x0, #1680]
	cbz	x8, .LBB5_5
// %bb.3:
	ldr	w0, [x8, #24]
.LBB5_4:
	ret
.LBB5_5:
	mov	w0, wzr
	ret
.Lfunc_end5:
	.size	get_mq_prio_tag_used, .Lfunc_end5-get_mq_prio_tag_used
                                        // -- End function
	.globl	ufs_mq_tag_get          // -- Begin function ufs_mq_tag_get
	.p2align	2
	.type	ufs_mq_tag_get,@function
ufs_mq_tag_get:                         // @ufs_mq_tag_get
// %bb.0:
	str	x21, [sp, #-48]!        // 8-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	mov	x19, x0
	ldr	x0, [x0]
	ldr	x21, [x19, #8]
	mov	w20, #4096
	add	x29, sp, #32            // =32
	movk	w20, #64, lsl #16
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB6_8
// %bb.1:
	ldr	x8, [x19, #16]
	tbnz	w8, #2, .LBB6_10
// %bb.2:
	ldr	x9, [x19, #8]
	and	x8, x8, #0x2
	and	x10, x9, x20
	orr	x8, x8, x10
	cbz	x8, .LBB6_22
// %bb.3:
	ldr	x2, [x19, #56]
	ldr	x8, [x2, #304]
	ldr	w9, [x8, #8]
	cbz	w9, .LBB6_42
// %bb.4:
	add	x1, x8, #152            // =152
	mov	x0, x19
	bl	ufs_tagset_bt_get
	cmn	w0, #1                  // =1
	b.eq	.LBB6_30
// %bb.5:
	ldr	x8, [x19]
	ldr	x8, [x8, #1808]
	cbz	x8, .LBB6_7
// %bb.6:
	ldr	x8, [x8, #280]
.LBB6_7:
	add	x8, x8, #24             // =24
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	b	.LBB6_21
.LBB6_8:
	add	x8, x20, #2048          // =2048
	and	x8, x21, x8
	cmp	x8, #2048               // =2048
	b.ne	.LBB6_31
.LBB6_9:
	mov	x0, x19
	bl	ufs_tagset_get_tag
	b	.LBB6_30
.LBB6_10:
	ldr	x2, [x19, #56]
	ldr	x9, [x2, #304]
	tbnz	w8, #1, .LBB6_16
// %bb.11:
	ldr	w8, [x9, #4]
	cbz	w8, .LBB6_42
// %bb.12:
	add	x1, x9, #96             // =96
	mov	x0, x19
	bl	ufs_tagset_bt_get
	cmn	w0, #1                  // =1
	b.eq	.LBB6_30
// %bb.13:
	ldr	x8, [x19]
	ldr	x8, [x8, #1808]
	cbz	x8, .LBB6_15
// %bb.14:
	ldr	x8, [x8, #280]
.LBB6_15:
	add	x8, x8, #20             // =20
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	b	.LBB6_28
.LBB6_16:
	ldr	w8, [x9, #8]
	cbz	w8, .LBB6_42
// %bb.17:
	add	x1, x9, #152            // =152
	mov	x0, x19
	bl	ufs_tagset_bt_get
	cmn	w0, #1                  // =1
	b.eq	.LBB6_30
// %bb.18:
	ldr	x8, [x19]
	ldr	x8, [x8, #1808]
	cbz	x8, .LBB6_20
// %bb.19:
	ldr	x8, [x8, #280]
.LBB6_20:
	add	x8, x8, #24             // =24
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
.LBB6_21:
	ldr	x8, [x19, #56]
	ldr	x8, [x8, #304]
	ldr	w8, [x8, #20]
	b	.LBB6_29
.LBB6_22:
	add	x8, x20, #2048          // =2048
	and	x8, x9, x8
	cmp	x8, #2048               // =2048
	b.eq	.LBB6_9
// %bb.23:
	ldr	x2, [x19, #56]
	ldr	x8, [x2, #304]
	ldr	w9, [x8, #4]
	cbz	w9, .LBB6_42
// %bb.24:
	add	x1, x8, #96             // =96
	mov	x0, x19
	bl	ufs_tagset_bt_get
	cmn	w0, #1                  // =1
	b.eq	.LBB6_30
// %bb.25:
	ldr	x8, [x19]
	ldr	x8, [x8, #1808]
	cbz	x8, .LBB6_27
// %bb.26:
	ldr	x8, [x8, #280]
.LBB6_27:
	add	x8, x8, #20             // =20
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
.LBB6_28:
	ldr	x8, [x19, #56]
	ldr	x8, [x8, #304]
	ldr	w8, [x8, #16]
.LBB6_29:
	add	w0, w8, w0
.LBB6_30:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldr	x21, [sp], #48          // 8-byte Folded Reload
	ret
.LBB6_31:
	ldr	x2, [x19, #56]
	tst	x21, x20
	ldr	x8, [x2, #304]
	b.eq	.LBB6_37
// %bb.32:
	ldr	w9, [x8, #8]
	cbz	w9, .LBB6_42
// %bb.33:
	add	x1, x8, #152            // =152
	mov	x0, x19
	bl	ufs_tagset_bt_get
	cmn	w0, #1                  // =1
	b.eq	.LBB6_30
// %bb.34:
	ldr	x8, [x19]
	ldr	x8, [x8, #1808]
	cbz	x8, .LBB6_36
// %bb.35:
	ldr	x8, [x8, #280]
.LBB6_36:
	add	x8, x8, #24             // =24
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	b	.LBB6_21
.LBB6_37:
	ldr	w9, [x8, #4]
	cbz	w9, .LBB6_42
// %bb.38:
	add	x1, x8, #96             // =96
	mov	x0, x19
	bl	ufs_tagset_bt_get
	cmn	w0, #1                  // =1
	b.eq	.LBB6_30
// %bb.39:
	ldr	x8, [x19]
	ldr	x8, [x8, #1808]
	cbz	x8, .LBB6_41
// %bb.40:
	ldr	x8, [x8, #280]
.LBB6_41:
	add	x8, x8, #20             // =20
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	b	.LBB6_28
.LBB6_42:
	mov	w0, #-1
	b	.LBB6_30
.Lfunc_end6:
	.size	ufs_mq_tag_get, .Lfunc_end6-ufs_mq_tag_get
                                        // -- End function
	.p2align	2               // -- Begin function ufs_tagset_get_tag
	.type	ufs_tagset_get_tag,@function
ufs_tagset_get_tag:                     // @ufs_tagset_get_tag
// %bb.0:
	sub	sp, sp, #48             // =48
	adrp	x8, __stack_chk_guard
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	str	x19, [sp, #16]          // 8-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	mov	x19, x0
	str	x8, [sp, #8]
	ldr	x0, [x0]
	add	x29, sp, #32            // =32
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB7_7
// %bb.1:
	ldr	x8, [x19, #24]
	ldrb	w8, [x8, #16]
	cmp	w8, #1                  // =1
	b.ne	.LBB7_7
// %bb.2:
	add	x1, sp, #4              // =4
	mov	x0, x19
	strb	wzr, [sp, #4]
	bl	ufs_tagset_bt_get_unistore
	cmn	w0, #1                  // =1
	b.eq	.LBB7_11
// %bb.3:
	ldr	x8, [x19]
	ldr	x8, [x8, #1808]
	cbz	x8, .LBB7_13
// %bb.4:
	ldr	x8, [x8, #280]
	cbz	x8, .LBB7_13
// %bb.5:
	ldrb	w9, [sp, #4]
	cbz	w9, .LBB7_14
// %bb.6:
	add	x8, x8, #24             // =24
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	ldr	x8, [x19, #56]
	ldr	x8, [x8, #304]
	ldr	w8, [x8, #20]
	add	w0, w8, w0
	b	.LBB7_11
.LBB7_7:
	ldr	x2, [x19, #56]
	mov	x0, x19
	ldr	x8, [x2, #304]
	add	x1, x8, #40             // =40
	bl	ufs_tagset_bt_get
	cmn	w0, #1                  // =1
	b.eq	.LBB7_11
// %bb.8:
	ldr	x8, [x19]
	ldr	x8, [x8, #1808]
	cbz	x8, .LBB7_10
// %bb.9:
	ldr	x8, [x8, #280]
.LBB7_10:
	add	x8, x8, #16             // =16
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
.LBB7_11:
	adrp	x9, __stack_chk_guard
	ldr	x8, [sp, #8]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB7_15
// %bb.12:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldr	x19, [sp, #16]          // 8-byte Folded Reload
	add	sp, sp, #48             // =48
	ret
.LBB7_13:
	mov	w0, #-1
	b	.LBB7_11
.LBB7_14:
	add	x8, x8, #16             // =16
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	b	.LBB7_11
.LBB7_15:
	bl	__stack_chk_fail
.Lfunc_end7:
	.size	ufs_tagset_get_tag, .Lfunc_end7-ufs_tagset_get_tag
                                        // -- End function
	.globl	ufs_mq_tag_put          // -- Begin function ufs_mq_tag_put
	.p2align	2
	.type	ufs_mq_tag_put,@function
ufs_mq_tag_put:                         // @ufs_mq_tag_put
// %bb.0:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	ldr	x8, [x0, #304]
	mov	x19, x0
	add	x29, sp, #16            // =16
	ldr	w9, [x8, #20]
	cmp	w9, w1
	b.ls	.LBB8_6
// %bb.1:
	ldr	w9, [x8, #16]
	cmp	w9, w1
	b.ls	.LBB8_9
// %bb.2:
	ldr	x9, [x2, #288]
	add	x0, x8, #40             // =40
	ldr	w2, [x9, #64]
	bl	sbitmap_queue_clear
	ldr	x8, [x19, #192]
	ldr	x8, [x8, #1808]
	cbz	x8, .LBB8_4
// %bb.3:
	ldr	x8, [x8, #280]
.LBB8_4:
	add	x8, x8, #16             // =16
	//APP
	// atomic_sub
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
.LBB8_5:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	mov	w0, wzr
	ldr	x19, [sp], #32          // 8-byte Folded Reload
	ret
.LBB8_6:
	ldr	x10, [x2, #288]
	add	x0, x8, #152            // =152
	sub	w1, w1, w9
	ldr	w2, [x10, #64]
	bl	sbitmap_queue_clear
	ldr	x8, [x19, #192]
	ldr	x8, [x8, #1808]
	cbz	x8, .LBB8_8
// %bb.7:
	ldr	x8, [x8, #280]
.LBB8_8:
	add	x8, x8, #24             // =24
	//APP
	// atomic_sub
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	b	.LBB8_5
.LBB8_9:
	ldr	x10, [x2, #288]
	add	x0, x8, #96             // =96
	sub	w1, w1, w9
	ldr	w2, [x10, #64]
	bl	sbitmap_queue_clear
	ldr	x8, [x19, #192]
	ldr	x8, [x8, #1808]
	cbz	x8, .LBB8_11
// %bb.10:
	ldr	x8, [x8, #280]
.LBB8_11:
	add	x8, x8, #20             // =20
	//APP
	// atomic_sub
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	b	.LBB8_5
.Lfunc_end8:
	.size	ufs_mq_tag_put, .Lfunc_end8-ufs_mq_tag_put
                                        // -- End function
	.globl	ufs_mq_sync_burst_check_timer_expire // -- Begin function ufs_mq_sync_burst_check_timer_expire
	.p2align	2
	.type	ufs_mq_sync_burst_check_timer_expire,@function
ufs_mq_sync_burst_check_timer_expire:   // @ufs_mq_sync_burst_check_timer_expire
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	mov	x19, x0
	bl	ktime_get
	ldr	w8, [x19, #312]
	sub	w8, w8, #1              // =1
	cmp	w8, #1                  // =1
	b.hi	.LBB9_3
// %bb.1:
	ldr	x8, [x19, #232]
	mov	w9, #25856
	movk	w9, #7629, lsl #16
	add	x8, x8, x9
	cmp	x8, x0
	b.ge	.LBB9_3
// %bb.2:
	orr	w8, wzr, #0x3
	str	w8, [x19, #312]
.LBB9_3:
	ldr	x8, [x19, #224]
	mov	w20, #19264
	movk	w20, #76, lsl #16
	add	x8, x8, x20
	cmp	x8, x0
	b.le	.LBB9_7
// %bb.4:
	ldr	w8, [x19, #312]
	cmp	w8, #3                  // =3
	b.eq	.LBB9_12
// %bb.5:
	cmp	w8, #1                  // =1
	b.ne	.LBB9_16
// %bb.6:
	orr	w8, wzr, #0x2
	b	.LBB9_15
.LBB9_7:
	bl	ktime_get
	ldr	w8, [x19, #312]
	cmp	w8, #3                  // =3
	b.eq	.LBB9_10
// %bb.8:
	cmp	w8, #2                  // =2
	b.eq	.LBB9_13
// %bb.9:
	cmp	w8, #1                  // =1
	b.ne	.LBB9_16
.LBB9_10:
	ldr	x8, [x19, #240]
	add	x8, x8, x20
	cmp	x8, x0
	b.ge	.LBB9_16
// %bb.11:
	str	wzr, [x19, #312]
	b	.LBB9_17
.LBB9_12:
	ldr	w8, [x19, #20]
	cbnz	w8, .LBB9_16
	b	.LBB9_14
.LBB9_13:
	ldr	x8, [x19, #224]
	add	x8, x8, x20
	cmp	x8, x0
	b.ge	.LBB9_16
.LBB9_14:
	orr	w8, wzr, #0x1
.LBB9_15:
	str	w8, [x19, #312]
.LBB9_16:
	adrp	x8, jiffies
	ldr	x8, [x8, :lo12:jiffies]
	add	x0, x19, #264           // =264
	add	x1, x8, #2              // =2
	bl	mod_timer
.LBB9_17:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end9:
	.size	ufs_mq_sync_burst_check_timer_expire, .Lfunc_end9-ufs_mq_sync_burst_check_timer_expire
                                        // -- End function
	.globl	ufs_mq_dump_request     // -- Begin function ufs_mq_dump_request
	.p2align	2
	.type	ufs_mq_dump_request,@function
ufs_mq_dump_request:                    // @ufs_mq_dump_request
// %bb.0:
	stp	x24, x23, [sp, #-64]!   // 16-byte Folded Spill
	stp	x22, x21, [sp, #16]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	ldr	x22, [x0, #1808]
	add	x29, sp, #48            // =48
	cbz	x22, .LBB10_24
// %bb.1:
	ldr	x23, [x22, #280]
	cbz	x23, .LBB10_24
// %bb.2:
	mov	x24, x23
	ldr	x8, [x24, #72]!
	mov	x19, x0
	mov	w20, w1
	cmp	x24, x8
	b.eq	.LBB10_7
// %bb.3:
	adrp	x0, .L.str
	add	x0, x0, :lo12:.L.str
	bl	printk
	ldr	x21, [x24]
	cmp	x21, x24
	b.ne	.LBB10_5
	b	.LBB10_7
.LBB10_4:                               //   in Loop: Header=BB10_5 Depth=1
	ldr	x21, [x21]
	cmp	x21, x24
	b.eq	.LBB10_7
.LBB10_5:                               // =>This Inner Loop Header: Depth=1
	ldr	x8, [x21, #64]
	cmp	x8, x19
	b.ne	.LBB10_4
// %bb.6:                               //   in Loop: Header=BB10_5 Depth=1
	mov	x0, x21
	mov	w1, w20
	bl	mas_blk_dump_request
	b	.LBB10_4
.LBB10_7:
	mov	x24, x23
	ldr	x8, [x24, #96]!
	cmp	x24, x8
	b.eq	.LBB10_13
// %bb.8:
	adrp	x0, .L.str.1
	add	x0, x0, :lo12:.L.str.1
	bl	printk
	ldr	x21, [x24]
	cmp	x21, x24
	b.ne	.LBB10_10
	b	.LBB10_12
.LBB10_9:                               //   in Loop: Header=BB10_10 Depth=1
	ldr	x21, [x21]
	cmp	x21, x24
	b.eq	.LBB10_12
.LBB10_10:                              // =>This Inner Loop Header: Depth=1
	ldr	x8, [x21, #64]
	cmp	x8, x19
	b.ne	.LBB10_9
// %bb.11:                              //   in Loop: Header=BB10_10 Depth=1
	mov	x0, x21
	mov	w1, w20
	bl	mas_blk_dump_request
	b	.LBB10_9
.LBB10_12:
	ldp	x1, x2, [x22, #8]
	ldr	x3, [x22, #24]
	adrp	x0, .L.str.9
	add	x0, x0, :lo12:.L.str.9
	bl	printk
.LBB10_13:
	ldr	x8, [x23, #152]!
	cmp	x23, x8
	b.eq	.LBB10_24
// %bb.14:
	adrp	x0, .L.str.2
	add	x0, x0, :lo12:.L.str.2
	bl	printk
	mov	x0, x19
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB10_19
// %bb.15:
	ldr	x21, [x23]
	cmp	x21, x23
	b.ne	.LBB10_17
	b	.LBB10_23
.LBB10_16:                              //   in Loop: Header=BB10_17 Depth=1
	ldr	x21, [x21]
	cmp	x21, x23
	b.eq	.LBB10_23
.LBB10_17:                              // =>This Inner Loop Header: Depth=1
	ldr	x8, [x21, #64]
	cmp	x8, x19
	b.ne	.LBB10_16
// %bb.18:                              //   in Loop: Header=BB10_17 Depth=1
	mov	x0, x21
	mov	w1, w20
	bl	mas_blk_dump_request
	b	.LBB10_16
.LBB10_19:
	ldr	x21, [x23]
	cmp	x23, x21
	b.ne	.LBB10_21
	b	.LBB10_23
.LBB10_20:                              //   in Loop: Header=BB10_21 Depth=1
	ldr	x21, [x21]
	cmp	x23, x21
	b.eq	.LBB10_23
.LBB10_21:                              // =>This Inner Loop Header: Depth=1
	sub	x8, x21, #408           // =408
	ldr	x8, [x8]
	cmp	x8, x19
	b.ne	.LBB10_20
// %bb.22:                              //   in Loop: Header=BB10_21 Depth=1
	sub	x0, x21, #472           // =472
	mov	w1, w20
	bl	mas_blk_dump_request
	b	.LBB10_20
.LBB10_23:
	ldp	x1, x2, [x22, #136]
	ldr	x3, [x22, #152]
	adrp	x0, .L.str.9
	add	x0, x0, :lo12:.L.str.9
	bl	printk
.LBB10_24:
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]     // 16-byte Folded Reload
	ldp	x24, x23, [sp], #64     // 16-byte Folded Reload
	ret
.Lfunc_end10:
	.size	ufs_mq_dump_request, .Lfunc_end10-ufs_mq_dump_request
                                        // -- End function
	.globl	ufs_mq_flush_plug_list  // -- Begin function ufs_mq_flush_plug_list
	.p2align	2
	.type	ufs_mq_flush_plug_list,@function
ufs_mq_flush_plug_list:                 // @ufs_mq_flush_plug_list
// %bb.0:
	sub	sp, sp, #128            // =128
	adrp	x8, .L__const.ufs_mq_flush_plug_list.bd
	adrp	x9, __stack_chk_guard
	add	x8, x8, :lo12:.L__const.ufs_mq_flush_plug_list.bd
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	ldp	x11, x10, [x8]
	add	x8, sp, #8              // =8
	stp	x26, x25, [sp, #48]     // 16-byte Folded Spill
	stp	x24, x23, [sp, #64]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #80]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #96]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #112]    // 16-byte Folded Spill
	str	x9, [sp, #40]
	stp	x11, x10, [sp, #24]
	stp	x8, x8, [sp, #8]
	mov	x9, x0
	ldr	x10, [x9, #48]!
	add	x29, sp, #112           // =112
	cmp	x9, x10
	b.eq	.LBB11_2
// %bb.1:
	ldr	x11, [x0, #56]
	str	x8, [x10, #8]
	str	x10, [sp, #8]
	str	x8, [x11]
	str	x11, [sp, #16]
	str	x9, [x0, #48]
	str	x9, [x0, #56]
	ldr	x19, [sp, #8]
	b	.LBB11_3
.LBB11_2:
	add	x19, sp, #8             // =8
.LBB11_3:
	adrp	x23, cpu_number
	adrp	x24, __per_cpu_offset
	add	x23, x23, :lo12:cpu_number
	add	x24, x24, :lo12:__per_cpu_offset
	add	x25, sp, #8             // =8
	adrp	x26, mas_blk_mq_sync_disp_wq
	//APP
	mrs x22, sp_el0
	//NO_APP
	//APP
	//NO_APP
	b	.LBB11_8
.LBB11_4:                               //   in Loop: Header=BB11_8 Depth=1
	bl	preempt_schedule
	cbz	w21, .LBB11_14
.LBB11_5:                               //   in Loop: Header=BB11_8 Depth=1
	cmp	w21, #9                 // =9
	b.ne	.LBB11_7
// %bb.6:                               //   in Loop: Header=BB11_8 Depth=1
	mov	x0, x19
	bl	__blk_mq_requeue_request
	mov	x0, x19
	mov	x1, x20
	bl	ufs_mq_insert_sync_list
	ldr	x19, [x20, #1808]
	ldr	x1, [x26, :lo12:mas_blk_mq_sync_disp_wq]
	orr	w0, wzr, #0x8
	mov	x3, xzr
	add	x2, x19, #32            // =32
	bl	queue_delayed_work_on
	bl	ktime_get
	str	x0, [x19, #8]
	b	.LBB11_14
.LBB11_7:                               //   in Loop: Header=BB11_8 Depth=1
	mov	w1, #10
	mov	x0, x19
	bl	blk_mq_end_request
	b	.LBB11_14
.LBB11_8:                               // =>This Inner Loop Header: Depth=1
	ldr	x20, [x19, #64]
	ldp	x8, x9, [x19]
	add	x3, sp, #24             // =24
	mov	x0, x19
	ldr	x10, [x20, #1808]
	str	x9, [x8, #8]
	str	x8, [x9]
	str	x19, [x19]
	str	x19, [x19, #8]
	str	x19, [sp, #24]
	ldr	w8, [x22, #16]
	mov	x4, x20
	add	w8, w8, #1              // =1
	str	w8, [x22, #16]
	//APP
	//NO_APP
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
	ldr	x9, [x20, #328]
	//APP
	//NO_APP
	ldr	x8, [x24, x8, lsl #3]
	add	x8, x8, x9
	str	x8, [x19, #72]
	ldrsw	x8, [x8, #64]
	ldr	x9, [x20, #320]
	ldr	x11, [x20, #344]
	ldr	x2, [x10, #280]
	ldr	w8, [x9, x8, lsl #2]
	ldr	x1, [x11, x8, lsl #3]
	bl	__ufs_mq_queue_rq
	ldr	x8, [x19, #72]
	mov	w21, w0
	ldr	x8, [x8, #144]
	ldr	x8, [x8, #1816]
	cbz	x8, .LBB11_11
// %bb.9:                               //   in Loop: Header=BB11_8 Depth=1
	ldr	x8, [x8, #80]
	cbz	x8, .LBB11_11
// %bb.10:                              //   in Loop: Header=BB11_8 Depth=1
	blr	x8
	cbz	w21, .LBB11_14
	b	.LBB11_5
.LBB11_11:                              //   in Loop: Header=BB11_8 Depth=1
	//APP
	//NO_APP
	ldr	w8, [x22, #16]
	subs	w8, w8, #1              // =1
	str	w8, [x22, #16]
	b.ne	.LBB11_13
// %bb.12:                              //   in Loop: Header=BB11_8 Depth=1
	ldr	x8, [x22]
	tbnz	w8, #1, .LBB11_4
.LBB11_13:                              //   in Loop: Header=BB11_8 Depth=1
	cbnz	w21, .LBB11_5
.LBB11_14:                              //   in Loop: Header=BB11_8 Depth=1
	ldr	x19, [sp, #8]
	cmp	x25, x19
	b.ne	.LBB11_8
// %bb.15:
	adrp	x9, __stack_chk_guard
	ldr	x8, [sp, #40]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB11_17
// %bb.16:
	ldp	x29, x30, [sp, #112]    // 16-byte Folded Reload
	ldp	x20, x19, [sp, #96]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #80]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #64]     // 16-byte Folded Reload
	ldp	x26, x25, [sp, #48]     // 16-byte Folded Reload
	add	sp, sp, #128            // =128
	ret
.LBB11_17:
	bl	__stack_chk_fail
.Lfunc_end11:
	.size	ufs_mq_flush_plug_list, .Lfunc_end11-ufs_mq_flush_plug_list
                                        // -- End function
	.p2align	2               // -- Begin function __ufs_mq_queue_rq
	.type	__ufs_mq_queue_rq,@function
__ufs_mq_queue_rq:                      // @__ufs_mq_queue_rq
// %bb.0:
	stp	x26, x25, [sp, #-80]!   // 16-byte Folded Spill
	stp	x20, x19, [sp, #48]     // 16-byte Folded Spill
	mov	x20, x0
	mov	x0, x4
	stp	x24, x23, [sp, #16]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #32]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #64]     // 16-byte Folded Spill
	add	x29, sp, #64            // =64
	mov	x22, x4
	mov	x21, x3
	mov	x19, x2
	mov	x23, x1
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB12_3
// %bb.1:
	ldrb	w8, [x20, #272]
	tbz	w8, #2, .LBB12_3
// %bb.2:
	ldr	w8, [x20, #84]
	mov	w9, #61439
	movk	w9, #65471, lsl #16
	and	w8, w8, w9
	str	w8, [x20, #84]
.LBB12_3:
	ldr	x25, [x19]
	ldrb	w8, [x25, #25]
	tbz	w8, #0, .LBB12_13
// %bb.4:
	ldrb	w8, [x20, #432]
	sub	w9, w8, #1              // =1
	and	w9, w9, #0xff
	cmp	w9, #3                  // =3
	b.hi	.LBB12_13
// %bb.5:
	ldrb	w9, [x20, #84]
	cmp	w9, #1                  // =1
	b.ne	.LBB12_13
// %bb.6:
	sub	x26, x8, #1             // =1
	orr	w8, wzr, #0x18
	madd	x8, x26, x8, x25
	add	x24, x8, #296           // =296
	mov	x0, x24
	bl	_raw_spin_lock_irqsave
	add	x9, x25, x26, lsl #4
	ldr	x8, [x9, #488]
	add	x10, x9, #488           // =488
	mov	x1, x0
	cmp	x8, x10
	b.ne	.LBB12_8
// %bb.7:
	ldr	x9, [x9, #496]
	cmp	x9, x10
	b.eq	.LBB12_44
.LBB12_8:
	ldr	x8, [x8, #16]
.LBB12_9:
	add	x9, x25, x26, lsl #3
	ldr	x10, [x20, #112]
	ldr	x11, [x9, #392]
	lsr	x9, x10, #3
	cmp	x11, x9
	b.eq	.LBB12_11
// %bb.10:
	cmp	x9, x8
	b.ne	.LBB12_12
.LBB12_11:
	ldr	x8, [x20, #232]
	orr	x8, x8, #0x2
	str	x8, [x20, #232]
.LBB12_12:
	mov	x0, x24
	bl	_raw_spin_unlock_irqrestore
.LBB12_13:
	ldr	w8, [x20, #84]
	tbz	w8, #11, .LBB12_51
// %bb.14:
	mov	w9, #4096
	movk	w9, #64, lsl #16
	tst	w8, w9
	b.eq	.LBB12_18
.LBB12_15:
	mov	w25, #4096
	mov	w8, w8
	movk	w25, #64, lsl #16
	tbnz	w8, #23, .LBB12_21
// %bb.16:
	and	x8, x8, x25
	cbz	x8, .LBB12_21
// %bb.17:
	add	x8, x19, #44            // =44
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	b	.LBB12_22
.LBB12_18:
	ldrb	w8, [x20, #232]
	tbz	w8, #1, .LBB12_29
// %bb.19:
	ldr	x0, [x20, #64]
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB12_29
// %bb.20:
	ldr	w8, [x20, #84]
	b	.LBB12_15
.LBB12_21:
	add	x8, x19, #40            // =40
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
.LBB12_22:
	ldr	x0, [x20, #64]
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB12_24
// %bb.23:
	ldrb	w8, [x20, #232]
	tbnz	w8, #1, .LBB12_49
.LBB12_24:
	ldr	x0, [x20, #64]
	bl	mas_blk_get_lld
	ldrb	w8, [x0, #24]
	tbz	w8, #6, .LBB12_49
// %bb.25:
	ldrb	w8, [x19, #576]
	cbnz	w8, .LBB12_49
// %bb.26:
	ldr	x0, [x20, #64]
	bl	mas_blk_get_lld
	ldr	w8, [x20, #84]
	mov	x24, x0
	tbz	w8, #23, .LBB12_35
// %bb.27:
	and	w9, w8, #0xff
	cmp	w9, #1                  // =1
	b.eq	.LBB12_35
.LBB12_28:
	ldr	x8, [x20, #272]
	ldr	w9, [x20, #84]
	orr	x8, x8, #0x2
	orr	w9, w9, #0x10000000
	str	x8, [x20, #272]
	str	w9, [x20, #84]
	add	x8, x19, #12            // =12
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	b	.LBB12_49
.LBB12_29:
	add	x8, x19, #32            // =32
	//APP
	// atomic_add_return
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stlxr	w10, w9, [x8]
	cbnz	w10, 1b
	dmb ish
	//NO_APP
	ldr	w8, [x19, #216]
	cmp	w9, w8
	b.le	.LBB12_31
// %bb.30:
	ldrb	w8, [x19, #576]
	cmp	w8, #1                  // =1
	b.ne	.LBB12_53
.LBB12_31:
	ldrb	w8, [x19, #392]
	cbnz	w8, .LBB12_49
// %bb.32:
	ldr	x0, [x20, #64]
	bl	mas_blk_get_lld
	ldrb	w8, [x0, #24]
	tbz	w8, #6, .LBB12_49
// %bb.33:
	ldr	w8, [x20, #84]
	tst	w8, #0xff
	b.ne	.LBB12_49
// %bb.34:
	ldr	x9, [x20, #272]
	orr	w8, w8, #0x10000000
	str	w8, [x20, #84]
	orr	x8, x9, #0x2
	str	x8, [x20, #272]
	add	x8, x19, #12            // =12
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	b	.LBB12_49
.LBB12_35:
	tbnz	w8, #23, .LBB12_38
// %bb.36:
	and	x8, x8, x25
	cbz	x8, .LBB12_38
// %bb.37:
	ldr	w8, [x19, #40]
	cmp	w8, #2                  // =2
	b.gt	.LBB12_49
.LBB12_38:
	ldr	w8, [x19, #32]
	cbnz	w8, .LBB12_40
// %bb.39:
	ldr	w8, [x19, #36]
	cbz	w8, .LBB12_28
.LBB12_40:
	ldr	w8, [x19, #12]
	cbz	w8, .LBB12_28
// %bb.41:
	ldr	x8, [x24, #1520]
	add	x26, x24, #1520         // =1520
	cmp	x26, x8
	b.eq	.LBB12_28
// %bb.42:
	add	x25, x24, #1552         // =1552
	mov	x0, x25
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x24, #1520]
	mov	x1, x0
	cmp	x26, x8
	b.eq	.LBB12_45
// %bb.43:
	ldur	x24, [x8, #-144]
	b	.LBB12_46
.LBB12_44:
	mov	x8, xzr
	b	.LBB12_9
.LBB12_45:
	mov	x24, xzr
.LBB12_46:
	mov	x0, x25
	bl	_raw_spin_unlock_irqrestore
	bl	ktime_get
	mov	w8, #57600
	movk	w8, #1525, lsl #16
	add	x8, x24, x8
	cmp	x0, x8
	b.lt	.LBB12_28
// %bb.47:
	mov	w8, #25856
	movk	w8, #7629, lsl #16
	add	x8, x24, x8
	cmp	x0, x8
	b.ge	.LBB12_49
// %bb.48:
	ldr	w8, [x19, #12]
	ldr	w9, [x19, #8]
	cmp	w8, w9
	b.lo	.LBB12_28
.LBB12_49:
	mov	x0, x23
	mov	x1, x22
	mov	x2, x21
	mov	x3, x19
	bl	__ufs_mq_queue_rq_internal
	ands	w21, w0, #0xff
	b.ne	.LBB12_54
.LBB12_50:
	mov	w0, w21
	ldp	x29, x30, [sp, #64]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #48]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #32]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #16]     // 16-byte Folded Reload
	ldp	x26, x25, [sp], #80     // 16-byte Folded Reload
	ret
.LBB12_51:
	add	x8, x19, #36            // =36
	//APP
	// atomic_add_return
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stlxr	w10, w9, [x8]
	cbnz	w10, 1b
	dmb ish
	//NO_APP
	ldr	w8, [x19, #220]
	cmp	w9, w8
	b.le	.LBB12_49
// %bb.52:
	ldrb	w8, [x19, #576]
	cmp	w8, #1                  // =1
	b.eq	.LBB12_49
.LBB12_53:
	orr	w8, wzr, #0x2
	str	w8, [x20, #284]
	mov	w21, #9
	b	.LBB12_56
.LBB12_54:
	orr	w8, wzr, #0x1
	str	w8, [x20, #284]
	ldr	w8, [x19, #44]
	ldr	w9, [x19, #40]
	ldr	w10, [x19, #32]
	ldr	w11, [x19, #36]
	add	w8, w9, w8
	add	w8, w8, w10
	add	w8, w8, w11
	cmp	w8, #8                  // =8
	b.gt	.LBB12_56
// %bb.55:
	orr	w8, wzr, #0x4
	str	w8, [x20, #284]
.LBB12_56:
	mov	x0, x20
	mov	x1, x19
	mov	w2, wzr
	bl	ufs_mq_rq_inflt_update
	b	.LBB12_50
.Lfunc_end12:
	.size	__ufs_mq_queue_rq, .Lfunc_end12-__ufs_mq_queue_rq
                                        // -- End function
	.p2align	2               // -- Begin function ufs_mq_insert_sync_list
	.type	ufs_mq_insert_sync_list,@function
ufs_mq_insert_sync_list:                // @ufs_mq_insert_sync_list
// %bb.0:
	str	x23, [sp, #-64]!        // 8-byte Folded Spill
	stp	x22, x21, [sp, #16]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	ldr	x8, [x1, #1808]
	orr	w1, wzr, #0x8
	add	x29, sp, #48            // =48
	mov	x21, x0
	ldr	x23, [x8, #280]
	bl	mas_blk_latency_req_check
	add	x19, x23, #48           // =48
	mov	x0, x19
	bl	_raw_spin_lock_irqsave
	ldr	w8, [x21, #84]
	mov	x20, x0
	tbnz	w8, #23, .LBB13_3
// %bb.1:
	mov	w9, #4096
	movk	w9, #64, lsl #16
	tst	x8, x9
	b.eq	.LBB13_9
// %bb.2:
	ldr	x8, [x23, #80]
	add	x9, x23, #72            // =72
	str	x21, [x23, #80]
	stp	x9, x8, [x21]
	str	x21, [x8]
	add	x8, x23, #88            // =88
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	b	.LBB13_27
.LBB13_3:
	mov	x9, x23
	ldr	x8, [x9, #72]!
	cmp	x9, x8
	b.eq	.LBB13_17
// %bb.4:
	cmp	x8, x9
	b.eq	.LBB13_19
// %bb.5:
	ldrb	w10, [x8, #86]
	tbz	w10, #7, .LBB13_8
.LBB13_6:                               // =>This Inner Loop Header: Depth=1
	ldr	x10, [x8]
	cmp	x10, x9
	b.eq	.LBB13_18
// %bb.7:                               //   in Loop: Header=BB13_6 Depth=1
	mov	x8, x10
	ldrb	w10, [x10, #86]
	tbnz	w10, #7, .LBB13_6
.LBB13_8:
	ldr	x9, [x8, #8]
	str	x21, [x8, #8]
	stp	x8, x9, [x21]
	str	x21, [x9]
	b	.LBB13_19
.LBB13_9:
	mov	x8, x23
	ldr	x9, [x8, #96]!
	cmp	x8, x9
	b.eq	.LBB13_20
// %bb.10:
	ldr	w10, [x21, #464]
	cbz	w10, .LBB13_20
// %bb.11:
	cmp	x9, x8
	b.eq	.LBB13_21
// %bb.12:
	mov	w22, wzr
.LBB13_13:                              // =>This Inner Loop Header: Depth=1
	ldr	w11, [x9, #464]
	cbz	w11, .LBB13_15
// %bb.14:                              //   in Loop: Header=BB13_13 Depth=1
	cmp	w10, w11
	b.pl	.LBB13_16
	b	.LBB13_23
.LBB13_15:                              //   in Loop: Header=BB13_13 Depth=1
	add	w22, w22, #1            // =1
.LBB13_16:                              //   in Loop: Header=BB13_13 Depth=1
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	.LBB13_13
	b	.LBB13_22
.LBB13_17:
	ldr	x8, [x23, #80]
.LBB13_18:
	str	x21, [x23, #80]
	stp	x9, x8, [x21]
	str	x21, [x8]
.LBB13_19:
	add	x8, x23, #88            // =88
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	b	.LBB13_27
.LBB13_20:
	ldr	x10, [x23, #104]
	mov	w22, wzr
	str	x21, [x23, #104]
	b	.LBB13_24
.LBB13_21:
	mov	w22, wzr
.LBB13_22:
	ldr	x10, [x9, #8]
	str	x21, [x9, #8]
	b	.LBB13_24
.LBB13_23:
	ldr	x10, [x9, #8]
	str	x21, [x9, #8]
	mov	x8, x9
.LBB13_24:
	stp	x8, x10, [x21]
	str	x21, [x10]
	ldr	x0, [x21, #64]
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB13_26
// %bb.25:
	mov	w0, #20
	mov	w1, w22
	bl	io_trace_unistore_count
.LBB13_26:
	add	x8, x23, #112           // =112
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
.LBB13_27:
	mov	x0, x19
	mov	x1, x20
	bl	_raw_spin_unlock_irqrestore
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]     // 16-byte Folded Reload
	ldr	x23, [sp], #64          // 8-byte Folded Reload
	ret
.Lfunc_end13:
	.size	ufs_mq_insert_sync_list, .Lfunc_end13-ufs_mq_insert_sync_list
                                        // -- End function
	.globl	ufs_mq_make_request     // -- Begin function ufs_mq_make_request
	.p2align	2
	.type	ufs_mq_make_request,@function
ufs_mq_make_request:                    // @ufs_mq_make_request
// %bb.0:
	sub	sp, sp, #208            // =208
	adrp	x8, __stack_chk_guard
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	stp	x29, x30, [sp, #192]    // 16-byte Folded Spill
	add	x29, sp, #192           // =192
	str	x27, [sp, #112]         // 8-byte Folded Spill
	stp	x26, x25, [sp, #128]    // 16-byte Folded Spill
	stp	x24, x23, [sp, #144]    // 16-byte Folded Spill
	stp	x22, x21, [sp, #160]    // 16-byte Folded Spill
	stp	x20, x19, [sp, #176]    // 16-byte Folded Spill
	stur	x8, [x29, #-88]
	stp	xzr, x1, [sp, #72]
	stp	xzr, xzr, [sp, #56]
	stp	xzr, xzr, [sp, #40]
	stp	xzr, xzr, [sp, #24]
	str	xzr, [sp, #16]
	ldr	w22, [x1, #16]
	mov	w8, #253
	mov	x19, x1
	mov	x20, x0
	tst	w22, w8
	b.eq	.LBB14_7
// %bb.1:
	mov	w8, #6144
	movk	w8, #68, lsl #16
	and	w8, w22, w8
	cbnz	w8, .LBB14_7
// %bb.2:
	orr	w21, wzr, #0x1
	ldr	x8, [x19, #8]
	cbz	x8, .LBB14_14
.LBB14_3:
	ldr	x0, [x8, #1120]
	cbz	x0, .LBB14_14
// %bb.4:
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB14_14
// %bb.5:
	ldrb	w8, [x19, #16]
	cbnz	w8, .LBB14_17
// %bb.6:
	//APP
	mrs x8, sp_el0
	//NO_APP
	ldr	x9, [x8, #2744]
	cbnz	x9, .LBB14_16
	b	.LBB14_17
.LBB14_7:
	ldr	x8, [x20, #1808]
	orr	w9, w22, #0x800
	str	w9, [x19, #16]
	ldr	x21, [x8, #280]
	bl	ktime_get
	ldr	w8, [x21, #312]
	mov	x19, x0
	cmp	w8, #1                  // =1
	b.eq	.LBB14_11
// %bb.8:
	cbnz	w8, .LBB14_13
// %bb.9:
	ldr	x8, [x21, #224]
	mov	w9, #19264
	movk	w9, #76, lsl #16
	add	x8, x8, x9
	cmp	x8, x19
	b.le	.LBB14_13
// %bb.10:
	orr	w8, wzr, #0x1
	str	x19, [x21, #232]
	str	w8, [x21, #312]
	adrp	x8, jiffies
	ldr	x8, [x8, :lo12:jiffies]
	add	x0, x21, #264           // =264
	add	x1, x8, #2              // =2
	bl	mod_timer
	b	.LBB14_13
.LBB14_11:
	ldr	x8, [x21, #224]
	mov	w9, #19264
	movk	w9, #76, lsl #16
	add	x8, x8, x9
	cmp	x8, x19
	b.le	.LBB14_13
// %bb.12:
	orr	w8, wzr, #0x2
	str	w8, [x21, #312]
.LBB14_13:
	str	x19, [x21, #224]
	ldr	x19, [sp, #80]
	mov	w21, wzr
	ldr	w22, [x19, #16]
	ldr	x8, [x19, #8]
	cbnz	x8, .LBB14_3
.LBB14_14:
	tst	w22, #0xc0000
	cset	w8, ne
	orr	w8, w21, w8
	tbnz	w8, #0, .LBB14_17
// %bb.15:
	//APP
	mrs x8, sp_el0
	//NO_APP
	ldr	x9, [x8, #2744]
	cbz	x9, .LBB14_17
.LBB14_16:
	ldr	w10, [x19, #48]
	cmp	w10, #128, lsl #12      // =524288
	b.ls	.LBB14_18
.LBB14_17:
	mov	x25, xzr
	b	.LBB14_21
.LBB14_18:
	ldr	x10, [x9, #64]
	cbz	x10, .LBB14_20
// %bb.19:
	adrp	x11, __cfi_ufs_mq_flush_plug_list
	add	x11, x11, :lo12:__cfi_ufs_mq_flush_plug_list
	cmp	x10, x11
	b.ne	.LBB14_17
.LBB14_20:
	adrp	x10, __cfi_ufs_mq_flush_plug_list
	add	x10, x10, :lo12:__cfi_ufs_mq_flush_plug_list
	str	x10, [x9, #64]
	ldr	x25, [x8, #2744]
.LBB14_21:
	ldr	x19, [sp, #80]
	ldrb	w8, [x19, #16]
	cmp	w8, #1                  // =1
	b.ne	.LBB14_24
// %bb.22:
	mov	x0, x20
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB14_24
// %bb.23:
	add	x1, sp, #80             // =80
	mov	x0, x20
	bl	mas_blk_queue_split_for_wop_write
.LBB14_24:
	mov	x0, x20
	bl	blk_queue_query_unistore_enable
	ldr	x8, [sp, #80]
	cmp	x8, x19
	b.eq	.LBB14_27
// %bb.25:
	tbz	w0, #0, .LBB14_27
// %bb.26:
	mov	w0, #21
	orr	w1, wzr, #0x1
	bl	io_trace_unistore_count
.LBB14_27:
	cbz	x25, .LBB14_37
// %bb.28:
	ldr	x22, [x25, #56]
	add	x24, x25, #48           // =48
	cmp	x22, x24
	b.eq	.LBB14_37
// %bb.29:
	ldr	x23, [sp, #80]
	mov	x26, xzr
.LBB14_30:                              // =>This Inner Loop Header: Depth=1
	ldr	x8, [x22, #64]
	cmp	x8, x20
	b.ne	.LBB14_36
// %bb.31:                              //   in Loop: Header=BB14_30 Depth=1
	mov	x0, x22
	mov	x1, x23
	bl	blk_rq_merge_ok
	mov	x26, x22
	tbz	w0, #0, .LBB14_36
// %bb.32:                              //   in Loop: Header=BB14_30 Depth=1
	mov	x0, x22
	mov	x1, x23
	bl	blk_try_merge
	cmp	w0, #1                  // =1
	b.eq	.LBB14_35
// %bb.33:                              //   in Loop: Header=BB14_30 Depth=1
	cmp	w0, #2                  // =2
	mov	x26, x22
	b.ne	.LBB14_36
// %bb.34:                              //   in Loop: Header=BB14_30 Depth=1
	mov	x0, x20
	mov	x1, x22
	mov	x2, x23
	bl	bio_attempt_back_merge
	mov	x26, x22
	tbz	w0, #0, .LBB14_36
	b	.LBB14_82
.LBB14_35:                              //   in Loop: Header=BB14_30 Depth=1
	mov	x0, x20
	mov	x1, x22
	mov	x2, x23
	bl	bio_attempt_front_merge
	mov	x26, x22
	tbnz	w0, #0, .LBB14_82
.LBB14_36:                              //   in Loop: Header=BB14_30 Depth=1
	ldr	x22, [x22, #8]
	cmp	x22, x24
	b.ne	.LBB14_30
	b	.LBB14_38
.LBB14_37:
	mov	x26, xzr
.LBB14_38:
	ldr	x0, [x20, #88]
	ldr	x1, [sp, #80]
	mov	x2, xzr
	bl	wbt_wait
	ldr	x8, [sp, #80]
	mov	w23, w0
	orr	w1, wzr, #0x3
	mov	x0, x8
	bl	mas_blk_latency_bio_check
	ldr	x1, [sp, #80]
	add	x3, sp, #16             // =16
	mov	x0, x20
	ldr	w2, [x1, #16]
	bl	blk_mq_get_request
	cbz	x0, .LBB14_85
// %bb.39:
	ldrb	w8, [x0, #84]
	mov	x22, x0
	cmp	w8, #1                  // =1
	b.eq	.LBB14_43
// %bb.40:
	cmp	w8, #3                  // =3
	b.eq	.LBB14_43
// %bb.41:
	cmp	w8, #2                  // =2
	b.ne	.LBB14_44
// %bb.42:
	ldr	x0, [x22, #64]
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB14_44
.LBB14_43:
	mov	x0, x22
	bl	blk_req_set_make_req_nr
.LBB14_44:
	ldr	x8, [x22, #528]
	ldr	x0, [x22, #64]
	orr	x8, x8, x23, lsl #61
	str	x8, [x22, #528]
	ldr	x20, [sp, #80]
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB14_46
// %bb.45:
	ldr	w8, [x20, #292]
	ldr	w9, [x22, #536]
	orr	w8, w9, w8
	str	w8, [x22, #536]
.LBB14_46:
	ldr	x0, [sp, #72]
	mov	x1, x22
	bl	request_to_qc_t
	ldr	x24, [sp, #80]
	mov	w20, w0
	ldr	w8, [x24, #16]
	and	w9, w8, #0xff
	cmp	w9, #2                  // =2
	b.eq	.LBB14_84
// %bb.47:
	tbnz	w8, #19, .LBB14_84
// %bb.48:
	ldr	w9, [x22, #84]
	tbz	w9, #11, .LBB14_89
// %bb.49:
	ldr	x8, [x22, #64]
	mov	x0, x22
	mov	x1, x24
	ldr	x23, [x8, #1808]
	bl	blk_init_request_from_bio
	orr	w1, wzr, #0x1
	mov	x0, x22
	bl	blk_account_io_start
	mov	x21, x22
	cbz	x25, .LBB14_61
// %bb.50:
	ldr	x0, [x22, #64]
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB14_57
// %bb.51:
	adrp	x8, __tracepoint_block_plug+8
	ldr	x21, [x22, #64]
	ldr	w8, [x8, :lo12:__tracepoint_block_plug+8]
	cmp	w8, #1                  // =1
	b.lt	.LBB14_57
// %bb.52:
	adrp	x9, cpu_number
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
	add	x9, x9, :lo12:cpu_number
	//APP
	//NO_APP
	ldr	w8, [x8, x9]
	adrp	x10, __cpu_online_mask
	add	x10, x10, :lo12:__cpu_online_mask
	add	w9, w8, #63             // =63
	cmp	w8, #0                  // =0
	csel	w9, w9, w8, lt
	asr	w9, w9, #6
	ldr	x9, [x10, w9, sxtw #3]
	orr	w10, wzr, #0x1
	lsl	x8, x10, x8
	tst	x8, x9
	b.eq	.LBB14_57
// %bb.53:
	//APP
	mrs x24, sp_el0
	//NO_APP
	ldr	w8, [x24, #16]
	add	w8, w8, #1              // =1
	str	w8, [x24, #16]
	//APP
	//NO_APP
	adrp	x8, __tracepoint_block_plug+32
	ldr	x27, [x8, :lo12:__tracepoint_block_plug+32]
	cbz	x27, .LBB14_55
.LBB14_54:                              // =>This Inner Loop Header: Depth=1
	ldp	x8, x0, [x27]
	mov	x1, x21
	blr	x8
	ldr	x8, [x27, #24]!
	cbnz	x8, .LBB14_54
.LBB14_55:
	//APP
	//NO_APP
	ldr	w8, [x24, #16]
	subs	w8, w8, #1              // =1
	str	w8, [x24, #16]
	b.ne	.LBB14_57
// %bb.56:
	ldr	x8, [x24]
	tbnz	w8, #1, .LBB14_112
.LBB14_57:
	cbz	x26, .LBB14_60
.LBB14_58:
	mov	x8, x25
	ldr	x9, [x8, #48]!
	cmp	x8, x9
	b.eq	.LBB14_100
// %bb.59:
	ldp	x8, x9, [x26]
	str	x9, [x8, #8]
	str	x8, [x9]
	str	x26, [x26]
	str	x26, [x26, #8]
.LBB14_60:
	ldr	x8, [x25, #56]
	add	x9, x25, #48            // =48
	orr	w1, wzr, #0x6
	mov	x0, x22
	str	x22, [x25, #56]
	stp	x9, x8, [x22]
	str	x22, [x8]
	bl	mas_blk_latency_req_check
	mov	x21, x26
.LBB14_61:
	str	x21, [sp, #88]
	ldr	x0, [x22, #64]
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB14_68
// %bb.62:
	cbz	x21, .LBB14_71
// %bb.63:
	ldr	x4, [x22, #64]
	ldrb	w9, [x21, #84]
	ldr	x8, [x4, #1808]
	ldr	x8, [x8, #280]
	cbz	w9, .LBB14_66
// %bb.64:
	mov	x9, x8
	ldr	x10, [x9, #72]!
	cmp	x9, x10
	b.ne	.LBB14_67
// %bb.65:
	mov	x9, x8
	ldr	x10, [x9, #96]!
	cmp	x9, x10
	b.ne	.LBB14_67
.LBB14_66:
	ldr	x9, [x8, #176]!
	cmp	x8, x9
	b.eq	.LBB14_81
.LBB14_67:
	mov	w21, #9
	b	.LBB14_71
.LBB14_68:
	cbz	x21, .LBB14_71
// %bb.69:
	ldr	x1, [sp, #72]
	ldr	x2, [x23, #280]
	ldr	x4, [x22, #64]
.LBB14_70:
	add	x3, sp, #88             // =88
	mov	x0, x21
	bl	__ufs_mq_queue_rq
	mov	w21, w0
.LBB14_71:
	ldr	x8, [sp, #64]
	ldr	x8, [x8, #144]
	ldr	x8, [x8, #1816]
	cbz	x8, .LBB14_74
// %bb.72:
	ldr	x8, [x8, #80]
	cbz	x8, .LBB14_74
// %bb.73:
	blr	x8
	cbz	w21, .LBB14_77
	b	.LBB14_102
.LBB14_74:
	//APP
	//NO_APP
	//APP
	mrs x8, sp_el0
	//NO_APP
	ldr	w9, [x8, #16]
	subs	w9, w9, #1              // =1
	str	w9, [x8, #16]
	b.ne	.LBB14_76
// %bb.75:
	ldr	x8, [x8]
	tbnz	w8, #1, .LBB14_101
.LBB14_76:
	cbnz	w21, .LBB14_102
.LBB14_77:
	ldr	x8, [sp, #80]
	cmp	x8, x19
	b.eq	.LBB14_79
// %bb.78:
	mov	x0, x19
	bl	generic_make_request
.LBB14_79:
	adrp	x9, __stack_chk_guard
	ldur	x8, [x29, #-88]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB14_113
// %bb.80:
	mov	w0, w20
	ldp	x29, x30, [sp, #192]    // 16-byte Folded Reload
	ldp	x20, x19, [sp, #176]    // 16-byte Folded Reload
	ldp	x22, x21, [sp, #160]    // 16-byte Folded Reload
	ldp	x24, x23, [sp, #144]    // 16-byte Folded Reload
	ldp	x26, x25, [sp, #128]    // 16-byte Folded Reload
	ldr	x27, [sp, #112]         // 8-byte Folded Reload
	add	sp, sp, #208            // =208
	ret
.LBB14_81:
	ldr	x1, [sp, #72]
	ldr	x2, [x23, #280]
	b	.LBB14_70
.LBB14_82:
	ldr	x8, [sp, #80]
	cmp	x8, x19
	b.eq	.LBB14_88
.LBB14_83:
	mov	x0, x19
	bl	generic_make_request
	mov	w20, #-1
	b	.LBB14_79
.LBB14_84:
	mov	x0, x22
	mov	x1, x24
	bl	blk_init_request_from_bio
	orr	w1, wzr, #0x1
	mov	x0, x22
	bl	blk_account_io_start
	mov	x0, x22
	bl	blk_insert_flush
	b	.LBB14_93
.LBB14_85:
	ldr	x0, [x20, #88]
	mov	w1, w23
	bl	__wbt_done
	ldr	x0, [sp, #80]
	ldrb	w8, [x0, #19]
	tbz	w8, #6, .LBB14_87
// %bb.86:
	orr	w8, wzr, #0xc
	strb	w8, [x0, #26]
	bl	bio_endio
	ldr	x0, [sp, #80]
.LBB14_87:
	cmp	x0, x19
	b.ne	.LBB14_83
.LBB14_88:
	mov	w20, #-1
	b	.LBB14_79
.LBB14_89:
	ldp	x23, x25, [sp, #64]
	mov	w10, #16384
	movk	w10, #12, lsl #16
	tst	w8, w10
	b.ne	.LBB14_91
// %bb.90:
	and	w8, w9, #0xfe
	orr	w8, w8, #0x1
	cmp	w8, #33                 // =33
	b.ne	.LBB14_105
.LBB14_91:
	mov	x0, x22
	mov	x1, x24
	bl	blk_init_request_from_bio
	orr	w1, wzr, #0x1
	mov	x0, x22
	bl	blk_account_io_start
	mov	x0, x23
	bl	_raw_spin_lock
.LBB14_92:
	ldr	x1, [x25, #192]
	mov	x0, x22
	bl	ufs_mq_req_insert
	mov	x0, x23
	bl	_raw_spin_unlock
.LBB14_93:
	ldr	x0, [sp, #72]
	mov	w1, w21
	bl	blk_mq_run_hw_queue
.LBB14_94:
	ldr	x8, [sp, #64]
	ldr	x8, [x8, #144]
	ldr	x8, [x8, #1816]
	cbz	x8, .LBB14_97
// %bb.95:
	ldr	x8, [x8, #80]
	cbz	x8, .LBB14_97
// %bb.96:
	blr	x8
	b	.LBB14_77
.LBB14_97:
	//APP
	//NO_APP
	//APP
	mrs x8, sp_el0
	//NO_APP
	ldr	w9, [x8, #16]
	subs	w9, w9, #1              // =1
	str	w9, [x8, #16]
	b.ne	.LBB14_77
// %bb.98:
	ldr	x8, [x8]
	tbz	w8, #1, .LBB14_77
// %bb.99:
	bl	preempt_schedule
	b	.LBB14_77
.LBB14_100:
	mov	x26, xzr
	b	.LBB14_60
.LBB14_101:
	bl	preempt_schedule
	cbz	w21, .LBB14_77
.LBB14_102:
	ldr	x0, [sp, #88]
	cmp	w21, #9                 // =9
	b.ne	.LBB14_104
// %bb.103:
	bl	__blk_mq_requeue_request
	ldr	x21, [x22, #64]
	ldr	x0, [sp, #88]
	mov	x1, x21
	bl	ufs_mq_insert_sync_list
	ldr	x21, [x21, #1808]
	adrp	x8, mas_blk_mq_sync_disp_wq
	ldr	x1, [x8, :lo12:mas_blk_mq_sync_disp_wq]
	orr	w0, wzr, #0x8
	add	x2, x21, #32            // =32
	mov	x3, xzr
	bl	queue_delayed_work_on
	bl	ktime_get
	str	x0, [x21, #8]
	b	.LBB14_77
.LBB14_104:
	mov	w1, #10
	bl	blk_mq_end_request
	b	.LBB14_77
.LBB14_105:
	ldr	x26, [x25, #192]
	mov	x0, x23
	str	x25, [sp, #8]           // 8-byte Folded Spill
	bl	_raw_spin_lock
	ldrb	w8, [x24, #17]
	tbnz	w8, #3, .LBB14_111
// %bb.106:
	ldr	x8, [x26, #1816]
	cbz	x8, .LBB14_111
// %bb.107:
	ldr	x27, [x8, #176]
	cbz	x27, .LBB14_111
// %bb.108:
	ldr	x8, [x27]
	ldr	x8, [x8, #40]
	cbz	x8, .LBB14_111
// %bb.109:
	ldr	x8, [x26, #1808]
	ldr	x8, [x8, #280]
	add	x25, x8, #120           // =120
	mov	x0, x25
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x27]
	mov	x27, x0
	mov	x0, x24
	mov	x1, x26
	ldr	x8, [x8, #40]
	blr	x8
	mov	w26, w0
	mov	x0, x25
	mov	x1, x27
	bl	_raw_spin_unlock_irqrestore
	tbz	w26, #0, .LBB14_111
// %bb.110:
	mov	x0, x23
	bl	_raw_spin_unlock
	mov	x0, x22
	bl	blk_mq_free_request
	b	.LBB14_94
.LBB14_111:
	mov	x0, x22
	mov	x1, x24
	bl	blk_init_request_from_bio
	orr	w1, wzr, #0x1
	mov	x0, x22
	bl	blk_account_io_start
	ldr	x25, [sp, #8]           // 8-byte Folded Reload
	b	.LBB14_92
.LBB14_112:
	bl	preempt_schedule_notrace
	cbnz	x26, .LBB14_58
	b	.LBB14_60
.LBB14_113:
	bl	__stack_chk_fail
.Lfunc_end14:
	.size	ufs_mq_make_request, .Lfunc_end14-ufs_mq_make_request
                                        // -- End function
	.globl	mas_blk_add_buf_to_recovery_list // -- Begin function mas_blk_add_buf_to_recovery_list
	.p2align	2
	.type	mas_blk_add_buf_to_recovery_list,@function
mas_blk_add_buf_to_recovery_list:       // @mas_blk_add_buf_to_recovery_list
// %bb.0:
	sub	sp, sp, #112            // =112
	stp	x28, x27, [sp, #16]     // 16-byte Folded Spill
	stp	x26, x25, [sp, #32]     // 16-byte Folded Spill
	stp	x24, x23, [sp, #48]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #64]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #80]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #96]     // 16-byte Folded Spill
	add	x29, sp, #96            // =96
	mov	x21, x1
	mov	x20, x0
	bl	mas_blk_get_lld
	cbz	x0, .LBB15_17
// %bb.1:
	mov	x19, x0
	mov	x0, x20
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB15_17
// %bb.2:
	add	x22, x19, #296          // =296
	mov	x0, x22
	bl	_raw_spin_lock_irqsave
	ldr	w8, [x21, #4]
	mov	x23, x0
	str	x8, [x19, #392]
	bl	ktime_get
	str	x0, [x19, #424]
	mov	x0, x22
	mov	x1, x23
	str	xzr, [x19, #456]
	bl	_raw_spin_unlock_irqrestore
	add	x22, x19, #320          // =320
	mov	x0, x22
	bl	_raw_spin_lock_irqsave
	ldr	w8, [x21, #8]
	mov	x23, x0
	str	x8, [x19, #400]
	bl	ktime_get
	str	x0, [x19, #432]
	mov	x0, x22
	mov	x1, x23
	str	xzr, [x19, #464]
	bl	_raw_spin_unlock_irqrestore
	add	x22, x19, #344          // =344
	mov	x0, x22
	bl	_raw_spin_lock_irqsave
	ldr	w8, [x21, #12]
	mov	x23, x0
	str	x8, [x19, #408]
	bl	ktime_get
	str	x0, [x19, #440]
	mov	x0, x22
	mov	x1, x23
	str	xzr, [x19, #472]
	bl	_raw_spin_unlock_irqrestore
	add	x22, x19, #368          // =368
	mov	x0, x22
	bl	_raw_spin_lock_irqsave
	ldr	w8, [x21, #16]
	mov	x23, x0
	str	x8, [x19, #416]
	bl	ktime_get
	str	x0, [x19, #448]
	mov	x0, x22
	mov	x1, x23
	str	xzr, [x19, #480]
	bl	_raw_spin_unlock_irqrestore
	ldr	x22, [x19, #1680]
	add	x0, x22, #192           // =192
	str	x0, [sp, #8]            // 8-byte Folded Spill
	bl	_raw_spin_lock_irqsave
	mov	x27, xzr
	add	x28, x22, #176          // =176
	str	x0, [sp]                // 8-byte Folded Spill
.LBB15_3:                               // =>This Loop Header: Depth=1
                                        //     Child Loop BB15_4 Depth 2
                                        //       Child Loop BB15_6 Depth 3
	orr	w8, wzr, #0x18
	madd	x8, x27, x8, x19
	add	x24, x8, #776           // =776
	mov	x0, x24
	bl	_raw_spin_lock_irqsave
	add	x8, x19, x27, lsl #4
	ldr	x22, [x8, #696]
	add	x23, x8, #696           // =696
	mov	x25, x0
	cmp	x22, x23
	b.eq	.LBB15_12
.LBB15_4:                               //   Parent Loop BB15_3 Depth=1
                                        // =>  This Loop Header: Depth=2
                                        //       Child Loop BB15_6 Depth 3
	sub	x26, x22, #352          // =352
	mov	x0, x26
	mov	x1, x20
	mov	x2, x21
	bl	mas_blk_bio_need_dispatch
	tbz	w0, #0, .LBB15_11
// %bb.5:                               //   in Loop: Header=BB15_4 Depth=2
	ldr	x8, [x28]
	cmp	x8, x28
	b.eq	.LBB15_8
.LBB15_6:                               //   Parent Loop BB15_3 Depth=1
                                        //     Parent Loop BB15_4 Depth=2
                                        // =>    This Inner Loop Header: Depth=3
	sub	x9, x8, #368            // =368
	cmp	x9, x26
	b.eq	.LBB15_11
// %bb.7:                               //   in Loop: Header=BB15_6 Depth=3
	ldr	x8, [x8]
	cmp	x28, x8
	b.ne	.LBB15_6
.LBB15_8:                               //   in Loop: Header=BB15_4 Depth=2
	mov	x0, x20
	bl	mas_blk_get_lld
	cbz	x0, .LBB15_11
// %bb.9:                               //   in Loop: Header=BB15_4 Depth=2
	ldr	x8, [x0, #1680]
	cbz	x8, .LBB15_11
// %bb.10:                              //   in Loop: Header=BB15_4 Depth=2
	ldr	x10, [x8, #184]
	add	x9, x22, #16            // =16
	add	x11, x8, #176           // =176
	str	x9, [x8, #184]
	stp	x11, x10, [x22, #16]
	str	x9, [x10]
.LBB15_11:                              //   in Loop: Header=BB15_4 Depth=2
	ldr	x22, [x22]
	cmp	x23, x22
	b.ne	.LBB15_4
.LBB15_12:                              //   in Loop: Header=BB15_3 Depth=1
	mov	x0, x24
	mov	x1, x25
	bl	_raw_spin_unlock_irqrestore
	add	x27, x27, #1            // =1
	cmp	x27, #5                 // =5
	b.ne	.LBB15_3
// %bb.13:
	ldp	x1, x0, [sp]            // 16-byte Folded Reload
	bl	_raw_spin_unlock_irqrestore
	ldr	x8, [x20, #1808]
	ldr	x8, [x8, #280]
	ldr	x9, [x8, #176]!
	cmp	x8, x9
	b.eq	.LBB15_17
// %bb.14:
	add	x20, x19, #632          // =632
	mov	x0, x20
	bl	mutex_lock
	ldr	w8, [x19, #552]
	cbnz	w8, .LBB15_16
// %bb.15:
	add	x0, x19, #560           // =560
	bl	down_write
	orr	w8, wzr, #0x1
	str	w8, [x19, #552]
.LBB15_16:
	mov	x0, x20
	bl	mutex_unlock
.LBB15_17:
	ldp	x29, x30, [sp, #96]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #80]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #64]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #48]     // 16-byte Folded Reload
	ldp	x26, x25, [sp, #32]     // 16-byte Folded Reload
	ldp	x28, x27, [sp, #16]     // 16-byte Folded Reload
	add	sp, sp, #112            // =112
	ret
.Lfunc_end15:
	.size	mas_blk_add_buf_to_recovery_list, .Lfunc_end15-mas_blk_add_buf_to_recovery_list
                                        // -- End function
	.globl	ufs_mq_sync_io_dispatch_work_fn // -- Begin function ufs_mq_sync_io_dispatch_work_fn
	.p2align	2
	.type	ufs_mq_sync_io_dispatch_work_fn,@function
ufs_mq_sync_io_dispatch_work_fn:        // @ufs_mq_sync_io_dispatch_work_fn
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	ldr	x0, [x0, #96]
	mov	x29, sp
	bl	ufs_mq_sync_dispatch
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end16:
	.size	ufs_mq_sync_io_dispatch_work_fn, .Lfunc_end16-ufs_mq_sync_io_dispatch_work_fn
                                        // -- End function
	.p2align	2               // -- Begin function ufs_mq_sync_dispatch
	.type	ufs_mq_sync_dispatch,@function
ufs_mq_sync_dispatch:                   // @ufs_mq_sync_dispatch
// %bb.0:
	sub	sp, sp, #240            // =240
	adrp	x8, __stack_chk_guard
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	stp	x29, x30, [sp, #224]    // 16-byte Folded Spill
	add	x29, sp, #224           // =224
	stp	x28, x27, [sp, #144]    // 16-byte Folded Spill
	stp	x26, x25, [sp, #160]    // 16-byte Folded Spill
	stp	x24, x23, [sp, #176]    // 16-byte Folded Spill
	stp	x22, x21, [sp, #192]    // 16-byte Folded Spill
	stp	x20, x19, [sp, #208]    // 16-byte Folded Spill
	stur	x8, [x29, #-88]
	ldr	x24, [x0, #1808]
	mov	x19, x0
	ldr	x20, [x24, #280]
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB17_38
// %bb.1:
	mov	x0, x19
	bl	mas_blk_get_lld
	mov	x21, x0
	cbz	x0, .LBB17_3
// %bb.2:
	ldr	w8, [x21, #552]
	cbz	w8, .LBB17_38
.LBB17_3:
	mov	x0, x19
	bl	mas_blk_get_lld
	cbz	x0, .LBB17_36
// %bb.4:
	ldr	x8, [x19, #1808]
	ldr	x8, [x8, #280]
	ldr	x9, [x8, #176]!
	cmp	x8, x9
	b.eq	.LBB17_30
// %bb.5:
	mov	x0, x19
	bl	mas_blk_get_lld
	cbz	x0, .LBB17_36
// %bb.6:
	ldr	x25, [x0, #1680]
	cbz	x25, .LBB17_36
// %bb.7:
	stp	x24, x0, [sp, #16]      // 16-byte Folded Spill
	add	x0, x25, #192           // =192
	str	x0, [sp, #8]            // 8-byte Folded Spill
	bl	_raw_spin_lock_irqsave
	add	x22, x25, #176          // =176
	str	x0, [sp]                // 8-byte Folded Spill
	//APP
	mrs x24, sp_el0
	//NO_APP
	b	.LBB17_9
.LBB17_8:                               //   in Loop: Header=BB17_9 Depth=1
	ldp	x8, x9, [x23]
	str	x9, [x8, #8]
	str	x8, [x9]
	str	x23, [x23]
	str	x23, [x23, #8]
.LBB17_9:                               // =>This Inner Loop Header: Depth=1
	ldr	x23, [x22]
	cmp	x23, x22
	b.ne	.LBB17_11
// %bb.10:                              //   in Loop: Header=BB17_9 Depth=1
	ldr	x8, [x25, #184]
	cmp	x8, x22
	b.eq	.LBB17_35
.LBB17_11:                              //   in Loop: Header=BB17_9 Depth=1
	add	x8, x25, #32            // =32
	sub	x9, x23, #360           // =360
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w10, [x8]
	add	w10, w10, #1
	stxr	w11, w10, [x8]
	cbnz	w11, 1b
	//NO_APP
	ldr	x8, [x9]
	sub	x28, x23, #368          // =368
	add	x3, sp, #72             // =72
	mov	x1, x28
	ldr	x27, [x8, #1120]
	orr	w8, wzr, #0x4
	stp	xzr, xzr, [sp, #120]
	stp	xzr, xzr, [sp, #104]
	stp	xzr, xzr, [sp, #72]
	stp	x8, xzr, [sp, #88]
	sub	x8, x23, #352           // =352
	ldr	w2, [x8]
	mov	x0, x27
	bl	blk_mq_get_request
	cbz	x0, .LBB17_25
// %bb.12:                              //   in Loop: Header=BB17_9 Depth=1
	ldrb	w8, [x0, #84]
	mov	x26, x0
	cmp	w8, #1                  // =1
	b.eq	.LBB17_16
// %bb.13:                              //   in Loop: Header=BB17_9 Depth=1
	cmp	w8, #3                  // =3
	b.eq	.LBB17_16
// %bb.14:                              //   in Loop: Header=BB17_9 Depth=1
	cmp	w8, #2                  // =2
	b.ne	.LBB17_17
// %bb.15:                              //   in Loop: Header=BB17_9 Depth=1
	ldr	x0, [x26, #64]
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB17_17
.LBB17_16:                              //   in Loop: Header=BB17_9 Depth=1
	mov	x0, x26
	bl	blk_req_set_make_req_nr
.LBB17_17:                              //   in Loop: Header=BB17_9 Depth=1
	mov	x0, x26
	mov	x1, x28
	bl	blk_init_request_from_bio
	orr	w1, wzr, #0x1
	mov	x0, x26
	bl	blk_account_io_start
	ldr	x8, [x26, #72]
	ldr	x9, [x27, #320]
	ldr	x10, [x27, #344]
	add	x2, sp, #56             // =56
	ldrsw	x8, [x8, #64]
	mov	x1, x27
	mov	x3, x25
	ldr	w8, [x9, x8, lsl #2]
	ldr	x0, [x10, x8, lsl #3]
	str	x26, [sp, #56]
	bl	__ufs_mq_queue_rq_internal
	ldr	x8, [sp, #120]
	mov	w27, w0
	ldr	x8, [x8, #144]
	ldr	x8, [x8, #1816]
	cbz	x8, .LBB17_20
// %bb.18:                              //   in Loop: Header=BB17_9 Depth=1
	ldr	x8, [x8, #80]
	cbz	x8, .LBB17_20
// %bb.19:                              //   in Loop: Header=BB17_9 Depth=1
	blr	x8
	tst	w27, #0xff
	b.eq	.LBB17_8
	b	.LBB17_24
.LBB17_20:                              //   in Loop: Header=BB17_9 Depth=1
	//APP
	//NO_APP
	ldr	w8, [x24, #16]
	subs	w8, w8, #1              // =1
	str	w8, [x24, #16]
	b.ne	.LBB17_22
// %bb.21:                              //   in Loop: Header=BB17_9 Depth=1
	ldr	x8, [x24]
	tbnz	w8, #1, .LBB17_23
.LBB17_22:                              //   in Loop: Header=BB17_9 Depth=1
	tst	w27, #0xff
	b.eq	.LBB17_8
	b	.LBB17_24
.LBB17_23:                              //   in Loop: Header=BB17_9 Depth=1
	bl	preempt_schedule
	tst	w27, #0xff
	b.eq	.LBB17_8
.LBB17_24:
	mov	x0, x26
	bl	blk_mq_free_request
.LBB17_25:
	add	x8, x25, #32            // =32
	//APP
	// atomic_sub
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	ldr	x23, [x25, #176]
	cmp	x22, x23
	b.eq	.LBB17_35
// %bb.26:
	ldr	x28, [sp, #24]          // 8-byte Folded Reload
	orr	w24, wzr, #0x18
.LBB17_27:                              // =>This Inner Loop Header: Depth=1
	ldurb	w8, [x23, #-104]
	sub	w9, w8, #1              // =1
	and	w9, w9, #0xff
	cmp	w9, #3                  // =3
	b.hi	.LBB17_29
// %bb.28:                              //   in Loop: Header=BB17_27 Depth=1
	sub	x9, x23, #328           // =328
	sub	x27, x8, #1             // =1
	ldr	x26, [x9]
	madd	x8, x27, x24, x28
	add	x25, x8, #296           // =296
	mov	x0, x25
	bl	_raw_spin_lock_irqsave
	add	x8, x28, x27, lsl #3
	ldr	x27, [x8, #392]
	mov	x1, x0
	mov	x0, x25
	bl	_raw_spin_unlock_irqrestore
	cmp	x27, x26, lsr #3
	b.eq	.LBB17_34
.LBB17_29:                              //   in Loop: Header=BB17_27 Depth=1
	ldr	x23, [x23]
	cmp	x22, x23
	b.ne	.LBB17_27
	b	.LBB17_35
.LBB17_30:
	ldr	x8, [x0, #208]
	mov	x22, x0
	add	x1, sp, #72             // =72
	mov	x0, x19
	blr	x8
	ldr	x8, [sp, #72]
	cbnz	x8, .LBB17_36
// %bb.31:
	mov	x23, x24
	add	x24, x22, #632          // =632
	mov	x0, x24
	bl	mutex_lock
	ldr	w8, [x22, #552]
	cbz	w8, .LBB17_33
// %bb.32:
	add	x0, x22, #560           // =560
	str	wzr, [x22, #552]
	bl	up_write
.LBB17_33:
	mov	x0, x24
	bl	mutex_unlock
	mov	x24, x23
	ldr	w8, [x21, #552]
	cbnz	w8, .LBB17_37
	b	.LBB17_38
.LBB17_34:
	ldp	x8, x9, [x23]
	str	x9, [x8, #8]
	str	x8, [x9]
	str	x23, [x23]
	str	x23, [x23, #8]
	ldr	x8, [x22]
	str	x23, [x8, #8]
	stp	x8, x22, [x23]
	str	x23, [x22]
.LBB17_35:
	ldp	x1, x0, [sp]            // 16-byte Folded Reload
	bl	_raw_spin_unlock_irqrestore
	ldr	x24, [sp, #16]          // 8-byte Folded Reload
.LBB17_36:
	ldr	w8, [x21, #552]
	cbz	w8, .LBB17_38
.LBB17_37:
	ldr	x19, [x19, #1808]
	adrp	x8, mas_blk_mq_sync_disp_wq
	ldr	x1, [x8, :lo12:mas_blk_mq_sync_disp_wq]
	orr	w0, wzr, #0x8
	add	x2, x19, #32            // =32
	orr	w3, wzr, #0x1
	bl	queue_delayed_work_on
	bl	ktime_get
	str	x0, [x19, #8]
	b	.LBB17_52
.LBB17_38:
	bl	ktime_get
	add	x21, x20, #48           // =48
	add	x23, x20, #72           // =72
	add	x28, x20, #96           // =96
	str	x0, [x24, #16]
	b	.LBB17_40
.LBB17_39:                              //   in Loop: Header=BB17_40 Depth=1
	mov	w1, #10
	mov	x0, x22
	bl	blk_mq_end_request
.LBB17_40:                              // =>This Inner Loop Header: Depth=1
	mov	x0, x21
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x23]
	mov	x1, x0
	cmp	x8, x23
	b.eq	.LBB17_42
.LBB17_41:                              //   in Loop: Header=BB17_40 Depth=1
	add	x8, x20, #88            // =88
	//APP
	// atomic_sub
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	mov	x8, x23
	ldr	x22, [x8]
	cbnz	x22, .LBB17_46
	b	.LBB17_50
.LBB17_42:                              //   in Loop: Header=BB17_40 Depth=1
	ldr	x8, [x20, #80]
	cmp	x8, x23
	b.ne	.LBB17_41
// %bb.43:                              //   in Loop: Header=BB17_40 Depth=1
	ldr	x8, [x28]
	cmp	x8, x28
	b.ne	.LBB17_45
// %bb.44:                              //   in Loop: Header=BB17_40 Depth=1
	ldr	x8, [x20, #104]
	cmp	x8, x28
	b.eq	.LBB17_50
.LBB17_45:                              //   in Loop: Header=BB17_40 Depth=1
	add	x8, x20, #112           // =112
	//APP
	// atomic_sub
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	mov	x8, x28
	ldr	x22, [x8]
	cbz	x22, .LBB17_50
.LBB17_46:                              //   in Loop: Header=BB17_40 Depth=1
	ldp	x8, x9, [x22]
	mov	x0, x21
	str	x9, [x8, #8]
	str	x8, [x9]
	str	x22, [x22]
	str	x22, [x22, #8]
	bl	_raw_spin_unlock_irqrestore
	ldr	x8, [x22, #72]
	ldr	x9, [x19, #320]
	ldr	x10, [x19, #344]
	ldrsw	x8, [x8, #64]
	ldr	w8, [x9, x8, lsl #2]
	ldr	x1, [x10, x8, lsl #3]
	str	x22, [sp, #40]
	ldr	w8, [x22, #84]
	and	w8, w8, #0xfc
	orr	w8, w8, #0x3
	cmp	w8, #35                 // =35
	b.eq	.LBB17_49
// %bb.47:                              //   in Loop: Header=BB17_40 Depth=1
	add	x3, sp, #40             // =40
	mov	x0, x22
	mov	x2, x20
	mov	x4, x19
	bl	__ufs_mq_queue_rq
	cbz	w0, .LBB17_40
.LBB17_48:                              //   in Loop: Header=BB17_40 Depth=1
	cmp	w0, #9                  // =9
	b.eq	.LBB17_54
	b	.LBB17_39
.LBB17_49:                              //   in Loop: Header=BB17_40 Depth=1
	add	x2, sp, #40             // =40
	mov	x0, x1
	mov	x1, x19
	mov	x3, x20
	bl	__ufs_mq_queue_rq_internal
	and	w0, w0, #0xff
	cbz	w0, .LBB17_40
	b	.LBB17_48
.LBB17_50:
	mov	x0, x21
	bl	_raw_spin_unlock_irqrestore
.LBB17_51:
	bl	ktime_get
	str	x0, [x24, #24]
.LBB17_52:
	adrp	x9, __stack_chk_guard
	ldur	x8, [x29, #-88]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB17_101
// %bb.53:
	ldp	x29, x30, [sp, #224]    // 16-byte Folded Reload
	ldp	x20, x19, [sp, #208]    // 16-byte Folded Reload
	ldp	x22, x21, [sp, #192]    // 16-byte Folded Reload
	ldp	x24, x23, [sp, #176]    // 16-byte Folded Reload
	ldp	x26, x25, [sp, #160]    // 16-byte Folded Reload
	ldp	x28, x27, [sp, #144]    // 16-byte Folded Reload
	add	sp, sp, #240            // =240
	ret
.LBB17_54:
	mov	x0, x22
	bl	__blk_mq_requeue_request
	mov	x0, x22
	mov	x1, x19
	bl	ufs_mq_insert_sync_list
	ldr	x27, [x20]
	str	wzr, [sp, #36]
	stp	xzr, xzr, [sp, #88]
	stp	xzr, xzr, [sp, #72]
	bl	ktime_get
	mov	x22, x0
	mov	x0, x19
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB17_92
// %bb.55:
	ldr	x8, [x28]
	cmp	x8, x28
	b.ne	.LBB17_57
// %bb.56:
	ldr	x8, [x20, #104]
	cmp	x8, x28
	b.eq	.LBB17_92
.LBB17_57:
	//APP
	mrs	x23, daif		// arch_local_irq_save
msr	daifset, #2
	//NO_APP
	mov	x0, x21
	bl	_raw_spin_trylock
	cbz	w0, .LBB17_90
// %bb.58:
	ldr	w8, [x27, #216]
	cbz	w8, .LBB17_91
// %bb.59:
	str	x24, [sp, #16]          // 8-byte Folded Spill
	add	x24, x27, #296          // =296
	mov	x0, x24
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x27, #392]
	ldr	w9, [x27, #216]
	mov	x1, x0
	udiv	x10, x8, x9
	msub	x8, x10, x9, x8
	cbnz	x8, .LBB17_61
// %bb.60:
	orr	w8, wzr, #0x1
	strb	w8, [sp, #36]
.LBB17_61:
	ldr	x8, [x27, #488]
	add	x9, x27, #488           // =488
	cmp	x8, x9
	b.ne	.LBB17_63
// %bb.62:
	ldr	x10, [x27, #496]
	cmp	x10, x9
	b.eq	.LBB17_64
.LBB17_63:
	ldr	x8, [x8, #16]
	str	x8, [sp, #72]
.LBB17_64:
	mov	x0, x24
	bl	_raw_spin_unlock_irqrestore
	add	x24, x27, #320          // =320
	mov	x0, x24
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x27, #400]
	ldr	w9, [x27, #216]
	mov	x1, x0
	udiv	x10, x8, x9
	msub	x8, x10, x9, x8
	cbnz	x8, .LBB17_66
// %bb.65:
	orr	w8, wzr, #0x1
	strb	w8, [sp, #37]
.LBB17_66:
	ldr	x8, [x27, #504]
	add	x9, x27, #504           // =504
	cmp	x8, x9
	b.ne	.LBB17_68
// %bb.67:
	ldr	x10, [x27, #512]
	cmp	x10, x9
	b.eq	.LBB17_69
.LBB17_68:
	ldr	x8, [x8, #16]
	str	x8, [sp, #80]
.LBB17_69:
	mov	x0, x24
	bl	_raw_spin_unlock_irqrestore
	add	x24, x27, #344          // =344
	mov	x0, x24
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x27, #408]
	ldr	w9, [x27, #216]
	mov	x1, x0
	udiv	x10, x8, x9
	msub	x8, x10, x9, x8
	cbnz	x8, .LBB17_71
// %bb.70:
	orr	w8, wzr, #0x1
	strb	w8, [sp, #38]
.LBB17_71:
	ldr	x8, [x27, #520]
	add	x9, x27, #520           // =520
	cmp	x8, x9
	b.ne	.LBB17_73
// %bb.72:
	ldr	x10, [x27, #528]
	cmp	x10, x9
	b.eq	.LBB17_74
.LBB17_73:
	ldr	x8, [x8, #16]
	str	x8, [sp, #88]
.LBB17_74:
	mov	x0, x24
	str	x23, [sp, #8]           // 8-byte Folded Spill
	str	x22, [sp, #24]          // 8-byte Folded Spill
	bl	_raw_spin_unlock_irqrestore
	add	x24, x27, #368          // =368
	mov	x0, x24
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x27, #416]
	ldr	w9, [x27, #216]
	mov	x1, x0
	udiv	x10, x8, x9
	msub	x8, x10, x9, x8
	cbnz	x8, .LBB17_76
// %bb.75:
	orr	w8, wzr, #0x1
	strb	w8, [sp, #39]
.LBB17_76:
	ldr	x8, [x27, #536]
	add	x9, x27, #536           // =536
	cmp	x8, x9
	b.ne	.LBB17_78
// %bb.77:
	ldr	x10, [x27, #544]
	cmp	x10, x9
	b.eq	.LBB17_79
.LBB17_78:
	ldr	x8, [x8, #16]
	str	x8, [sp, #96]
.LBB17_79:
	mov	x0, x24
	bl	_raw_spin_unlock_irqrestore
	ldr	x24, [x28]
	cmp	x24, x28
	b.eq	.LBB17_88
// %bb.80:
	orr	w23, wzr, #0x18
.LBB17_81:                              // =>This Inner Loop Header: Depth=1
	ldrb	w8, [x24, #84]
	cmp	w8, #1                  // =1
	b.ne	.LBB17_87
// %bb.82:                              //   in Loop: Header=BB17_81 Depth=1
	ldrb	w8, [x24, #432]
	sub	w9, w8, #1              // =1
	and	w9, w9, #0xff
	cmp	w9, #3                  // =3
	b.hi	.LBB17_87
// %bb.83:                              //   in Loop: Header=BB17_81 Depth=1
	sub	x22, x8, #1             // =1
	madd	x8, x22, x23, x27
	add	x25, x8, #296           // =296
	mov	x0, x25
	bl	_raw_spin_lock_irqsave
	add	x8, x27, x22, lsl #3
	ldr	x9, [x24, #112]
	ldr	x10, [x8, #392]
	mov	x26, x0
	lsr	x8, x9, #3
	cmp	x10, x8
	b.eq	.LBB17_97
// %bb.84:                              //   in Loop: Header=BB17_81 Depth=1
	add	x9, sp, #36             // =36
	ldrb	w9, [x9, x22]
	cbz	w9, .LBB17_86
// %bb.85:                              //   in Loop: Header=BB17_81 Depth=1
	add	x9, sp, #72             // =72
	ldr	x9, [x9, x22, lsl #3]
	cmp	x8, x9
	b.eq	.LBB17_97
.LBB17_86:                              //   in Loop: Header=BB17_81 Depth=1
	mov	x0, x25
	mov	x1, x26
	bl	_raw_spin_unlock_irqrestore
.LBB17_87:                              //   in Loop: Header=BB17_81 Depth=1
	ldr	x24, [x24]
	cmp	x24, x28
	b.ne	.LBB17_81
.LBB17_88:
	ldr	x1, [sp, #8]            // 8-byte Folded Reload
	mov	x0, x21
	bl	_raw_spin_unlock_irqrestore
	ldr	x21, [sp, #24]          // 8-byte Folded Reload
.LBB17_89:
	bl	ktime_get
	sub	w1, w0, w21
	mov	w0, #19
	bl	io_trace_unistore_count
	ldr	x24, [sp, #16]          // 8-byte Folded Reload
	b	.LBB17_92
.LBB17_90:
	//APP
	msr	daif, x23		// arch_local_irq_restore
	//NO_APP
	b	.LBB17_92
.LBB17_91:
	adrp	x0, .L.str.11
	adrp	x1, .L__func__.ufs_mq_dispatch_match_expected_lba
	add	x0, x0, :lo12:.L.str.11
	add	x1, x1, :lo12:.L__func__.ufs_mq_dispatch_match_expected_lba
	mov	w2, wzr
	bl	printk
	mov	x0, x21
	mov	x1, x23
	bl	_raw_spin_unlock_irqrestore
.LBB17_92:
	mov	w21, #37888
	movk	w21, #30517, lsl #16
	bl	ktime_get
	ldr	x8, [x20, #240]
	add	x8, x8, x21
	cmp	x8, x0
	b.ge	.LBB17_95
// %bb.93:
	bl	ktime_get
	ldr	x8, [x20, #248]
	add	x8, x8, x21
	cmp	x8, x0
	b.ge	.LBB17_95
// %bb.94:
	ldr	x19, [x19, #1808]
	adrp	x8, mas_blk_mq_sync_disp_wq
	ldr	x1, [x8, :lo12:mas_blk_mq_sync_disp_wq]
	orr	w0, wzr, #0x8
	add	x2, x19, #32            // =32
	orr	w3, wzr, #0x1
	b	.LBB17_96
.LBB17_95:
	ldr	x19, [x19, #1808]
	adrp	x8, mas_blk_mq_sync_disp_wq
	ldr	x1, [x8, :lo12:mas_blk_mq_sync_disp_wq]
	orr	w0, wzr, #0x8
	add	x2, x19, #32            // =32
	mov	x3, xzr
.LBB17_96:
	bl	queue_delayed_work_on
	bl	ktime_get
	str	x0, [x19, #8]
	b	.LBB17_51
.LBB17_97:
	mov	w0, #18
	orr	w1, wzr, #0x1
	bl	io_trace_unistore_count
	mov	x0, x25
	mov	x1, x26
	bl	_raw_spin_unlock_irqrestore
	ldp	x8, x9, [x24]
	mov	x0, x21
	str	x9, [x8, #8]
	str	x8, [x9]
	str	x24, [x24]
	ldr	x1, [sp, #8]            // 8-byte Folded Reload
	str	x24, [x24, #8]
	bl	_raw_spin_unlock_irqrestore
	ldr	x8, [x24, #72]
	ldr	x9, [x19, #320]
	ldr	x10, [x19, #344]
	ldrsw	x8, [x8, #64]
	ldr	w8, [x9, x8, lsl #2]
	ldr	x1, [x10, x8, lsl #3]
	str	x24, [sp, #56]
	ldr	w8, [x24, #84]
	and	w8, w8, #0xfc
	orr	w8, w8, #0x3
	cmp	w8, #35                 // =35
	b.eq	.LBB17_102
// %bb.98:
	add	x3, sp, #56             // =56
	mov	x0, x24
	mov	x2, x20
	mov	x4, x19
	bl	__ufs_mq_queue_rq
.LBB17_99:
	ldr	x21, [sp, #24]          // 8-byte Folded Reload
	cmp	w0, #9                  // =9
	b.ne	.LBB17_89
// %bb.100:
	mov	x0, x24
	bl	__blk_mq_requeue_request
	mov	x0, x24
	mov	x1, x19
	bl	ufs_mq_insert_sync_list
	b	.LBB17_89
.LBB17_101:
	bl	__stack_chk_fail
.LBB17_102:
	add	x2, sp, #56             // =56
	mov	x0, x1
	mov	x1, x19
	mov	x3, x20
	bl	__ufs_mq_queue_rq_internal
	and	w0, w0, #0xff
	b	.LBB17_99
.Lfunc_end17:
	.size	ufs_mq_sync_dispatch, .Lfunc_end17-ufs_mq_sync_dispatch
                                        // -- End function
	.globl	ufs_mq_write_throttle_check_timer_expire // -- Begin function ufs_mq_write_throttle_check_timer_expire
	.p2align	2
	.type	ufs_mq_write_throttle_check_timer_expire,@function
ufs_mq_write_throttle_check_timer_expire: // @ufs_mq_write_throttle_check_timer_expire
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	add	x20, x0, #368           // =368
	mov	x19, x0
	mov	x0, x20
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	bl	_raw_spin_lock_irqsave
	mov	x1, x0
	orr	w8, wzr, #0x1
	mov	x0, x20
	strb	w8, [x19, #392]
	bl	_raw_spin_unlock_irqrestore
	mov	x0, x19
	bl	ufs_mq_async_disp_inflt_lmt_decision
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end18:
	.size	ufs_mq_write_throttle_check_timer_expire, .Lfunc_end18-ufs_mq_write_throttle_check_timer_expire
                                        // -- End function
	.p2align	2               // -- Begin function ufs_mq_async_disp_inflt_lmt_decision
	.type	ufs_mq_async_disp_inflt_lmt_decision,@function
ufs_mq_async_disp_inflt_lmt_decision:   // @ufs_mq_async_disp_inflt_lmt_decision
// %bb.0:
	str	x21, [sp, #-48]!        // 8-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	mov	x19, x0
	bl	pch_lowmem_check2
	cmp	w0, #1                  // =1
	b.lt	.LBB19_6
// %bb.1:
	ldr	w8, [x19, #220]
	orr	w9, wzr, #0x2
	str	w9, [x19, #316]
	cmp	w8, #16                 // =16
	b.eq	.LBB19_8
// %bb.2:
	add	x20, x19, #368          // =368
	mov	x0, x20
	bl	_raw_spin_lock_irqsave
	ldrb	w8, [x19, #392]
	mov	x1, x0
	cbz	w8, .LBB19_5
// %bb.3:
	orr	w8, wzr, #0x10
.LBB19_4:
	str	w8, [x19, #220]
.LBB19_5:
	mov	x0, x20
	bl	_raw_spin_unlock_irqrestore
	bl	ktime_get
	str	x0, [x19, #256]
	b	.LBB19_8
.LBB19_6:
	adrp	x8, vm_dirty_ratio
	ldrsw	x8, [x8, :lo12:vm_dirty_ratio]
	add	x9, x8, #10             // =10
	cmp	w8, #51                 // =51
	csel	x20, x9, x8, lt
	bl	global_dirtyable_memory
	mov	x9, #62915
	movk	x9, #23592, lsl #16
	adrp	x10, vm_node_stat+128
	mul	x8, x0, x20
	movk	x9, #49807, lsl #32
	add	x10, x10, :lo12:vm_node_stat+128
	movk	x9, #10485, lsl #48
	lsr	x8, x8, #2
	ldr	x11, [x10]
	ldr	x12, [x10, #56]
	umulh	x8, x8, x9
	ldr	x9, [x10, #8]
	bic	x10, x11, x11, asr #63
	bic	x11, x12, x12, asr #63
	add	x10, x11, x10
	bic	x9, x9, x9, asr #63
	add	x9, x10, x9
	cmp	x9, x8, lsr #2
	cset	w8, hi
	str	w8, [x19, #316]
	b.ls	.LBB19_9
// %bb.7:
	mov	x0, x19
	bl	ufs_mq_async_fwb_mode_inflt_lmt_calc
.LBB19_8:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldr	x21, [sp], #48          // 8-byte Folded Reload
	ret
.LBB19_9:
	ldr	x8, [x19]
	ldr	w8, [x8, #1716]
	ldr	w10, [x19, #20]
	ldr	w9, [x19, #312]
	sub	w8, w8, w10
	cmp	w9, #3                  // =3
	b.hi	.LBB19_14
// %bb.10:
	adrp	x10, .LJTI19_0
	add	x10, x10, :lo12:.LJTI19_0
	adr	x11, .LBB19_11
	ldrb	w12, [x10, x9]
	add	x11, x11, x12, lsl #2
	br	x11
.LBB19_11:
	ldr	w8, [x19, #220]
	cmp	w8, #14                 // =14
	b.eq	.LBB19_8
// %bb.12:
	add	x20, x19, #368          // =368
	mov	x0, x20
	bl	_raw_spin_lock_irqsave
	ldrb	w8, [x19, #392]
	mov	x1, x0
	cbz	w8, .LBB19_5
// %bb.13:
	orr	w8, wzr, #0xe
	b	.LBB19_4
.LBB19_14:
	cmp	w8, #4                  // =4
	ldr	w8, [x19, #220]
	orr	w9, wzr, #0xe
	orr	w10, wzr, #0x10
	csel	w21, w10, w9, lo
	cmp	w8, w21
	b.eq	.LBB19_8
// %bb.15:
	add	x20, x19, #368          // =368
	mov	x0, x20
	bl	_raw_spin_lock_irqsave
	ldrb	w8, [x19, #392]
	mov	x1, x0
	cbz	w8, .LBB19_5
// %bb.16:
	str	w21, [x19, #220]
	b	.LBB19_5
.LBB19_17:
	ldr	w8, [x19, #220]
	cmp	w8, #10                 // =10
	b.eq	.LBB19_8
// %bb.18:
	add	x20, x19, #368          // =368
	mov	x0, x20
	bl	_raw_spin_lock_irqsave
	ldrb	w8, [x19, #392]
	mov	x1, x0
	cbz	w8, .LBB19_5
// %bb.19:
	mov	w8, #10
	b	.LBB19_4
.Lfunc_end19:
	.size	ufs_mq_async_disp_inflt_lmt_decision, .Lfunc_end19-ufs_mq_async_disp_inflt_lmt_decision
	.section	.rodata,"a",@progbits
.LJTI19_0:
	.byte	(.LBB19_14-.LBB19_11)>>2
	.byte	(.LBB19_11-.LBB19_11)>>2
	.byte	(.LBB19_17-.LBB19_11)>>2
	.byte	(.LBB19_11-.LBB19_11)>>2
                                        // -- End function
	.text
	.globl	ufs_mq_write_throttle_handler // -- Begin function ufs_mq_write_throttle_handler
	.p2align	2
	.type	ufs_mq_write_throttle_handler,@function
ufs_mq_write_throttle_handler:          // @ufs_mq_write_throttle_handler
// %bb.0:
	str	x21, [sp, #-48]!        // 8-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	ldr	x8, [x0, #1808]
	add	x29, sp, #32            // =32
	mov	w19, w1
	ldr	x21, [x8, #280]
	add	x20, x21, #368          // =368
	mov	x0, x20
	bl	_raw_spin_lock_irqsave
	tst	w19, #0x1
	mov	w8, #5
	mov	w9, #20
	mov	x1, x0
	csel	w8, w9, w8, ne
	mov	x0, x20
	strb	wzr, [x21, #392]
	str	w8, [x21, #220]
	bl	_raw_spin_unlock_irqrestore
	adrp	x8, jiffies
	ldr	x8, [x8, :lo12:jiffies]
	add	x0, x21, #320           // =320
	add	x1, x8, #250            // =250
	bl	mod_timer
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldr	x21, [sp], #48          // 8-byte Folded Reload
	ret
.Lfunc_end20:
	.size	ufs_mq_write_throttle_handler, .Lfunc_end20-ufs_mq_write_throttle_handler
                                        // -- End function
	.globl	ufs_mq_async_io_dispatch_work_fn // -- Begin function ufs_mq_async_io_dispatch_work_fn
	.p2align	2
	.type	ufs_mq_async_io_dispatch_work_fn,@function
ufs_mq_async_io_dispatch_work_fn:       // @ufs_mq_async_io_dispatch_work_fn
// %bb.0:
	sub	sp, sp, #112            // =112
	adrp	x8, __stack_chk_guard
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	str	x25, [sp, #32]          // 8-byte Folded Spill
	stp	x24, x23, [sp, #48]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #64]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #80]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #96]     // 16-byte Folded Spill
	str	x8, [sp, #24]
	ldr	x19, [x0, #96]
	add	x29, sp, #96            // =96
	ldr	x25, [x19, #1808]
	ldr	x20, [x25, #280]
	bl	ktime_get
	str	x0, [x25, #144]
	b	.LBB21_2
.LBB21_1:                               //   in Loop: Header=BB21_2 Depth=1
	ldr	x8, [x21, #112]
	ldr	w9, [x21, #104]
	add	x8, x8, x9, lsr #9
	str	x8, [x19, #360]
.LBB21_2:                               // =>This Inner Loop Header: Depth=1
	mov	x0, x20
	bl	ufs_mq_async_disp_inflt_lmt_decision
	ldr	x8, [x19, #1816]
	cbz	x8, .LBB21_12
// %bb.3:                               //   in Loop: Header=BB21_2 Depth=1
	ldr	x21, [x8, #176]
	cbz	x21, .LBB21_12
// %bb.4:                               //   in Loop: Header=BB21_2 Depth=1
	ldr	x8, [x21]
	ldr	x8, [x8, #24]
	cbz	x8, .LBB21_12
// %bb.5:                               //   in Loop: Header=BB21_2 Depth=1
	ldr	x8, [x19, #1808]
	ldr	x23, [x8, #280]
	add	x22, x23, #120          // =120
	mov	x0, x22
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x21]
	mov	x24, x0
	mov	x0, x23
	ldr	x8, [x8, #24]
	blr	x8
	cbz	x0, .LBB21_11
// %bb.6:                               //   in Loop: Header=BB21_2 Depth=1
	add	x8, x23, #168           // =168
	//APP
	// atomic_sub
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	mov	x21, x0
	mov	x0, x22
	mov	x1, x24
	bl	_raw_spin_unlock_irqrestore
	ldr	x8, [x21, #72]
	ldr	x9, [x19, #320]
	ldr	x10, [x19, #344]
	ldrsw	x8, [x8, #64]
	ldr	w8, [x9, x8, lsl #2]
	ldr	x1, [x10, x8, lsl #3]
	str	x21, [sp, #8]
	ldr	w8, [x21, #84]
	and	w8, w8, #0xfc
	orr	w8, w8, #0x3
	cmp	w8, #35                 // =35
	b.eq	.LBB21_10
// %bb.7:                               //   in Loop: Header=BB21_2 Depth=1
	add	x3, sp, #8              // =8
	mov	x0, x21
	mov	x2, x20
	mov	x4, x19
	bl	__ufs_mq_queue_rq
	cbz	w0, .LBB21_1
.LBB21_8:                               //   in Loop: Header=BB21_2 Depth=1
	cmp	w0, #9                  // =9
	b.eq	.LBB21_14
// %bb.9:                               //   in Loop: Header=BB21_2 Depth=1
	mov	w1, #10
	mov	x0, x21
	bl	blk_mq_end_request
	b	.LBB21_2
.LBB21_10:                              //   in Loop: Header=BB21_2 Depth=1
	add	x2, sp, #8              // =8
	mov	x0, x1
	mov	x1, x19
	mov	x3, x20
	bl	__ufs_mq_queue_rq_internal
	and	w0, w0, #0xff
	cbz	w0, .LBB21_1
	b	.LBB21_8
.LBB21_11:
	mov	x0, x22
	mov	x1, x24
	bl	_raw_spin_unlock_irqrestore
.LBB21_12:
	bl	ktime_get
	str	x0, [x25, #152]
	adrp	x9, __stack_chk_guard
	ldr	x8, [sp, #24]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB21_19
// %bb.13:
	ldp	x29, x30, [sp, #96]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #80]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #64]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #48]     // 16-byte Folded Reload
	ldr	x25, [sp, #32]          // 8-byte Folded Reload
	add	sp, sp, #112            // =112
	ret
.LBB21_14:
	mov	x0, x21
	bl	__blk_mq_requeue_request
	ldr	x8, [x19, #1816]
	ldr	x20, [x19, #1808]
	cbz	x8, .LBB21_18
// %bb.15:
	ldr	x23, [x8, #176]
	cbz	x23, .LBB21_18
// %bb.16:
	ldr	x8, [x23]
	ldr	x8, [x8, #32]
	cbz	x8, .LBB21_18
// %bb.17:
	ldr	x20, [x20, #280]
	mov	w1, #9
	mov	x0, x21
	bl	mas_blk_latency_req_check
	add	x22, x20, #120          // =120
	mov	x0, x22
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x23]
	mov	x23, x0
	mov	x0, x21
	mov	x1, x20
	ldr	x8, [x8, #32]
	blr	x8
	add	x8, x20, #168           // =168
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	mov	x0, x22
	mov	x1, x23
	bl	_raw_spin_unlock_irqrestore
	ldr	x20, [x19, #1808]
.LBB21_18:
	adrp	x8, mas_blk_mq_async_disp_wq
	ldr	x1, [x8, :lo12:mas_blk_mq_async_disp_wq]
	add	x2, x20, #160           // =160
	orr	w0, wzr, #0x8
	orr	w3, wzr, #0x1
	bl	queue_delayed_work_on
	bl	ktime_get
	str	x0, [x20, #136]
	b	.LBB21_12
.LBB21_19:
	bl	__stack_chk_fail
.Lfunc_end21:
	.size	ufs_mq_async_io_dispatch_work_fn, .Lfunc_end21-ufs_mq_async_io_dispatch_work_fn
                                        // -- End function
	.globl	__ufs_mq_complete_request_remote // -- Begin function __ufs_mq_complete_request_remote
	.p2align	2
	.type	__ufs_mq_complete_request_remote,@function
__ufs_mq_complete_request_remote:       // @__ufs_mq_complete_request_remote
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	ldr	w8, [x0, #84]
	mov	w9, #2048
	movk	w9, #64, lsl #16
	mov	x19, x0
	tst	w8, w9
	add	x29, sp, #16            // =16
	b.eq	.LBB22_3
// %bb.1:
	orr	w1, wzr, #0xc
	mov	x0, x19
	bl	mas_blk_latency_req_check
	ldr	x8, [x19, #64]
	mov	x0, x19
	ldr	x8, [x8, #256]
	blr	x8
.LBB22_2:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.LBB22_3:
	//APP
	mrs	x20, daif		// arch_local_irq_save
msr	daifset, #2
	//NO_APP
	adrp	x9, blk_cpu_done
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
	add	x9, x9, :lo12:blk_cpu_done
	//APP
	//NO_APP
	add	x8, x8, x9
	ldr	x9, [x8, #8]
	add	x10, x19, #136          // =136
	str	x10, [x8, #8]
	stp	x8, x9, [x19, #136]
	str	x10, [x9]
	ldr	x8, [x8]
	cmp	x8, x10
	b.ne	.LBB22_5
// %bb.4:
	orr	w0, wzr, #0x4
	bl	raise_softirq_irqoff
.LBB22_5:
	//APP
	msr	daif, x20		// arch_local_irq_restore
	//NO_APP
	b	.LBB22_2
.Lfunc_end22:
	.size	__ufs_mq_complete_request_remote, .Lfunc_end22-__ufs_mq_complete_request_remote
                                        // -- End function
	.globl	ufs_mq_io_guard_work_fn // -- Begin function ufs_mq_io_guard_work_fn
	.p2align	2
	.type	ufs_mq_io_guard_work_fn,@function
ufs_mq_io_guard_work_fn:                // @ufs_mq_io_guard_work_fn
// %bb.0:
	stp	x24, x23, [sp, #-64]!   // 16-byte Folded Spill
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	adrp	x19, io_guard_queue_list_lock
	add	x19, x19, :lo12:io_guard_queue_list_lock
	mov	x0, x19
	stp	x22, x21, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	add	x29, sp, #48            // =48
	bl	_raw_spin_lock
	adrp	x20, mas_io_guard_queue_list
	add	x20, x20, :lo12:mas_io_guard_queue_list
	ldr	x21, [x20]
	mov	x0, x19
	bl	_raw_spin_unlock
	cmp	x21, x20
	b.eq	.LBB23_9
// %bb.1:
	adrp	x19, io_guard_queue_list_lock
	add	x19, x19, :lo12:io_guard_queue_list_lock
	adrp	x22, mas_blk_mq_async_disp_wq
.LBB23_2:                               // =>This Inner Loop Header: Depth=1
	ldr	x23, [x21, #16]
	sub	x24, x21, #264          // =264
	ldr	w8, [x23, #24]
	ldr	w9, [x23, #16]
	cmn	w8, w9
	b.eq	.LBB23_5
// %bb.3:                               //   in Loop: Header=BB23_2 Depth=1
	ldr	w8, [x23, #44]
	ldr	w9, [x23, #40]
	ldr	w10, [x23, #32]
	add	w8, w9, w8
	cmn	w8, w10
	b.ne	.LBB23_5
// %bb.4:                               //   in Loop: Header=BB23_2 Depth=1
	ldr	x0, [x24]
	bl	ufs_mq_sync_dispatch
.LBB23_5:                               //   in Loop: Header=BB23_2 Depth=1
	ldr	w8, [x23, #20]
	cbz	w8, .LBB23_8
// %bb.6:                               //   in Loop: Header=BB23_2 Depth=1
	ldr	w8, [x23, #36]
	cbnz	w8, .LBB23_8
// %bb.7:                               //   in Loop: Header=BB23_2 Depth=1
	ldr	x8, [x24]
	ldr	x1, [x22, :lo12:mas_blk_mq_async_disp_wq]
	orr	w0, wzr, #0x8
	mov	x3, xzr
	ldr	x23, [x8, #1808]
	add	x2, x23, #160           // =160
	bl	queue_delayed_work_on
	bl	ktime_get
	str	x0, [x23, #136]
.LBB23_8:                               //   in Loop: Header=BB23_2 Depth=1
	mov	x0, x19
	bl	_raw_spin_lock
	ldr	x21, [x21]
	mov	x0, x19
	bl	_raw_spin_unlock
	cmp	x21, x20
	b.ne	.LBB23_2
.LBB23_9:
	adrp	x8, mas_blk_io_guard_wq
	ldr	x1, [x8, :lo12:mas_blk_io_guard_wq]
	adrp	x2, mas_io_guard_work
	add	x2, x2, :lo12:mas_io_guard_work
	orr	w0, wzr, #0x8
	mov	w3, #500
	bl	queue_delayed_work_on
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]     // 16-byte Folded Reload
	ldp	x24, x23, [sp], #64     // 16-byte Folded Reload
	ret
.Lfunc_end23:
	.size	ufs_mq_io_guard_work_fn, .Lfunc_end23-ufs_mq_io_guard_work_fn
                                        // -- End function
	.globl	ufs_mq_req_alloc_prep   // -- Begin function ufs_mq_req_alloc_prep
	.p2align	2
	.type	ufs_mq_req_alloc_prep,@function
ufs_mq_req_alloc_prep:                  // @ufs_mq_req_alloc_prep
// %bb.0:
	orr	x8, x1, #0x800
	tst	w2, #0x1
	csel	x8, x1, x8, ne
	str	x8, [x0, #8]
	ret
.Lfunc_end24:
	.size	ufs_mq_req_alloc_prep, .Lfunc_end24-ufs_mq_req_alloc_prep
                                        // -- End function
	.globl	ufs_mq_req_init         // -- Begin function ufs_mq_req_init
	.p2align	2
	.type	ufs_mq_req_init,@function
ufs_mq_req_init:                        // @ufs_mq_req_init
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	str	x0, [x1, #288]
	mov	x0, x1
	mov	x29, sp
	str	xzr, [x1, #272]
	bl	mas_blk_request_init_unistore
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end25:
	.size	ufs_mq_req_init, .Lfunc_end25-ufs_mq_req_init
                                        // -- End function
	.globl	ufs_mq_req_complete     // -- Begin function ufs_mq_req_complete
	.p2align	2
	.type	ufs_mq_req_complete,@function
ufs_mq_req_complete:                    // @ufs_mq_req_complete
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	mov	x19, x0
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	tbz	w2, #0, .LBB26_3
// %bb.1:
	ldr	x8, [x1, #1808]
	ldr	x1, [x8, #280]
	ldr	w8, [x19, #308]
	add	x8, x1, x8, lsl #2
	add	x8, x8, #396            // =396
	//APP
	// atomic_sub
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	ldr	w8, [x19, #84]
	and	w8, w8, #0xfc
	orr	w8, w8, #0x3
	cmp	w8, #35                 // =35
	b.eq	.LBB26_3
// %bb.2:
	orr	w2, wzr, #0x1
	mov	x0, x19
	bl	ufs_mq_rq_inflt_update
.LBB26_3:
	ldr	x8, [x19, #72]
	cbz	x8, .LBB26_7
// %bb.4:
	adrp	x11, cpu_number
	ldr	w9, [x8, #64]
	//APP
	.if 1 == 1
661:
	mrs x10, tpidr_el1
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
	mrs x10, tpidr_el2
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
	add	x11, x11, :lo12:cpu_number
	//APP
	//NO_APP
	ldr	w10, [x10, x11]
	cmp	w9, w10
	b.eq	.LBB26_7
// %bb.5:
	add	w10, w9, #63            // =63
	cmp	w9, #0                  // =0
	csel	w10, w10, w9, lt
	adrp	x11, __cpu_online_mask
	asr	w10, w10, #6
	add	x11, x11, :lo12:__cpu_online_mask
	ldr	x10, [x11, w10, sxtw #3]
	orr	w11, wzr, #0x1
	lsl	x9, x11, x9
	tst	x10, x9
	b.eq	.LBB26_7
// %bb.6:
	adrp	x9, __cfi__ufs_mq_complete_request_remote
	add	x9, x9, :lo12:__cfi__ufs_mq_complete_request_remote
	stp	x9, x19, [x19, #40]
	str	wzr, [x19, #56]
	ldr	w0, [x8, #64]
	add	x1, x19, #32            // =32
	bl	smp_call_function_single_async
	b	.LBB26_10
.LBB26_7:
	//APP
	mrs	x20, daif		// arch_local_irq_save
msr	daifset, #2
	//NO_APP
	adrp	x9, blk_cpu_done
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
	add	x9, x9, :lo12:blk_cpu_done
	//APP
	//NO_APP
	add	x8, x8, x9
	ldr	x9, [x8, #8]
	add	x10, x19, #136          // =136
	str	x10, [x8, #8]
	stp	x8, x9, [x19, #136]
	str	x10, [x9]
	ldr	x8, [x8]
	cmp	x8, x10
	b.ne	.LBB26_9
// %bb.8:
	orr	w0, wzr, #0x4
	bl	raise_softirq_irqoff
.LBB26_9:
	//APP
	msr	daif, x20		// arch_local_irq_restore
	//NO_APP
.LBB26_10:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end26:
	.size	ufs_mq_req_complete, .Lfunc_end26-ufs_mq_req_complete
                                        // -- End function
	.p2align	2               // -- Begin function ufs_mq_rq_inflt_update
	.type	ufs_mq_rq_inflt_update,@function
ufs_mq_rq_inflt_update:                 // @ufs_mq_rq_inflt_update
// %bb.0:
	str	x21, [sp, #-48]!        // 8-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	ldr	w8, [x0, #84]
	mov	w20, w2
	mov	x19, x1
	add	x29, sp, #32            // =32
	tbz	w8, #11, .LBB27_16
// %bb.1:
	mov	w9, #4096
	movk	w9, #64, lsl #16
	mov	x21, x0
	tst	w8, w9
	b.eq	.LBB27_5
// %bb.2:
	mov	w8, w8
	tbnz	w8, #23, .LBB27_8
.LBB27_3:
	mov	w9, #4096
	movk	w9, #64, lsl #16
	and	x8, x8, x9
	cbz	x8, .LBB27_8
// %bb.4:
	add	x8, x19, #44            // =44
	//APP
	// atomic_sub
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	ldr	x8, [x21, #272]
	tbnz	w8, #1, .LBB27_9
	b	.LBB27_10
.LBB27_5:
	ldrb	w8, [x21, #232]
	tbz	w8, #1, .LBB27_11
// %bb.6:
	ldr	x0, [x21, #64]
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB27_11
// %bb.7:
	ldr	w8, [x21, #84]
	mov	w8, w8
	tbz	w8, #23, .LBB27_3
.LBB27_8:
	add	x8, x19, #40            // =40
	//APP
	// atomic_sub
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	ldr	x8, [x21, #272]
	tbz	w8, #1, .LBB27_10
.LBB27_9:
	and	x8, x8, #0xfffffffffffffffd
	str	x8, [x21, #272]
	add	x8, x19, #12            // =12
	//APP
	// atomic_sub
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
.LBB27_10:
	tbnz	w20, #0, .LBB27_14
	b	.LBB27_15
.LBB27_11:
	ldr	x8, [x21, #272]
	tbz	w8, #1, .LBB27_13
// %bb.12:
	and	x8, x8, #0xfffffffffffffffd
	str	x8, [x21, #272]
	add	x8, x19, #12            // =12
	//APP
	// atomic_sub
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
.LBB27_13:
	add	x8, x19, #32            // =32
	//APP
	// atomic_sub
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	tbz	w20, #0, .LBB27_15
.LBB27_14:
	bl	ktime_get
	str	x0, [x19, #240]
.LBB27_15:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldr	x21, [sp], #48          // 8-byte Folded Reload
	ret
.LBB27_16:
	add	x8, x19, #36            // =36
	//APP
	// atomic_sub
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	tbz	w20, #0, .LBB27_15
// %bb.17:
	bl	ktime_get
	str	x0, [x19, #248]
	b	.LBB27_15
.Lfunc_end27:
	.size	ufs_mq_rq_inflt_update, .Lfunc_end27-ufs_mq_rq_inflt_update
                                        // -- End function
	.globl	ufs_mq_req_deinit       // -- Begin function ufs_mq_req_deinit
	.p2align	2
	.type	ufs_mq_req_deinit,@function
ufs_mq_req_deinit:                      // @ufs_mq_req_deinit
// %bb.0:
	ret
.Lfunc_end28:
	.size	ufs_mq_req_deinit, .Lfunc_end28-ufs_mq_req_deinit
                                        // -- End function
	.globl	ufs_mq_req_insert       // -- Begin function ufs_mq_req_insert
	.p2align	2
	.type	ufs_mq_req_insert,@function
ufs_mq_req_insert:                      // @ufs_mq_req_insert
// %bb.0:
	stp	x22, x21, [sp, #-48]!   // 16-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	ldr	w8, [x0, #84]
	mov	x19, x0
	mov	x21, x1
	add	x29, sp, #32            // =32
	tbnz	w8, #11, .LBB29_2
// %bb.1:
	and	w8, w8, #0xfc
	orr	w8, w8, #0x3
	cmp	w8, #35                 // =35
	b.ne	.LBB29_4
.LBB29_2:
	mov	x0, x19
	mov	x1, x21
	bl	ufs_mq_insert_sync_list
.LBB29_3:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldp	x22, x21, [sp], #48     // 16-byte Folded Reload
	ret
.LBB29_4:
	ldr	x8, [x21, #1808]
	mov	x0, x21
	ldr	x20, [x8, #280]
	bl	blk_queue_query_unistore_enable
	tbnz	w0, #0, .LBB29_2
// %bb.5:
	ldr	x8, [x21, #1816]
	cbz	x8, .LBB29_3
// %bb.6:
	ldr	x22, [x8, #176]
	cbz	x22, .LBB29_3
// %bb.7:
	ldr	x8, [x22]
	ldr	x8, [x8, #16]
	cbz	x8, .LBB29_3
// %bb.8:
	mov	w1, #9
	mov	x0, x19
	bl	mas_blk_latency_req_check
	add	x21, x20, #120          // =120
	mov	x0, x21
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x22]
	mov	x22, x0
	mov	x0, x19
	mov	x1, x20
	ldr	x8, [x8, #16]
	blr	x8
	add	x8, x20, #168           // =168
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	mov	x0, x21
	mov	x1, x22
	bl	_raw_spin_unlock_irqrestore
	b	.LBB29_3
.Lfunc_end29:
	.size	ufs_mq_req_insert, .Lfunc_end29-ufs_mq_req_insert
                                        // -- End function
	.globl	ufs_mq_req_requeue      // -- Begin function ufs_mq_req_requeue
	.p2align	2
	.type	ufs_mq_req_requeue,@function
ufs_mq_req_requeue:                     // @ufs_mq_req_requeue
// %bb.0:
	stp	x22, x21, [sp, #-48]!   // 16-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	mov	x19, x0
	str	wzr, [x0, #468]
	mov	x0, x1
	add	x29, sp, #32            // =32
	mov	x20, x1
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB30_7
// %bb.1:
	ldr	x8, [x19, #120]
	cbz	x8, .LBB30_4
// %bb.2:
	ldrb	w9, [x8, #16]
	tbz	w9, #0, .LBB30_4
// %bb.3:
	ldrb	w8, [x8, #320]
	cbnz	w8, .LBB30_8
.LBB30_4:
	ldr	w8, [x19, #84]
	tbnz	w8, #11, .LBB30_6
// %bb.5:
	and	w8, w8, #0xfc
	orr	w8, w8, #0x3
	cmp	w8, #35                 // =35
	b.ne	.LBB30_9
.LBB30_6:
	mov	x0, x19
	mov	x1, x20
	bl	ufs_mq_insert_sync_list
	b	.LBB30_8
.LBB30_7:
	mov	x0, x19
	mov	x1, x20
	bl	ufs_mq_req_insert
.LBB30_8:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldp	x22, x21, [sp], #48     // 16-byte Folded Reload
	ret
.LBB30_9:
	ldr	x8, [x20, #1816]
	cbz	x8, .LBB30_8
// %bb.10:
	ldr	x22, [x8, #176]
	cbz	x22, .LBB30_8
// %bb.11:
	ldr	x8, [x22]
	ldr	x8, [x8, #32]
	cbz	x8, .LBB30_8
// %bb.12:
	ldr	x8, [x20, #1808]
	mov	w1, #9
	mov	x0, x19
	ldr	x20, [x8, #280]
	bl	mas_blk_latency_req_check
	add	x21, x20, #120          // =120
	mov	x0, x21
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x22]
	mov	x22, x0
	mov	x0, x19
	mov	x1, x20
	ldr	x8, [x8, #32]
	blr	x8
	add	x8, x20, #168           // =168
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	mov	x0, x21
	mov	x1, x22
	bl	_raw_spin_unlock_irqrestore
	b	.LBB30_8
.Lfunc_end30:
	.size	ufs_mq_req_requeue, .Lfunc_end30-ufs_mq_req_requeue
                                        // -- End function
	.globl	ufs_mq_req_timeout_handler // -- Begin function ufs_mq_req_timeout_handler
	.p2align	2
	.type	ufs_mq_req_timeout_handler,@function
ufs_mq_req_timeout_handler:             // @ufs_mq_req_timeout_handler
// %bb.0:
	stp	x22, x21, [sp, #-48]!   // 16-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	ldr	x8, [x0, #64]
	mov	x20, x0
	add	x29, sp, #32            // =32
	ldr	x9, [x8, #1808]
	ldr	x22, [x8, #312]
	ldr	x21, [x9, #280]
	ldr	x8, [x20, #96]!
	tbz	w8, #1, .LBB31_21
// %bb.1:
	ldr	w8, [x0, #308]
	mov	x19, x0
	add	x8, x21, x8, lsl #2
	add	x8, x8, #396            // =396
	//APP
	// atomic_sub
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	ldr	w8, [x0, #84]
	and	w8, w8, #0xfc
	orr	w8, w8, #0x3
	cmp	w8, #35                 // =35
	b.eq	.LBB31_3
// %bb.2:
	orr	w2, wzr, #0x1
	mov	x0, x19
	mov	x1, x21
	bl	ufs_mq_rq_inflt_update
.LBB31_3:
	ldr	x8, [x22, #8]
	cbz	x8, .LBB31_8
// %bb.4:
	mov	x0, x19
	mov	w1, wzr
	blr	x8
	cmp	w0, #3                  // =3
	b.eq	.LBB31_18
// %bb.5:
	cmp	w0, #2                  // =2
	b.eq	.LBB31_8
// %bb.6:
	cmp	w0, #1                  // =1
	b.ne	.LBB31_21
// %bb.7:
	ldr	x1, [x19, #64]
	mov	x0, x19
	mov	w2, wzr
	bl	ufs_mq_req_complete
	b	.LBB31_21
.LBB31_8:
	ldr	w8, [x19, #308]
	add	x8, x21, x8, lsl #2
	add	x8, x8, #396            // =396
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	ldr	w8, [x19, #84]
	and	w9, w8, #0xfc
	orr	w9, w9, #0x3
	cmp	w9, #35                 // =35
	b.eq	.LBB31_20
// %bb.9:
	tbz	w8, #11, .LBB31_22
// %bb.10:
	mov	w9, #4096
	movk	w9, #64, lsl #16
	tst	w8, w9
	b.eq	.LBB31_14
// %bb.11:
	mov	w8, w8
	tbnz	w8, #23, .LBB31_17
.LBB31_12:
	mov	w9, #4096
	movk	w9, #64, lsl #16
	and	x8, x8, x9
	cbz	x8, .LBB31_17
// %bb.13:
	add	x8, x21, #44            // =44
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	b	.LBB31_20
.LBB31_14:
	ldrb	w8, [x19, #232]
	tbz	w8, #1, .LBB31_19
// %bb.15:
	ldr	x0, [x19, #64]
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB31_19
// %bb.16:
	ldr	w8, [x19, #84]
	mov	w8, w8
	tbz	w8, #23, .LBB31_12
.LBB31_17:
	add	x8, x21, #40            // =40
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	b	.LBB31_20
.LBB31_18:
	orr	w1, wzr, #0x1
	mov	x0, x19
	bl	blk_mq_requeue_request
	ldr	x0, [x19, #64]
	bl	blk_mq_kick_requeue_list
	b	.LBB31_21
.LBB31_19:
	add	x8, x21, #32            // =32
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
.LBB31_20:
	mov	x0, x19
	bl	blk_add_timer
	mov	w0, wzr
	mov	x1, x20
	bl	clear_bit
.LBB31_21:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldp	x22, x21, [sp], #48     // 16-byte Folded Reload
	ret
.LBB31_22:
	add	x8, x21, #36            // =36
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	b	.LBB31_20
.Lfunc_end31:
	.size	ufs_mq_req_timeout_handler, .Lfunc_end31-ufs_mq_req_timeout_handler
                                        // -- End function
	.globl	ufs_mq_ctx_put          // -- Begin function ufs_mq_ctx_put
	.p2align	2
	.type	ufs_mq_ctx_put,@function
ufs_mq_ctx_put:                         // @ufs_mq_ctx_put
// %bb.0:
	//APP
	//NO_APP
	//APP
	mrs x8, sp_el0
	//NO_APP
	ldr	w9, [x8, #16]
	sub	w9, w9, #1              // =1
	str	w9, [x8, #16]
	ret
.Lfunc_end32:
	.size	ufs_mq_ctx_put, .Lfunc_end32-ufs_mq_ctx_put
                                        // -- End function
	.globl	ufs_mq_hctx_get_by_req  // -- Begin function ufs_mq_hctx_get_by_req
	.p2align	2
	.type	ufs_mq_hctx_get_by_req,@function
ufs_mq_hctx_get_by_req:                 // @ufs_mq_hctx_get_by_req
// %bb.0:
	ldr	x8, [x0, #288]
	ldr	x9, [x0, #64]
	ldrsw	x8, [x8, #64]
	ldr	x10, [x9, #320]
	ldr	x9, [x9, #344]
	ldr	w8, [x10, x8, lsl #2]
	ldr	x8, [x9, x8, lsl #3]
	str	x8, [x1]
	ret
.Lfunc_end33:
	.size	ufs_mq_hctx_get_by_req, .Lfunc_end33-ufs_mq_hctx_get_by_req
                                        // -- End function
	.globl	ufs_mq_exec_queue       // -- Begin function ufs_mq_exec_queue
	.p2align	2
	.type	ufs_mq_exec_queue,@function
ufs_mq_exec_queue:                      // @ufs_mq_exec_queue
// %bb.0:
	stp	x22, x21, [sp, #-48]!   // 16-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	mov	x19, x0
	bl	ufs_mq_sync_dispatch
	ldr	x8, [x19, #1816]
	ldr	x20, [x19, #1808]
	cbz	x8, .LBB34_5
// %bb.1:
	ldr	x22, [x8, #176]
	cbz	x22, .LBB34_5
// %bb.2:
	ldr	x8, [x22]
	ldr	x8, [x8, #48]
	cbz	x8, .LBB34_5
// %bb.3:
	ldr	x20, [x20, #280]
	add	x21, x20, #120          // =120
	mov	x0, x21
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x22]
	mov	x22, x0
	mov	x0, x20
	ldr	x8, [x8, #48]
	blr	x8
	mov	w20, w0
	mov	x0, x21
	mov	x1, x22
	bl	_raw_spin_unlock_irqrestore
	tbnz	w20, #0, .LBB34_6
// %bb.4:
	ldr	x20, [x19, #1808]
.LBB34_5:
	adrp	x8, mas_blk_mq_async_disp_wq
	ldr	x1, [x8, :lo12:mas_blk_mq_async_disp_wq]
	add	x2, x20, #160           // =160
	orr	w0, wzr, #0x8
	mov	x3, xzr
	bl	queue_delayed_work_on
	bl	ktime_get
	str	x0, [x20, #136]
.LBB34_6:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldp	x22, x21, [sp], #48     // 16-byte Folded Reload
	ret
.Lfunc_end34:
	.size	ufs_mq_exec_queue, .Lfunc_end34-ufs_mq_exec_queue
                                        // -- End function
	.globl	ufs_mq_run_hw_queue     // -- Begin function ufs_mq_run_hw_queue
	.p2align	2
	.type	ufs_mq_run_hw_queue,@function
ufs_mq_run_hw_queue:                    // @ufs_mq_run_hw_queue
// %bb.0:
	stp	x22, x21, [sp, #-48]!   // 16-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	ldr	x20, [x0, #1808]
	adrp	x8, mas_blk_mq_sync_disp_wq
	ldr	x1, [x8, :lo12:mas_blk_mq_sync_disp_wq]
	mov	x19, x0
	add	x2, x20, #32            // =32
	orr	w0, wzr, #0x8
	mov	x3, xzr
	add	x29, sp, #32            // =32
	bl	queue_delayed_work_on
	bl	ktime_get
	str	x0, [x20, #8]
	ldr	x8, [x19, #1816]
	ldr	x20, [x19, #1808]
	cbz	x8, .LBB35_5
// %bb.1:
	ldr	x22, [x8, #176]
	cbz	x22, .LBB35_5
// %bb.2:
	ldr	x8, [x22]
	ldr	x8, [x8, #48]
	cbz	x8, .LBB35_5
// %bb.3:
	ldr	x20, [x20, #280]
	add	x21, x20, #120          // =120
	mov	x0, x21
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x22]
	mov	x22, x0
	mov	x0, x20
	ldr	x8, [x8, #48]
	blr	x8
	mov	w20, w0
	mov	x0, x21
	mov	x1, x22
	bl	_raw_spin_unlock_irqrestore
	tbnz	w20, #0, .LBB35_6
// %bb.4:
	ldr	x20, [x19, #1808]
.LBB35_5:
	adrp	x8, mas_blk_mq_async_disp_wq
	ldr	x1, [x8, :lo12:mas_blk_mq_async_disp_wq]
	add	x2, x20, #160           // =160
	orr	w0, wzr, #0x8
	mov	x3, xzr
	bl	queue_delayed_work_on
	bl	ktime_get
	str	x0, [x20, #136]
.LBB35_6:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldp	x22, x21, [sp], #48     // 16-byte Folded Reload
	ret
.Lfunc_end35:
	.size	ufs_mq_run_hw_queue, .Lfunc_end35-ufs_mq_run_hw_queue
                                        // -- End function
	.globl	ufs_mq_run_requeue      // -- Begin function ufs_mq_run_requeue
	.p2align	2
	.type	ufs_mq_run_requeue,@function
ufs_mq_run_requeue:                     // @ufs_mq_run_requeue
// %bb.0:
	stp	x22, x21, [sp, #-48]!   // 16-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	ldr	x20, [x0, #1808]
	adrp	x8, mas_blk_mq_sync_disp_wq
	ldr	x1, [x8, :lo12:mas_blk_mq_sync_disp_wq]
	mov	x19, x0
	add	x2, x20, #32            // =32
	orr	w0, wzr, #0x8
	mov	x3, xzr
	add	x29, sp, #32            // =32
	bl	queue_delayed_work_on
	bl	ktime_get
	str	x0, [x20, #8]
	ldr	x8, [x19, #1816]
	ldr	x20, [x19, #1808]
	cbz	x8, .LBB36_5
// %bb.1:
	ldr	x22, [x8, #176]
	cbz	x22, .LBB36_5
// %bb.2:
	ldr	x8, [x22]
	ldr	x8, [x8, #48]
	cbz	x8, .LBB36_5
// %bb.3:
	ldr	x20, [x20, #280]
	add	x21, x20, #120          // =120
	mov	x0, x21
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x22]
	mov	x22, x0
	mov	x0, x20
	ldr	x8, [x8, #48]
	blr	x8
	mov	w20, w0
	mov	x0, x21
	mov	x1, x22
	bl	_raw_spin_unlock_irqrestore
	tbnz	w20, #0, .LBB36_6
// %bb.4:
	ldr	x20, [x19, #1808]
.LBB36_5:
	adrp	x8, mas_blk_mq_async_disp_wq
	ldr	x1, [x8, :lo12:mas_blk_mq_async_disp_wq]
	add	x2, x20, #160           // =160
	orr	w0, wzr, #0x8
	mov	x3, xzr
	bl	queue_delayed_work_on
	bl	ktime_get
	str	x0, [x20, #136]
.LBB36_6:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldp	x22, x21, [sp], #48     // 16-byte Folded Reload
	ret
.Lfunc_end36:
	.size	ufs_mq_run_requeue, .Lfunc_end36-ufs_mq_run_requeue
                                        // -- End function
	.globl	ufs_mq_poll_enable      // -- Begin function ufs_mq_poll_enable
	.p2align	2
	.type	ufs_mq_poll_enable,@function
ufs_mq_poll_enable:                     // @ufs_mq_poll_enable
// %bb.0:
	stp	x24, x23, [sp, #-64]!   // 16-byte Folded Spill
	adrp	x23, __cpu_online_mask
	stp	x22, x21, [sp, #16]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	mov	x19, x0
	mov	w21, wzr
	mov	w20, wzr
	adrp	x22, __cpu_possible_mask
	add	x23, x23, :lo12:__cpu_online_mask
	orr	w24, wzr, #0x1
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	add	x29, sp, #48            // =48
	b	.LBB37_2
.LBB37_1:                               //   in Loop: Header=BB37_2 Depth=1
	add	w8, w21, #63            // =63
	cmp	w21, #0                 // =0
	csel	w8, w8, w21, lt
	asr	w8, w8, #6
	ldr	x8, [x23, w8, sxtw #3]
	lsl	x9, x24, x21
	add	w21, w21, #1            // =1
	tst	x8, x9
	cinc	w20, w20, ne
.LBB37_2:                               // =>This Inner Loop Header: Depth=1
	ldrb	w0, [x22, :lo12:__cpu_possible_mask]
	bl	__sw_hweight64
	cmp	w21, w0
	b.lo	.LBB37_1
// %bb.3:
	cmp	w20, #1                 // =1
	cset	w8, hi
	strb	w8, [x19]
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]     // 16-byte Folded Reload
	ldp	x24, x23, [sp], #64     // 16-byte Folded Reload
	ret
.Lfunc_end37:
	.size	ufs_mq_poll_enable, .Lfunc_end37-ufs_mq_poll_enable
                                        // -- End function
	.globl	ufs_order_panic_wait_datasync_handle // -- Begin function ufs_order_panic_wait_datasync_handle
	.p2align	2
	.type	ufs_order_panic_wait_datasync_handle,@function
ufs_order_panic_wait_datasync_handle:   // @ufs_order_panic_wait_datasync_handle
// %bb.0:
	ldr	x8, [x0, #1680]
	ldr	w9, [x8, #24]
	ldr	w10, [x8, #16]
	add	w9, w10, w9
	cmp	w9, #1                  // =1
	cset	w0, lt
	cmp	w9, #0                  // =0
	cset	w9, gt
	strb	w9, [x8, #576]
	ret
.Lfunc_end38:
	.size	ufs_order_panic_wait_datasync_handle, .Lfunc_end38-ufs_order_panic_wait_datasync_handle
                                        // -- End function
	.globl	ufs_order_panic_datasync_handle // -- Begin function ufs_order_panic_datasync_handle
	.p2align	2
	.type	ufs_order_panic_datasync_handle,@function
ufs_order_panic_datasync_handle:        // @ufs_order_panic_datasync_handle
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	ldr	x8, [x0, #1680]
	orr	w9, wzr, #0x1
	orr	w0, wzr, #0x8
	mov	x3, xzr
	strb	w9, [x8, #576]
	adrp	x9, mas_blk_mq_sync_disp_wq
	ldr	x1, [x9, :lo12:mas_blk_mq_sync_disp_wq]
	add	x2, x8, #480            // =480
	mov	x29, sp
	bl	queue_delayed_work_on
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end39:
	.size	ufs_order_panic_datasync_handle, .Lfunc_end39-ufs_order_panic_datasync_handle
                                        // -- End function
	.globl	ufs_tagset_power_off_proc // -- Begin function ufs_tagset_power_off_proc
	.p2align	2
	.type	ufs_tagset_power_off_proc,@function
ufs_tagset_power_off_proc:              // @ufs_tagset_power_off_proc
// %bb.0:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	ldr	x19, [x0, #1680]
	orr	w8, wzr, #0x1
	orr	w0, wzr, #0x8
	mov	x3, xzr
	strb	w8, [x19, #576]
	adrp	x8, mas_blk_mq_sync_disp_wq
	ldr	x1, [x8, :lo12:mas_blk_mq_sync_disp_wq]
	add	x2, x19, #480           // =480
	add	x29, sp, #16            // =16
	bl	queue_delayed_work_on
	adrp	x8, jiffies
	ldr	x8, [x8, :lo12:jiffies]
	add	x0, x19, #432           // =432
	add	x1, x8, #125            // =125
	bl	mod_timer
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldr	x19, [sp], #32          // 8-byte Folded Reload
	ret
.Lfunc_end40:
	.size	ufs_tagset_power_off_proc, .Lfunc_end40-ufs_tagset_power_off_proc
                                        // -- End function
	.globl	ufs_mq_status_dump      // -- Begin function ufs_mq_status_dump
	.p2align	2
	.type	ufs_mq_status_dump,@function
ufs_mq_status_dump:                     // @ufs_mq_status_dump
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	ldr	x8, [x0, #1808]
	adrp	x9, .L.str.14
	adrp	x10, .L.str.13
	add	x9, x9, :lo12:.L.str.14
	add	x10, x10, :lo12:.L.str.13
	cmp	w1, #1                  // =1
	csel	x19, x10, x9, eq
	add	x29, sp, #16            // =16
	cbz	x8, .LBB41_3
// %bb.1:
	ldr	x20, [x8, #280]
	cbz	x20, .LBB41_3
// %bb.2:
	ldr	w2, [x20, #28]
	adrp	x0, .L.str.4
	add	x0, x0, :lo12:.L.str.4
	mov	x1, x19
	bl	printk
	ldr	w2, [x20, #24]
	ldr	w3, [x20, #16]
	ldr	w4, [x20, #20]
	adrp	x0, .L.str.5
	add	x0, x0, :lo12:.L.str.5
	mov	x1, x19
	bl	printk
	ldr	w2, [x20, #44]
	ldr	w3, [x20, #40]
	ldr	w4, [x20, #32]
	ldr	w5, [x20, #36]
	ldr	w6, [x20, #12]
	adrp	x0, .L.str.6
	add	x0, x0, :lo12:.L.str.6
	mov	x1, x19
	bl	printk
.LBB41_3:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end41:
	.size	ufs_mq_status_dump, .Lfunc_end41-ufs_mq_status_dump
                                        // -- End function
	.globl	ufs_mq_iosched_init     // -- Begin function ufs_mq_iosched_init
	.p2align	2
	.type	ufs_mq_iosched_init,@function
ufs_mq_iosched_init:                    // @ufs_mq_iosched_init
// %bb.0:
	stp	x26, x25, [sp, #-80]!   // 16-byte Folded Spill
	adrp	x8, kmalloc_caches+72
	ldr	x8, [x8, :lo12:kmalloc_caches+72]
	mov	w1, #32960
	stp	x20, x19, [sp, #48]     // 16-byte Folded Spill
	mov	x20, x0
	movk	w1, #320, lsl #16
	mov	w2, #288
	mov	x0, x8
	stp	x24, x23, [sp, #16]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #32]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #64]     // 16-byte Folded Spill
	add	x29, sp, #64            // =64
	bl	kmem_cache_alloc_trace
	cbz	x0, .LBB42_8
// %bb.1:
	mov	x19, x0
	mov	x0, x20
	bl	mas_blk_get_lld
	ldrb	w8, [x0, #1672]
	mov	x22, x0
	cbz	w8, .LBB42_9
// %bb.2:
	ldr	x8, [x22, #1680]
	add	x8, x8, #428            // =428
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	ldr	x21, [x22, #1680]
	cbz	x21, .LBB42_23
.LBB42_3:
	ldr	x8, [x20, #1816]
	cbz	x8, .LBB42_18
.LBB42_4:
	ldr	x23, [x8, #176]
	cbz	x23, .LBB42_18
// %bb.5:
	ldr	x8, [x23]
	ldr	x8, [x8, #8]
	cbz	x8, .LBB42_7
// %bb.6:
	add	x22, x21, #120          // =120
	mov	x0, x22
	bl	_raw_spin_lock_irqsave
	ldr	x8, [x23]
	mov	x23, x0
	mov	x0, x21
	ldr	x8, [x8, #8]
	blr	x8
	mov	x0, x22
	mov	x1, x23
	bl	_raw_spin_unlock_irqrestore
.LBB42_7:
	adrp	x9, __cfi_ufs_mq_sync_io_dispatch_work_fn
	mov	x22, #68719476704
	mov	x23, x19
	add	x8, x19, #40            // =40
	add	x9, x9, :lo12:__cfi_ufs_mq_sync_io_dispatch_work_fn
	add	x0, x19, #64            // =64
	orr	w1, wzr, #0x200000
	mov	x2, xzr
	mov	x3, xzr
	str	x22, [x23, #32]!
	str	x8, [x19, #40]
	stp	x8, x9, [x19, #48]
	bl	init_timer_key
	adrp	x24, delayed_work_timer_fn
	adrp	x9, __cfi_ufs_mq_async_io_dispatch_work_fn
	add	x24, x24, :lo12:delayed_work_timer_fn
	mov	x25, x19
	add	x8, x19, #168           // =168
	add	x9, x9, :lo12:__cfi_ufs_mq_async_io_dispatch_work_fn
	add	x0, x19, #192           // =192
	orr	w1, wzr, #0x200000
	mov	x2, xzr
	mov	x3, xzr
	str	x20, [x19, #128]
	stp	x24, x23, [x19, #88]
	str	x22, [x25, #160]!
	str	x8, [x19, #168]
	stp	x8, x9, [x19, #176]
	bl	init_timer_key
	str	x21, [x19, #280]
	adrp	x21, io_guard_queue_list_lock
	add	x21, x21, :lo12:io_guard_queue_list_lock
	mov	x0, x21
	stp	x24, x25, [x19, #216]
	str	x20, [x19, #256]
	str	x20, [x19]
	str	x19, [x20, #1808]
	bl	_raw_spin_lock
	adrp	x8, mas_io_guard_queue_list
	add	x8, x8, :lo12:mas_io_guard_queue_list
	ldr	x9, [x8, #8]
	add	x10, x19, #264          // =264
	mov	x0, x21
	str	x10, [x8, #8]
	stp	x8, x9, [x19, #264]
	str	x10, [x9]
	bl	_raw_spin_unlock
	adrp	x8, mas_blk_io_guard_wq
	ldr	x1, [x8, :lo12:mas_blk_io_guard_wq]
	adrp	x2, mas_io_guard_work
	add	x2, x2, :lo12:mas_io_guard_work
	orr	w0, wzr, #0x8
	mov	w3, #500
	bl	queue_delayed_work_on
	adrp	x1, __cfi_ufs_mq_make_request
	add	x1, x1, :lo12:__cfi_ufs_mq_make_request
	mov	x0, x20
	bl	blk_queue_make_request
	mov	w0, wzr
	b	.LBB42_25
.LBB42_8:
	adrp	x0, .L.str.7
	adrp	x1, .L__func__.ufs_mq_iosched_init
	add	x0, x0, :lo12:.L.str.7
	add	x1, x1, :lo12:.L__func__.ufs_mq_iosched_init
	mov	w2, #3355
	bl	printk
	b	.LBB42_24
.LBB42_9:
	adrp	x8, kmalloc_caches+80
	ldr	x0, [x8, :lo12:kmalloc_caches+80]
	mov	w1, #32960
	movk	w1, #320, lsl #16
	mov	w2, #584
	bl	kmem_cache_alloc_trace
	cbz	x0, .LBB42_23
// %bb.10:
	orr	w8, wzr, #0x1
	mov	x21, x0
	str	w8, [x0, #428]
	add	x0, x0, #432            // =432
	mov	w1, wzr
	mov	x2, xzr
	mov	x3, xzr
	bl	init_timer_key
	adrp	x8, ufs_turbo_check_timer_expire
	adrp	x11, ufs_datasync_work
	add	x8, x8, :lo12:ufs_turbo_check_timer_expire
	mov	x9, #68719476704
	add	x10, x21, #488          // =488
	add	x11, x11, :lo12:ufs_datasync_work
	add	x0, x21, #512           // =512
	orr	w1, wzr, #0x200000
	mov	x2, xzr
	mov	x3, xzr
	strb	wzr, [x21, #576]
	add	x24, x21, #480          // =480
	stp	x8, x21, [x21, #456]
	str	x9, [x21, #480]
	str	x10, [x21, #488]
	stp	x10, x11, [x21, #496]
	bl	init_timer_key
	adrp	x23, delayed_work_timer_fn
	adrp	x1, .L.str.19
	adrp	x2, ufs_mq_dispatch_list_init.__key
	add	x23, x23, :lo12:delayed_work_timer_fn
	add	x8, x21, #72            // =72
	add	x9, x21, #96            // =96
	add	x0, x21, #48            // =48
	add	x1, x1, :lo12:.L.str.19
	add	x2, x2, :lo12:ufs_mq_dispatch_list_init.__key
	str	x24, [x21, #544]
	str	x23, [x21, #536]
	str	wzr, [x21, #16]
	str	wzr, [x21, #20]
	str	wzr, [x21, #24]
	str	wzr, [x21, #28]
	str	x8, [x21, #72]
	str	x8, [x21, #80]
	str	x9, [x21, #96]
	str	x9, [x21, #104]
	bl	__raw_spin_lock_init
	adrp	x1, .L.str.21
	adrp	x2, ufs_mq_dispatch_list_init.__key.20
	add	x0, x21, #120           // =120
	add	x1, x1, :lo12:.L.str.21
	add	x2, x2, :lo12:ufs_mq_dispatch_list_init.__key.20
	bl	__raw_spin_lock_init
	adrp	x1, .L.str.23
	adrp	x2, ufs_mq_dispatch_list_init.__key.22
	add	x8, x21, #176           // =176
	add	x0, x21, #192           // =192
	add	x1, x1, :lo12:.L.str.23
	add	x2, x2, :lo12:ufs_mq_dispatch_list_init.__key.22
	str	x8, [x21, #176]
	str	x8, [x21, #184]
	bl	__raw_spin_lock_init
	mov	x9, #28
	mov	w8, #5
	movk	x9, #14, lsl #32
	str	wzr, [x21, #88]
	str	wzr, [x21, #112]
	str	wzr, [x21, #168]
	str	w8, [x21, #8]
	str	x9, [x21, #216]
	str	wzr, [x21, #12]
	str	wzr, [x21, #36]
	str	wzr, [x21, #44]
	str	wzr, [x21, #40]
	str	wzr, [x21, #32]
	bl	ktime_get
	str	x0, [x21, #224]
	bl	ktime_get
	str	x0, [x21, #232]
	bl	ktime_get
	str	x0, [x21, #240]
	bl	ktime_get
	str	x0, [x21, #248]
	bl	ktime_get
	mov	w24, #99
	adrp	x25, __cpu_possible_mask
	str	x0, [x21, #256]
	b	.LBB42_12
.LBB42_11:                              //   in Loop: Header=BB42_12 Depth=1
	str	wzr, [x21, x24, lsl #2]
	add	x24, x24, #1            // =1
.LBB42_12:                              // =>This Inner Loop Header: Depth=1
	ldrb	w0, [x25, :lo12:__cpu_possible_mask]
	sub	x26, x24, #99           // =99
	bl	__sw_hweight64
	cmp	x26, w0, uxtw
	b.lo	.LBB42_11
// %bb.13:
	add	x0, x21, #264           // =264
	mov	w1, wzr
	mov	x2, xzr
	mov	x3, xzr
	bl	init_timer_key
	adrp	x8, __cfi_ufs_mq_sync_burst_check_timer_expire
	add	x8, x8, :lo12:__cfi_ufs_mq_sync_burst_check_timer_expire
	add	x0, x21, #320           // =320
	mov	w1, wzr
	mov	x2, xzr
	mov	x3, xzr
	stp	x8, x21, [x21, #288]
	bl	init_timer_key
	adrp	x8, __cfi_ufs_mq_write_throttle_check_timer_expire
	adrp	x1, .L.str.15
	adrp	x2, ufs_mq_sched_ds_lld_init.__key
	add	x8, x8, :lo12:__cfi_ufs_mq_write_throttle_check_timer_expire
	add	x0, x21, #368           // =368
	add	x1, x1, :lo12:.L.str.15
	add	x2, x2, :lo12:ufs_mq_sched_ds_lld_init.__key
	stp	x8, x21, [x21, #344]
	bl	__raw_spin_lock_init
	adrp	x0, .L.str.24
	orr	w8, wzr, #0x1
	add	x0, x0, :lo12:.L.str.24
	orr	w1, wzr, #0x10
	mov	w2, wzr
	mov	x3, xzr
	mov	x4, xzr
	strb	w8, [x21, #392]
	str	wzr, [x21, #312]
	str	wzr, [x21, #316]
	bl	__alloc_workqueue_key
	adrp	x24, mas_blk_mq_sync_disp_wq
	str	x0, [x24, :lo12:mas_blk_mq_sync_disp_wq]
	cbz	x0, .LBB42_19
// %bb.14:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	orr	w1, wzr, #0x2
	mov	w2, wzr
	mov	x3, xzr
	mov	x4, xzr
	bl	__alloc_workqueue_key
	adrp	x25, mas_blk_mq_async_disp_wq
	str	x0, [x25, :lo12:mas_blk_mq_async_disp_wq]
	cbz	x0, .LBB42_21
// %bb.15:
	adrp	x0, .L.str.27
	add	x0, x0, :lo12:.L.str.27
	orr	w1, wzr, #0x6
	mov	w2, wzr
	mov	x3, xzr
	mov	x4, xzr
	bl	__alloc_workqueue_key
	adrp	x8, mas_blk_io_guard_wq
	str	x0, [x8, :lo12:mas_blk_io_guard_wq]
	cbz	x0, .LBB42_20
// %bb.16:
	adrp	x24, mas_io_guard_work
	add	x24, x24, :lo12:mas_io_guard_work
	mov	x8, #68719476704
	adrp	x9, __cfi_ufs_mq_io_guard_work_fn
	add	x9, x9, :lo12:__cfi_ufs_mq_io_guard_work_fn
	str	x8, [x24]
	add	x8, x24, #8             // =8
	add	x0, x24, #32            // =32
	orr	w1, wzr, #0x200000
	mov	x2, xzr
	mov	x3, xzr
	str	x8, [x24, #8]
	stp	x8, x9, [x24, #16]
	bl	init_timer_key
	stp	x23, x24, [x24, #56]
	str	x22, [x21]
	ldr	w8, [x22, #4]
	orr	w9, wzr, #0x1
	str	x21, [x22, #1680]
	strb	w9, [x22, #1672]
	cmp	w8, #2                  // =2
	b.ne	.LBB42_3
// %bb.17:
	add	x0, x22, #1656          // =1656
	bl	mas_blk_flush_list_register
	ldr	x8, [x20, #1816]
	cbnz	x8, .LBB42_4
.LBB42_18:
	mov	x0, x20
	bl	ufs_mq_sched_ds_lld_exit
	b	.LBB42_23
.LBB42_19:
	adrp	x0, .L.str.25
	adrp	x1, .L__func__.ufs_mq_workqueue_init
	add	x0, x0, :lo12:.L.str.25
	add	x1, x1, :lo12:.L__func__.ufs_mq_workqueue_init
	mov	w2, #3198
	bl	printk
	b	.LBB42_22
.LBB42_20:
	ldr	x0, [x25, :lo12:mas_blk_mq_async_disp_wq]
	bl	destroy_workqueue
.LBB42_21:
	ldr	x0, [x24, :lo12:mas_blk_mq_sync_disp_wq]
	bl	destroy_workqueue
	adrp	x0, .L.str.28
	adrp	x1, .L__func__.ufs_mq_workqueue_init
	add	x0, x0, :lo12:.L.str.28
	add	x1, x1, :lo12:.L__func__.ufs_mq_workqueue_init
	bl	printk
.LBB42_22:
	mov	x0, x21
	bl	kfree
.LBB42_23:
	mov	x0, x19
	bl	kfree
.LBB42_24:
	mov	w0, #-12
.LBB42_25:
	ldp	x29, x30, [sp, #64]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #48]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #32]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #16]     // 16-byte Folded Reload
	ldp	x26, x25, [sp], #80     // 16-byte Folded Reload
	ret
.Lfunc_end42:
	.size	ufs_mq_iosched_init, .Lfunc_end42-ufs_mq_iosched_init
                                        // -- End function
	.p2align	2               // -- Begin function ufs_mq_sched_ds_lld_exit
	.type	ufs_mq_sched_ds_lld_exit,@function
ufs_mq_sched_ds_lld_exit:               // @ufs_mq_sched_ds_lld_exit
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	bl	mas_blk_get_lld
	ldr	w8, [x0, #4]
	ldr	x20, [x0, #1680]
	mov	x19, x0
	cmp	w8, #2                  // =2
	b.ne	.LBB43_2
// %bb.1:
	add	x0, x19, #1656          // =1656
	bl	mas_blk_flush_list_unregister
.LBB43_2:
	ldrb	w8, [x19, #1672]
	cbz	w8, .LBB43_5
// %bb.3:
	add	x8, x20, #428           // =428
	//APP
	// atomic_sub_return
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stlxr	w10, w9, [x8]
	cbnz	w10, 1b
	dmb ish
	//NO_APP
	cbnz	w9, .LBB43_5
// %bb.4:
	strb	wzr, [x19, #1672]
	adrp	x8, mas_blk_mq_async_disp_wq
	ldr	x0, [x8, :lo12:mas_blk_mq_async_disp_wq]
	bl	destroy_workqueue
	adrp	x8, mas_blk_mq_sync_disp_wq
	ldr	x0, [x8, :lo12:mas_blk_mq_sync_disp_wq]
	bl	destroy_workqueue
	adrp	x8, mas_blk_io_guard_wq
	ldr	x0, [x8, :lo12:mas_blk_io_guard_wq]
	bl	destroy_workqueue
	ldr	x0, [x19, #1680]
	bl	kfree
	str	xzr, [x19, #1680]
.LBB43_5:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end43:
	.size	ufs_mq_sched_ds_lld_exit, .Lfunc_end43-ufs_mq_sched_ds_lld_exit
                                        // -- End function
	.globl	ufs_mq_iosched_exit     // -- Begin function ufs_mq_iosched_exit
	.p2align	2
	.type	ufs_mq_iosched_exit,@function
ufs_mq_iosched_exit:                    // @ufs_mq_iosched_exit
// %bb.0:
	str	x21, [sp, #-48]!        // 8-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	ldr	x21, [x0, #1808]
	add	x29, sp, #32            // =32
	cbz	x21, .LBB44_2
// %bb.1:
	mov	x19, x0
	adrp	x0, mas_io_guard_work
	add	x0, x0, :lo12:mas_io_guard_work
	bl	cancel_delayed_work_sync
	add	x0, x21, #32            // =32
	bl	cancel_delayed_work_sync
	add	x0, x21, #160           // =160
	bl	cancel_delayed_work_sync
	adrp	x20, io_guard_queue_list_lock
	add	x20, x20, :lo12:io_guard_queue_list_lock
	mov	x0, x20
	bl	_raw_spin_lock
	ldp	x8, x9, [x21, #264]
	add	x10, x21, #264          // =264
	mov	x0, x20
	str	x9, [x8, #8]
	str	x8, [x9]
	str	x10, [x21, #264]
	str	x10, [x21, #272]
	bl	_raw_spin_unlock
	mov	x0, x19
	bl	ufs_mq_sched_ds_lld_exit
	ldr	x0, [x19, #1808]
	bl	kfree
	str	xzr, [x19, #1808]
.LBB44_2:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldr	x21, [sp], #48          // 8-byte Folded Reload
	ret
.Lfunc_end44:
	.size	ufs_mq_iosched_exit, .Lfunc_end44-ufs_mq_iosched_exit
                                        // -- End function
	.globl	blk_mq_tagset_ufs_mq_iosched_enable // -- Begin function blk_mq_tagset_ufs_mq_iosched_enable
	.p2align	2
	.type	blk_mq_tagset_ufs_mq_iosched_enable,@function
blk_mq_tagset_ufs_mq_iosched_enable:    // @blk_mq_tagset_ufs_mq_iosched_enable
// %bb.0:
	ldr	x8, [x0, #40]
	cmp	w1, #0                  // =0
	and	x9, x8, #0xffffffffffffffef
	orr	x8, x8, #0x10
	csel	x8, x9, x8, eq
	str	x8, [x0, #40]
	ret
.Lfunc_end45:
	.size	blk_mq_tagset_ufs_mq_iosched_enable, .Lfunc_end45-blk_mq_tagset_ufs_mq_iosched_enable
                                        // -- End function
	.globl	blk_mq_get_io_in_list_count // -- Begin function blk_mq_get_io_in_list_count
	.p2align	2
	.type	blk_mq_get_io_in_list_count,@function
blk_mq_get_io_in_list_count:            // @blk_mq_get_io_in_list_count
// %bb.0:
	cbz	x0, .LBB46_5
// %bb.1:
	ldr	x8, [x0, #168]
	cbz	x8, .LBB46_6
// %bb.2:
	ldr	x8, [x8, #1808]
	cbz	x8, .LBB46_7
// %bb.3:
	ldr	x8, [x8, #280]
	cbz	x8, .LBB46_8
// %bb.4:
	ldr	w9, [x8, #88]
	ldr	w10, [x8, #112]
	ldr	w8, [x8, #168]
	add	w9, w10, w9
	add	w0, w9, w8
	ret
.LBB46_5:
	mov	w0, #-1
	ret
.LBB46_6:
	mov	w0, #-1
	ret
.LBB46_7:
	mov	w0, #-1
	ret
.LBB46_8:
	mov	w0, #-1
	ret
.Lfunc_end46:
	.size	blk_mq_get_io_in_list_count, .Lfunc_end46-blk_mq_get_io_in_list_count
                                        // -- End function
	.p2align	2               // -- Begin function __ufs_mq_queue_rq_internal
	.type	__ufs_mq_queue_rq_internal,@function
__ufs_mq_queue_rq_internal:             // @__ufs_mq_queue_rq_internal
// %bb.0:
	stp	x28, x27, [sp, #-96]!   // 16-byte Folded Spill
	stp	x26, x25, [sp, #16]     // 16-byte Folded Spill
	stp	x24, x23, [sp, #32]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #48]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #64]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #80]     // 16-byte Folded Spill
	ldr	x23, [x2]
	mov	x20, x3
	mov	x19, x2
	mov	x21, x1
	ldr	x8, [x23, #288]
	mov	x22, x0
	add	x29, sp, #80            // =80
	ldr	w8, [x8, #64]
	add	x9, x3, x8, lsl #2
	ldr	w9, [x9, #396]
	cmp	w9, #8                  // =8
	b.ge	.LBB47_4
.LBB47_1:
	str	w8, [x23, #308]
	ldr	x8, [x19]
	ldr	w9, [x8, #84]
	tbnz	w9, #23, .LBB47_9
// %bb.2:
	mov	w10, #4096
	movk	w10, #64, lsl #16
	tst	x9, x10
	b.eq	.LBB47_15
// %bb.3:
	orr	w9, wzr, #0x6
	b	.LBB47_10
.LBB47_4:
	ldr	w9, [x23, #84]
	tbz	w9, #11, .LBB47_8
// %bb.5:
	mov	w10, #4096
	movk	w10, #64, lsl #16
	orr	w11, wzr, #0x7
	tst	x9, x10
	mov	w27, wzr
	add	w24, w8, #1             // =1
	cinc	w25, w11, ne
	adrp	x26, __cpu_possible_mask
.LBB47_6:                               // =>This Inner Loop Header: Depth=1
	ldrb	w0, [x26, :lo12:__cpu_possible_mask]
	bl	__sw_hweight64
	cmp	w27, w0
	b.hs	.LBB47_8
// %bb.7:                               //   in Loop: Header=BB47_6 Depth=1
	ldrb	w0, [x26, :lo12:__cpu_possible_mask]
	add	w28, w27, #1            // =1
	add	w27, w24, w27
	bl	__sw_hweight64
	udiv	w8, w27, w0
	msub	w8, w8, w0, w27
	add	x9, x20, w8, uxtw #2
	ldr	w9, [x9, #396]
	mov	w27, w28
	cmp	w9, w25
	b.ge	.LBB47_6
	b	.LBB47_1
.LBB47_8:
	mov	w21, #9
	b	.LBB47_14
.LBB47_9:
	orr	w9, wzr, #0x7
.LBB47_10:
	strb	w9, [x8, #304]
.LBB47_11:
	ldr	x8, [x19]
	mov	x0, x22
	mov	x1, x19
	ldr	w8, [x8, #308]
	add	x8, x20, x8, lsl #2
	add	x8, x8, #396            // =396
	//APP
	// atomic_add
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	add	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
	ldr	x8, [x21, #312]
	ldr	x8, [x8]
	blr	x8
	mov	w21, w0
	tst	w0, #0xff
	b.eq	.LBB47_13
// %bb.12:
	ldr	x8, [x19]
	ldr	w8, [x8, #308]
	add	x8, x20, x8, lsl #2
	add	x8, x8, #396            // =396
	//APP
	// atomic_sub
	prfm	pstl1strm, [x8]
1:	ldxr	w9, [x8]
	sub	w9, w9, #1
	stxr	w10, w9, [x8]
	cbnz	w10, 1b
	//NO_APP
.LBB47_13:
	ldr	x0, [x19]
	mov	w1, #10
	bl	mas_blk_latency_req_check
.LBB47_14:
	mov	w0, w21
	ldp	x29, x30, [sp, #80]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #64]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #48]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #32]     // 16-byte Folded Reload
	ldp	x26, x25, [sp, #16]     // 16-byte Folded Reload
	ldp	x28, x27, [sp], #96     // 16-byte Folded Reload
	ret
.LBB47_15:
	tbnz	w9, #11, .LBB47_17
// %bb.16:
	strb	wzr, [x8, #304]
	b	.LBB47_11
.LBB47_17:
	mov	w9, #5
	b	.LBB47_10
.Lfunc_end47:
	.size	__ufs_mq_queue_rq_internal, .Lfunc_end47-__ufs_mq_queue_rq_internal
                                        // -- End function
	.p2align	2               // -- Begin function ufs_mq_async_fwb_mode_inflt_lmt_calc
	.type	ufs_mq_async_fwb_mode_inflt_lmt_calc,@function
ufs_mq_async_fwb_mode_inflt_lmt_calc:   // @ufs_mq_async_fwb_mode_inflt_lmt_calc
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	ldr	w8, [x0, #312]
	mov	x19, x0
	add	x29, sp, #16            // =16
	cmp	w8, #3                  // =3
	b.hi	.LBB48_5
// %bb.1:
	adrp	x9, .LJTI48_0
	add	x9, x9, :lo12:.LJTI48_0
	adr	x10, .LBB48_2
	ldrb	w11, [x9, x8]
	add	x10, x10, x11, lsl #2
	br	x10
.LBB48_2:
	ldr	w8, [x19, #220]
	cmp	w8, #14                 // =14
	b.eq	.LBB48_13
// %bb.3:
	add	x20, x19, #368          // =368
	mov	x0, x20
	bl	_raw_spin_lock_irqsave
	ldrb	w8, [x19, #392]
	mov	x1, x0
	cbz	w8, .LBB48_12
// %bb.4:
	orr	w8, wzr, #0xe
	b	.LBB48_11
.LBB48_5:
	ldr	w8, [x19, #220]
	cmp	w8, #16                 // =16
	b.eq	.LBB48_13
// %bb.6:
	add	x20, x19, #368          // =368
	mov	x0, x20
	bl	_raw_spin_lock_irqsave
	ldrb	w8, [x19, #392]
	mov	x1, x0
	cbz	w8, .LBB48_12
// %bb.7:
	orr	w8, wzr, #0x10
	b	.LBB48_11
.LBB48_8:
	ldr	w8, [x19, #220]
	cmp	w8, #10                 // =10
	b.eq	.LBB48_13
// %bb.9:
	add	x20, x19, #368          // =368
	mov	x0, x20
	bl	_raw_spin_lock_irqsave
	ldrb	w8, [x19, #392]
	mov	x1, x0
	cbz	w8, .LBB48_12
// %bb.10:
	mov	w8, #10
.LBB48_11:
	str	w8, [x19, #220]
.LBB48_12:
	mov	x0, x20
	bl	_raw_spin_unlock_irqrestore
	bl	ktime_get
	str	x0, [x19, #256]
.LBB48_13:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end48:
	.size	ufs_mq_async_fwb_mode_inflt_lmt_calc, .Lfunc_end48-ufs_mq_async_fwb_mode_inflt_lmt_calc
	.section	.rodata,"a",@progbits
.LJTI48_0:
	.byte	(.LBB48_5-.LBB48_2)>>2
	.byte	(.LBB48_2-.LBB48_2)>>2
	.byte	(.LBB48_8-.LBB48_2)>>2
	.byte	(.LBB48_2-.LBB48_2)>>2
                                        // -- End function
	.text
	.p2align	2               // -- Begin function ufs_turbo_check_timer_expire
	.type	ufs_turbo_check_timer_expire,@function
ufs_turbo_check_timer_expire:           // @ufs_turbo_check_timer_expire
// %bb.0:
	sub	sp, sp, #64             // =64
	str	x21, [sp, #16]          // 8-byte Folded Spill
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	adrp	x20, ufs_turbo_check_timer_expire.check_count
	ldrb	w3, [x0, #576]
	ldr	w2, [x20, :lo12:ufs_turbo_check_timer_expire.check_count]
	ldr	w4, [x0, #24]
	ldr	w5, [x0, #16]
	ldr	w6, [x0, #44]
	ldr	w7, [x0, #40]
	ldr	w8, [x0, #32]
	ldr	w9, [x0, #36]
	mov	x19, x0
	adrp	x0, .L.str.16
	adrp	x1, .L__func__.ufs_turbo_check_timer_expire
	add	x0, x0, :lo12:.L.str.16
	add	x1, x1, :lo12:.L__func__.ufs_turbo_check_timer_expire
	add	x29, sp, #48            // =48
	str	w9, [sp, #8]
	str	w8, [sp]
	bl	printk
	ldr	x0, [x19]
	ldr	x21, [x0, #40]
	bl	mas_blk_get_queue_by_lld
	mov	w1, wzr
	blr	x21
	ldr	w8, [x20, :lo12:ufs_turbo_check_timer_expire.check_count]
	cbz	w8, .LBB49_3
// %bb.1:
	ldr	w9, [x19, #24]
	ldr	w10, [x19, #16]
	add	w9, w10, w9
	cmp	w9, #0                  // =0
	b.le	.LBB49_4
// %bb.2:
	orr	w9, wzr, #0x1
	b	.LBB49_5
.LBB49_3:
	mov	w8, #18
	strb	wzr, [x19, #576]
	str	w8, [x20, :lo12:ufs_turbo_check_timer_expire.check_count]
	b	.LBB49_6
.LBB49_4:
	orr	w0, wzr, #0x1
	bl	blk_power_off_flush
	ldr	w8, [x20, :lo12:ufs_turbo_check_timer_expire.check_count]
	mov	w9, wzr
.LBB49_5:
	sub	w8, w8, #1              // =1
	strb	w9, [x19, #576]
	str	w8, [x20, :lo12:ufs_turbo_check_timer_expire.check_count]
	adrp	x8, jiffies
	ldr	x8, [x8, :lo12:jiffies]
	add	x0, x19, #432           // =432
	add	x1, x8, #125            // =125
	bl	mod_timer
.LBB49_6:
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldr	x21, [sp, #16]          // 8-byte Folded Reload
	add	sp, sp, #64             // =64
	ret
.Lfunc_end49:
	.size	ufs_turbo_check_timer_expire, .Lfunc_end49-ufs_turbo_check_timer_expire
                                        // -- End function
	.p2align	2               // -- Begin function ufs_datasync_work
	.type	ufs_datasync_work,@function
ufs_datasync_work:                      // @ufs_datasync_work
// %bb.0:
	sub	sp, sp, #32             // =32
	adrp	x8, __stack_chk_guard
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	adrp	x0, .L.str.17
	orr	w9, wzr, #0x1
	add	x0, x0, :lo12:.L.str.17
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	str	x8, [sp, #8]
	str	w9, [sp, #4]
	bl	printk
	adrp	x0, ufs_sync_fs
	add	x0, x0, :lo12:ufs_sync_fs
	add	x1, sp, #4              // =4
	bl	iterate_supers
	adrp	x0, .L.str.18
	add	x0, x0, :lo12:.L.str.18
	bl	printk
	adrp	x9, __stack_chk_guard
	ldr	x8, [sp, #8]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB50_2
// %bb.1:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	add	sp, sp, #32             // =32
	ret
.LBB50_2:
	bl	__stack_chk_fail
.Lfunc_end50:
	.size	ufs_datasync_work, .Lfunc_end50-ufs_datasync_work
                                        // -- End function
	.p2align	2               // -- Begin function ufs_sync_fs
	.type	ufs_sync_fs,@function
ufs_sync_fs:                            // @ufs_sync_fs
// %bb.0:
	ldrb	w8, [x0, #80]
	tbnz	w8, #0, .LBB51_3
// %bb.1:
	ldr	x8, [x0, #48]
	ldr	x8, [x8, #56]
	cbz	x8, .LBB51_3
// %bb.2:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	ldr	w1, [x1]
	mov	x29, sp
	blr	x8
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
.LBB51_3:
	ret
.Lfunc_end51:
	.size	ufs_sync_fs, .Lfunc_end51-ufs_sync_fs
                                        // -- End function
	.p2align	2               // -- Begin function ufs_mq_async_sched_fifo_init
	.type	ufs_mq_async_sched_fifo_init,@function
ufs_mq_async_sched_fifo_init:           // @ufs_mq_async_sched_fifo_init
// %bb.0:
	ldrb	w8, [x0, #144]
	cbz	w8, .LBB52_2
// %bb.1:
	ret
.LBB52_2:
	add	x8, x0, #152            // =152
	orr	w9, wzr, #0x1
	str	x8, [x0, #152]
	str	x8, [x0, #160]
	strb	w9, [x0, #144]
	ret
.Lfunc_end52:
	.size	ufs_mq_async_sched_fifo_init, .Lfunc_end52-ufs_mq_async_sched_fifo_init
                                        // -- End function
	.p2align	2               // -- Begin function ufs_mq_async_sched_fifo_insert
	.type	ufs_mq_async_sched_fifo_insert,@function
ufs_mq_async_sched_fifo_insert:         // @ufs_mq_async_sched_fifo_insert
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	mov	x19, x0
	ldr	x0, [x0, #64]
	add	x29, sp, #16            // =16
	mov	x20, x1
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB53_2
// %bb.1:
	ldr	x8, [x20, #160]
	add	x9, x20, #152           // =152
	str	x19, [x20, #160]
	stp	x9, x8, [x19]
	b	.LBB53_3
.LBB53_2:
	ldr	x8, [x20, #160]
	add	x9, x19, #472           // =472
	add	x10, x20, #152          // =152
	str	x9, [x20, #160]
	stp	x10, x8, [x19, #472]
	mov	x19, x9
.LBB53_3:
	str	x19, [x8]
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end53:
	.size	ufs_mq_async_sched_fifo_insert, .Lfunc_end53-ufs_mq_async_sched_fifo_insert
                                        // -- End function
	.p2align	2               // -- Begin function ufs_mq_async_sched_fifo_seek
	.type	ufs_mq_async_sched_fifo_seek,@function
ufs_mq_async_sched_fifo_seek:           // @ufs_mq_async_sched_fifo_seek
// %bb.0:
	mov	x9, x0
	ldr	x8, [x9, #152]!
	cmp	x8, x9
	b.ne	.LBB54_2
// %bb.1:
	ldr	x10, [x0, #160]
	cmp	x10, x9
	b.eq	.LBB54_3
.LBB54_2:
	ldr	x9, [x0]
	ldp	x10, x11, [x8]
	sub	x12, x8, #472           // =472
	ldr	x9, [x9, #24]
	str	x11, [x10, #8]
	str	x10, [x11]
	str	x8, [x8]
	tst	x9, #0x100
	csel	x0, x12, x8, eq
	str	x8, [x8, #8]
	ret
.LBB54_3:
	mov	x0, xzr
	ret
.Lfunc_end54:
	.size	ufs_mq_async_sched_fifo_seek, .Lfunc_end54-ufs_mq_async_sched_fifo_seek
                                        // -- End function
	.p2align	2               // -- Begin function ufs_mq_async_sched_fifo_requeue
	.type	ufs_mq_async_sched_fifo_requeue,@function
ufs_mq_async_sched_fifo_requeue:        // @ufs_mq_async_sched_fifo_requeue
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	mov	x20, x0
	ldr	x0, [x0, #64]
	add	x29, sp, #16            // =16
	mov	x19, x1
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB55_2
// %bb.1:
	ldr	x8, [x19, #152]!
	str	x20, [x8, #8]
	stp	x8, x19, [x20]
	b	.LBB55_3
.LBB55_2:
	ldr	x8, [x19, #152]!
	add	x9, x20, #472           // =472
	str	x9, [x8, #8]
	stp	x8, x19, [x20, #472]
	mov	x20, x9
.LBB55_3:
	str	x20, [x19]
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end55:
	.size	ufs_mq_async_sched_fifo_requeue, .Lfunc_end55-ufs_mq_async_sched_fifo_requeue
                                        // -- End function
	.p2align	2               // -- Begin function ufs_mq_async_sched_fifo_attempt_merge_bio
	.type	ufs_mq_async_sched_fifo_attempt_merge_bio,@function
ufs_mq_async_sched_fifo_attempt_merge_bio: // @ufs_mq_async_sched_fifo_attempt_merge_bio
// %bb.0:
	str	x23, [sp, #-64]!        // 8-byte Folded Spill
	stp	x22, x21, [sp, #16]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	ldr	x8, [x1, #1808]
	mov	x20, x0
	mov	x0, x1
	add	x29, sp, #48            // =48
	ldr	x21, [x8, #280]
	mov	x19, x1
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB56_8
// %bb.1:
	ldr	x8, [x19, #1808]
	ldr	x8, [x8, #280]
	ldr	x21, [x8, #160]
	add	x22, x8, #152           // =152
	cmp	x21, x22
	b.eq	.LBB56_15
.LBB56_2:                               // =>This Inner Loop Header: Depth=1
	mov	x0, x21
	mov	x1, x20
	bl	blk_rq_merge_ok
	tbz	w0, #0, .LBB56_7
// %bb.3:                               //   in Loop: Header=BB56_2 Depth=1
	mov	x0, x21
	mov	x1, x20
	bl	blk_try_merge
	cmp	w0, #1                  // =1
	b.eq	.LBB56_6
// %bb.4:                               //   in Loop: Header=BB56_2 Depth=1
	cmp	w0, #2                  // =2
	b.ne	.LBB56_7
// %bb.5:                               //   in Loop: Header=BB56_2 Depth=1
	mov	x0, x19
	mov	x1, x21
	mov	x2, x20
	bl	bio_attempt_back_merge
	tbz	w0, #0, .LBB56_7
	b	.LBB56_17
.LBB56_6:                               //   in Loop: Header=BB56_2 Depth=1
	mov	x0, x19
	mov	x1, x21
	mov	x2, x20
	bl	bio_attempt_front_merge
	tbnz	w0, #0, .LBB56_17
.LBB56_7:                               //   in Loop: Header=BB56_2 Depth=1
	ldr	x21, [x21, #8]
	cmp	x21, x22
	b.ne	.LBB56_2
	b	.LBB56_15
.LBB56_8:
	ldr	x22, [x21, #160]
	add	x23, x21, #152          // =152
	cmp	x23, x22
	b.eq	.LBB56_15
.LBB56_9:                               // =>This Inner Loop Header: Depth=1
	sub	x21, x22, #472          // =472
	mov	x0, x21
	mov	x1, x20
	bl	blk_rq_merge_ok
	tbz	w0, #0, .LBB56_14
// %bb.10:                              //   in Loop: Header=BB56_9 Depth=1
	mov	x0, x21
	mov	x1, x20
	bl	blk_try_merge
	cmp	w0, #1                  // =1
	b.eq	.LBB56_13
// %bb.11:                              //   in Loop: Header=BB56_9 Depth=1
	cmp	w0, #2                  // =2
	b.ne	.LBB56_14
// %bb.12:                              //   in Loop: Header=BB56_9 Depth=1
	mov	x0, x19
	mov	x1, x21
	mov	x2, x20
	bl	bio_attempt_back_merge
	tbz	w0, #0, .LBB56_14
	b	.LBB56_17
.LBB56_13:                              //   in Loop: Header=BB56_9 Depth=1
	mov	x0, x19
	mov	x1, x21
	mov	x2, x20
	bl	bio_attempt_front_merge
	tbnz	w0, #0, .LBB56_17
.LBB56_14:                              //   in Loop: Header=BB56_9 Depth=1
	ldr	x22, [x22, #8]
	cmp	x23, x22
	b.ne	.LBB56_9
.LBB56_15:
	mov	w0, wzr
.LBB56_16:
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]     // 16-byte Folded Reload
	ldr	x23, [sp], #64          // 8-byte Folded Reload
	ret
.LBB56_17:
	orr	w0, wzr, #0x1
	b	.LBB56_16
.Lfunc_end56:
	.size	ufs_mq_async_sched_fifo_attempt_merge_bio, .Lfunc_end56-ufs_mq_async_sched_fifo_attempt_merge_bio
                                        // -- End function
	.p2align	2               // -- Begin function ufs_mq_async_sched_fifo_empty
	.type	ufs_mq_async_sched_fifo_empty,@function
ufs_mq_async_sched_fifo_empty:          // @ufs_mq_async_sched_fifo_empty
// %bb.0:
	ldr	x8, [x0, #152]!
	cmp	x0, x8
	cset	w0, eq
	ret
.Lfunc_end57:
	.size	ufs_mq_async_sched_fifo_empty, .Lfunc_end57-ufs_mq_async_sched_fifo_empty
                                        // -- End function
	.type	.L.str,@object          // @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	"\0013[BLK-IO]hp_sync_disp_list:\n"
	.size	.L.str, 30

	.type	.L.str.1,@object        // @.str.1
.L.str.1:
	.asciz	"\0013[BLK-IO]sync_disp_list:\n"
	.size	.L.str.1, 27

	.type	.L.str.2,@object        // @.str.2
.L.str.2:
	.asciz	"\0013[BLK-IO]async_fifo_list:\n"
	.size	.L.str.2, 28

	.type	.L__const.ufs_mq_flush_plug_list.bd,@object // @__const.ufs_mq_flush_plug_list.bd
	.section	.rodata.cst16,"aM",@progbits,16
	.p2align	3
.L__const.ufs_mq_flush_plug_list.bd:
	.xword	0
	.byte	1                       // 0x1
	.zero	7
	.size	.L__const.ufs_mq_flush_plug_list.bd, 16

	.type	io_guard_queue_list_lock,@object // @io_guard_queue_list_lock
	.data
	.p2align	3
io_guard_queue_list_lock:
	.zero	4
	.word	3735899821              // 0xdead4ead
	.word	4294967295              // 0xffffffff
	.zero	4
	.xword	-1
	.size	io_guard_queue_list_lock, 24

	.type	mas_io_guard_queue_list,@object // @mas_io_guard_queue_list
	.p2align	3
mas_io_guard_queue_list:
	.xword	mas_io_guard_queue_list
	.xword	mas_io_guard_queue_list
	.size	mas_io_guard_queue_list, 16

	.type	mas_blk_io_guard_wq,@object // @mas_blk_io_guard_wq
	.local	mas_blk_io_guard_wq
	.comm	mas_blk_io_guard_wq,8,8
	.type	mas_io_guard_work,@object // @mas_io_guard_work
	.local	mas_io_guard_work
	.comm	mas_io_guard_work,96,8
	.type	mas_blk_mq_sync_disp_wq,@object // @mas_blk_mq_sync_disp_wq
	.local	mas_blk_mq_sync_disp_wq
	.comm	mas_blk_mq_sync_disp_wq,8,8
	.type	.L.str.4,@object        // @.str.4
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.4:
	.asciz	"\0013[BLK-IO]%s: vip_wait_cnt: %d\n"
	.size	.L.str.4, 32

	.type	.L.str.5,@object        // @.str.5
.L.str.5:
	.asciz	"\0013[BLK-IO]%s: h_tag_used_cnt: %d tag_used_cnt: %d r_tag_used_cnt: %d\n"
	.size	.L.str.5, 70

	.type	.L.str.6,@object        // @.str.6
.L.str.6:
	.asciz	"\0013[BLK-IO]%s: fg_inflt: %d, vip_inflt: %d, s_inflt: %d a_inflt: %d, cp_inflt: %d\n"
	.size	.L.str.6, 82

	.type	.L.str.7,@object        // @.str.7
.L.str.7:
	.asciz	"\0013[BLK-IO]%s %d Failed to alloc sched_ds!\n"
	.size	.L.str.7, 43

	.type	.L__func__.ufs_mq_iosched_init,@object // @__func__.ufs_mq_iosched_init
.L__func__.ufs_mq_iosched_init:
	.asciz	"ufs_mq_iosched_init"
	.size	.L__func__.ufs_mq_iosched_init, 20

	.type	mas_ufs_mq_async_io_fifo_sched,@object // @mas_ufs_mq_async_io_fifo_sched
	.data
	.p2align	3
mas_ufs_mq_async_io_fifo_sched:
	.word	0                       // 0x0
	.zero	4
	.xword	ufs_mq_async_sched_fifo_init
	.xword	ufs_mq_async_sched_fifo_insert
	.xword	ufs_mq_async_sched_fifo_seek
	.xword	ufs_mq_async_sched_fifo_requeue
	.xword	ufs_mq_async_sched_fifo_attempt_merge_bio
	.xword	ufs_mq_async_sched_fifo_empty
	.size	mas_ufs_mq_async_io_fifo_sched, 56

	.type	mas_ufs_mq,@object      // @mas_ufs_mq
	.globl	mas_ufs_mq
	.p2align	3
mas_ufs_mq:
	.xword	mas_ufs_mq_async_io_fifo_sched
	.size	mas_ufs_mq, 8

	.type	.L.str.9,@object        // @.str.9
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.9:
	.asciz	"\0013[BLK-IO]lst_queue_t: %lld, lst_in_t: %lld, lst_out_t: %lld\n"
	.size	.L.str.9, 62

	.type	.L.str.11,@object       // @.str.11
.L.str.11:
	.asciz	"\0013[BLK-IO]%s - section size error: %u\n"
	.size	.L.str.11, 39

	.type	.L__func__.ufs_mq_dispatch_match_expected_lba,@object // @__func__.ufs_mq_dispatch_match_expected_lba
.L__func__.ufs_mq_dispatch_match_expected_lba:
	.asciz	"ufs_mq_dispatch_match_expected_lba"
	.size	.L__func__.ufs_mq_dispatch_match_expected_lba, 35

	.type	mas_blk_mq_async_disp_wq,@object // @mas_blk_mq_async_disp_wq
	.local	mas_blk_mq_async_disp_wq
	.comm	mas_blk_mq_async_disp_wq,8,8
	.type	.L.str.13,@object       // @.str.13
.L.str.13:
	.asciz	"dump"
	.size	.L.str.13, 5

	.type	.L.str.14,@object       // @.str.14
.L.str.14:
	.asciz	"io_latency"
	.size	.L.str.14, 11

	.type	ufs_mq_sched_ds_lld_init.__key,@object // @ufs_mq_sched_ds_lld_init.__key
	.local	ufs_mq_sched_ds_lld_init.__key
	.comm	ufs_mq_sched_ds_lld_init.__key,1,1
	.type	.L.str.15,@object       // @.str.15
.L.str.15:
	.asciz	"&(&ds_lld->async_limit_update_lock)->rlock"
	.size	.L.str.15, 43

	.type	ufs_turbo_check_timer_expire.check_count,@object // @ufs_turbo_check_timer_expire.check_count
	.data
	.p2align	2
ufs_turbo_check_timer_expire.check_count:
	.word	18                      // 0x12
	.size	ufs_turbo_check_timer_expire.check_count, 4

	.type	.L.str.16,@object       // @.str.16
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.16:
	.asciz	"\0013[BLK-IO]%s: check_count = %d turbo_mode = %d prio io = %d, sync io = %d fg_inflt: %d, vip_inflt: %d, s_inflt: %d a_inflt: %d\n"
	.size	.L.str.16, 128

	.type	.L__func__.ufs_turbo_check_timer_expire,@object // @__func__.ufs_turbo_check_timer_expire
.L__func__.ufs_turbo_check_timer_expire:
	.asciz	"ufs_turbo_check_timer_expire"
	.size	.L__func__.ufs_turbo_check_timer_expire, 29

	.type	.L.str.17,@object       // @.str.17
.L.str.17:
	.asciz	"\0013[BLK-IO]UFS Sync start\n"
	.size	.L.str.17, 26

	.type	.L.str.18,@object       // @.str.18
.L.str.18:
	.asciz	"\0013[BLK-IO]UFS Sync complete\n"
	.size	.L.str.18, 29

	.type	ufs_mq_dispatch_list_init.__key,@object // @ufs_mq_dispatch_list_init.__key
	.local	ufs_mq_dispatch_list_init.__key
	.comm	ufs_mq_dispatch_list_init.__key,1,1
	.type	.L.str.19,@object       // @.str.19
.L.str.19:
	.asciz	"&(&ds_lld->sync_disp_lock)->rlock"
	.size	.L.str.19, 34

	.type	ufs_mq_dispatch_list_init.__key.20,@object // @ufs_mq_dispatch_list_init.__key.20
	.local	ufs_mq_dispatch_list_init.__key.20
	.comm	ufs_mq_dispatch_list_init.__key.20,1,1
	.type	.L.str.21,@object       // @.str.21
.L.str.21:
	.asciz	"&(&ds_lld->async_disp_lock)->rlock"
	.size	.L.str.21, 35

	.type	ufs_mq_dispatch_list_init.__key.22,@object // @ufs_mq_dispatch_list_init.__key.22
	.local	ufs_mq_dispatch_list_init.__key.22
	.comm	ufs_mq_dispatch_list_init.__key.22,1,1
	.type	.L.str.23,@object       // @.str.23
.L.str.23:
	.asciz	"&(&ds_lld->rec_bio_disp_list_lock)->rlock"
	.size	.L.str.23, 42

	.type	.L.str.24,@object       // @.str.24
.L.str.24:
	.asciz	"sync_dispatch"
	.size	.L.str.24, 14

	.type	.L.str.25,@object       // @.str.25
.L.str.25:
	.asciz	"\0013[BLK-IO]%s %d Failed to alloc sync_dispatch_workqueue\n"
	.size	.L.str.25, 57

	.type	.L__func__.ufs_mq_workqueue_init,@object // @__func__.ufs_mq_workqueue_init
.L__func__.ufs_mq_workqueue_init:
	.asciz	"ufs_mq_workqueue_init"
	.size	.L__func__.ufs_mq_workqueue_init, 22

	.type	.L.str.26,@object       // @.str.26
.L.str.26:
	.asciz	"async_dispatch"
	.size	.L.str.26, 15

	.type	.L.str.27,@object       // @.str.27
.L.str.27:
	.asciz	"io_guard"
	.size	.L.str.27, 9

	.type	.L.str.28,@object       // @.str.28
.L.str.28:
	.asciz	"\0013[BLK-IO]%s init Failed!\n"
	.size	.L.str.28, 27


	.ident	"Android (27847 based on r353983c) clang version 9.0.3 (https://android.googlesource.com/toolchain/clang dbdb7a2acf33dc76cd6f506c323253b8d892ae05) (https://android.googlesource.com/toolchain/llvm f32df87f6769ededcd9e2a6c3c4c45c1baf361c2) (based on LLVM 9.0.3svn)"
	.section	".note.GNU-stack","",@progbits

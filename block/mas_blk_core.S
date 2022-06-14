	.text
	.file	"mas_blk_core.c"
                                        // Start of file scope inline assembly
	.section "___kcrctab+blk_dev_write_order_preserved", "a"	
	.weak	__crc_blk_dev_write_order_preserved	
	.long	__crc_blk_dev_write_order_preserved	
	.previous					
	.section "___kcrctab+blk_req_get_order_nr", "a"	
	.weak	__crc_blk_req_get_order_nr	
	.long	__crc_blk_req_get_order_nr	
	.previous					
	.section "___kcrctab+blk_queue_order_enable", "a"	
	.weak	__crc_blk_queue_order_enable	
	.long	__crc_blk_queue_order_enable	
	.previous					
	.section "___kcrctab+blk_queue_query_order_enable", "a"	
	.weak	__crc_blk_queue_query_order_enable	
	.long	__crc_blk_queue_query_order_enable	
	.previous					
	.section "___kcrctab+blk_order_nr_reset", "a"	
	.weak	__crc_blk_order_nr_reset	
	.long	__crc_blk_order_nr_reset	
	.previous					

                                        // End of file scope inline assembly
	.globl	mas_blk_get_lld         // -- Begin function mas_blk_get_lld
	.p2align	2
	.type	mas_blk_get_lld,@function
mas_blk_get_lld:                        // @mas_blk_get_lld
// %bb.0:
	ldr	x8, [x0, #312]
	cbz	x8, .LBB0_3
// %bb.1:
	ldr	x8, [x0, #4088]
	add	x9, x0, #1824           // =1824
	add	x10, x8, #16            // =16
.LBB0_2:
	cmp	x8, #0                  // =0
	csel	x0, x9, x10, eq
	ret
.LBB0_3:
	ldr	x8, [x0, #720]
	add	x9, x0, #1824           // =1824
	add	x10, x8, #120           // =120
	b	.LBB0_2
.Lfunc_end0:
	.size	mas_blk_get_lld, .Lfunc_end0-mas_blk_get_lld
                                        // -- End function
	.globl	mas_blk_get_queue_by_lld // -- Begin function mas_blk_get_queue_by_lld
	.p2align	2
	.type	mas_blk_get_queue_by_lld,@function
mas_blk_get_queue_by_lld:               // @mas_blk_get_queue_by_lld
// %bb.0:
	ldr	w8, [x0, #4]
	cmp	w8, #2                  // =2
	b.eq	.LBB1_4
// %bb.1:
	cmp	w8, #1                  // =1
	b.eq	.LBB1_5
// %bb.2:
	cbnz	w8, .LBB1_6
// %bb.3:
	ldr	x0, [x0, #8]
	ret
.LBB1_4:
	ldr	x8, [x0, #8]
	ldr	x8, [x8, #1840]
	sub	x0, x8, #1, lsl #12     // =4096
	ret
.LBB1_5:
	ldr	x8, [x0, #8]
	ldr	x8, [x8, #112]
	sub	x0, x8, #1, lsl #12     // =4096
	ret
.LBB1_6:
	mov	x0, xzr
	ret
.Lfunc_end1:
	.size	mas_blk_get_queue_by_lld, .Lfunc_end1-mas_blk_get_queue_by_lld
                                        // -- End function
	.globl	blk_dio_ck              // -- Begin function blk_dio_ck
	.p2align	2
	.type	blk_dio_ck,@function
blk_dio_ck:                             // @blk_dio_ck
// %bb.0:
	str	x21, [sp, #-48]!        // 8-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	cbz	x0, .LBB2_13
// %bb.1:
	ldr	x8, [x0, #1120]
	cbz	x8, .LBB2_13
// %bb.2:
	mov	w19, w3
	mov	x20, x1
	cmp	w2, #1                  // =1
	b.eq	.LBB2_7
// %bb.3:
	cbnz	w2, .LBB2_13
// %bb.4:
	ldr	x9, [x8, #312]
	cbz	x9, .LBB2_14
// %bb.5:
	ldr	x9, [x8, #4088]
	add	x8, x8, #1824           // =1824
	add	x10, x9, #16            // =16
.LBB2_6:
	cmp	x9, #0                  // =0
	csel	x8, x8, x10, eq
	ldrsw	x21, [x8, #1688]
	cbnz	w21, .LBB2_10
	b	.LBB2_13
.LBB2_7:
	ldr	x9, [x8, #312]
	cbz	x9, .LBB2_15
// %bb.8:
	ldr	x9, [x8, #4088]
	add	x8, x8, #1824           // =1824
	add	x10, x9, #16            // =16
.LBB2_9:
	cmp	x9, #0                  // =0
	csel	x8, x8, x10, eq
	ldrsw	x21, [x8, #1692]
	cbz	w21, .LBB2_13
.LBB2_10:
	bl	ktime_get
	sub	x8, x0, x20
	cmn	x8, #999                // =999
	b.lt	.LBB2_13
// %bb.11:
	mov	x9, #63439
	movk	x9, #58195, lsl #16
	movk	x9, #39845, lsl #32
	movk	x9, #8388, lsl #48
	mov	w10, #3906
	smulh	x8, x8, x9
	smull	x9, w19, w10
	asr	x10, x8, #7
	add	x8, x10, x8, lsr #63
	sdiv	x9, x9, x21
	sub	x0, x9, x8
	cmp	x0, #1                  // =1
	b.lt	.LBB2_13
// %bb.12:
	bl	__udelay
.LBB2_13:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldr	x21, [sp], #48          // 8-byte Folded Reload
	ret
.LBB2_14:
	ldr	x9, [x8, #720]
	add	x8, x8, #1824           // =1824
	add	x10, x9, #120           // =120
	b	.LBB2_6
.LBB2_15:
	ldr	x9, [x8, #720]
	add	x8, x8, #1824           // =1824
	add	x10, x9, #120           // =120
	b	.LBB2_9
.Lfunc_end2:
	.size	blk_dio_ck, .Lfunc_end2-blk_dio_ck
                                        // -- End function
	.globl	blk_mq_tagset_vl_setup  // -- Begin function blk_mq_tagset_vl_setup
	.p2align	2
	.type	blk_mq_tagset_vl_setup,@function
blk_mq_tagset_vl_setup:                 // @blk_mq_tagset_vl_setup
// %bb.0:
	lsr	x8, x1, #28
	mov	w9, #500
	mov	w10, #300
	cmp	x8, #0                  // =0
	csel	w8, w10, w9, eq
	str	wzr, [x0, #1704]
	str	w8, [x0, #1708]
	ret
.Lfunc_end3:
	.size	blk_mq_tagset_vl_setup, .Lfunc_end3-blk_mq_tagset_vl_setup
                                        // -- End function
	.globl	mas_blk_queue_usr_ctrl_set // -- Begin function mas_blk_queue_usr_ctrl_set
	.p2align	2
	.type	mas_blk_queue_usr_ctrl_set,@function
mas_blk_queue_usr_ctrl_set:             // @mas_blk_queue_usr_ctrl_set
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	ldr	x8, [x0, #1568]
	mov	w9, #28570
	movk	w9, #4, lsl #16
	mov	x29, sp
	cmp	x8, x9
	b.ne	.LBB4_2
// %bb.1:
	mov	w8, #100
	str	x8, [x0, #1752]
	adrp	x8, jiffies
	ldr	x8, [x8, :lo12:jiffies]
	mov	w9, #48032
	movk	w9, #13, lsl #16
	add	x0, x0, #1760           // =1760
	add	x1, x8, x9
	bl	mod_timer
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.LBB4_2:
	str	xzr, [x0, #1752]
	add	x0, x0, #1760           // =1760
	bl	del_timer_sync
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end4:
	.size	mas_blk_queue_usr_ctrl_set, .Lfunc_end4-mas_blk_queue_usr_ctrl_set
                                        // -- End function
	.globl	mas_blk_queue_usr_ctrl_recovery_timer_expire // -- Begin function mas_blk_queue_usr_ctrl_recovery_timer_expire
	.p2align	2
	.type	mas_blk_queue_usr_ctrl_recovery_timer_expire,@function
mas_blk_queue_usr_ctrl_recovery_timer_expire: // @mas_blk_queue_usr_ctrl_recovery_timer_expire
// %bb.0:
	str	xzr, [x0, #1568]
	str	xzr, [x0, #1752]
	ret
.Lfunc_end5:
	.size	mas_blk_queue_usr_ctrl_recovery_timer_expire, .Lfunc_end5-mas_blk_queue_usr_ctrl_recovery_timer_expire
                                        // -- End function
	.globl	mas_blk_bio_clone_fast  // -- Begin function mas_blk_bio_clone_fast
	.p2align	2
	.type	mas_blk_bio_clone_fast,@function
mas_blk_bio_clone_fast:                 // @mas_blk_bio_clone_fast
// %bb.0:
	ldr	x8, [x1, #88]
	str	x8, [x0, #88]
	ldrb	w8, [x1, #264]
	strb	w8, [x0, #264]
	ldrb	w8, [x1, #265]
	strb	w8, [x0, #265]
	ldr	x8, [x1, #272]
	str	x8, [x0, #272]
	ldr	x8, [x1, #280]
	str	x8, [x0, #280]
	ldrb	w8, [x1, #288]
	strb	w8, [x0, #288]
	ldrb	w8, [x1, #289]
	strb	w8, [x0, #289]
	ret
.Lfunc_end6:
	.size	mas_blk_bio_clone_fast, .Lfunc_end6-mas_blk_bio_clone_fast
                                        // -- End function
	.globl	mas_blk_set_data_flag   // -- Begin function mas_blk_set_data_flag
	.p2align	2
	.type	mas_blk_set_data_flag,@function
mas_blk_set_data_flag:                  // @mas_blk_set_data_flag
// %bb.0:
	str	x21, [sp, #-48]!        // 8-byte Folded Spill
	mov	w8, w3
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	mov	x20, x2
	mov	x19, x1
	mov	x21, x0
	str	x8, [x1, #8]
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB7_3
// %bb.1:
	ldr	x8, [x19, #16]
	mov	x0, x21
	mov	x1, x20
	and	x8, x8, #0xfffffffffffffffd
	stp	x8, x20, [x19, #16]
	bl	mas_blk_match_expected_lba
	tbz	w0, #0, .LBB7_3
// %bb.2:
	ldr	x8, [x19, #16]
	orr	x8, x8, #0x2
	str	x8, [x19, #16]
.LBB7_3:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldr	x21, [sp], #48          // 8-byte Folded Reload
	ret
.Lfunc_end7:
	.size	mas_blk_set_data_flag, .Lfunc_end7-mas_blk_set_data_flag
                                        // -- End function
	.globl	mas_blk_bio_queue_split // -- Begin function mas_blk_bio_queue_split
	.p2align	2
	.type	mas_blk_bio_queue_split,@function
mas_blk_bio_queue_split:                // @mas_blk_bio_queue_split
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	ldr	x8, [x1]
	mov	x20, x0
	add	x29, sp, #16            // =16
	mov	x19, x2
	mov	x0, x8
	bl	mas_blk_busyidle_check_bio_endio
	mov	x0, x20
	mov	x1, x19
	bl	mas_blk_busyidle_check_bio
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end8:
	.size	mas_blk_bio_queue_split, .Lfunc_end8-mas_blk_bio_queue_split
                                        // -- End function
	.globl	mas_blk_bio_segment_bytes_split // -- Begin function mas_blk_bio_segment_bytes_split
	.p2align	2
	.type	mas_blk_bio_segment_bytes_split,@function
mas_blk_bio_segment_bytes_split:        // @mas_blk_bio_segment_bytes_split
// %bb.0:
	str	x23, [sp, #-64]!        // 8-byte Folded Spill
	stp	x22, x21, [sp, #16]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #32]     // 16-byte Folded Spill
	mov	w19, w4
	mov	w20, w3
	mov	w23, w2
	mov	x21, x0
	stp	x29, x30, [sp, #48]     // 16-byte Folded Spill
	add	x29, sp, #48            // =48
	cbz	w2, .LBB9_6
// %bb.1:
	ldr	w8, [x21, #48]
	cmp	w8, w23
	b.ls	.LBB9_6
// %bb.2:
	mov	x2, x1
	mov	w1, #20971520
	mov	x0, x21
	bl	bio_clone_fast
	mov	x22, x0
	cbz	x0, .LBB9_7
// %bb.3:
	mov	x0, x21
	mov	x1, x22
	str	w23, [x22, #48]
	bl	mas_blk_inline_crypto_bio_split_pre
	ldr	w1, [x22, #48]
	mov	x0, x21
	bl	bio_advance
	ldrb	w8, [x21, #21]
	str	wzr, [x21, #56]
	tbz	w8, #2, .LBB9_5
// %bb.4:
	ldrh	w8, [x22, #20]
	orr	w8, w8, #0x400
	strh	w8, [x22, #20]
.LBB9_5:
	mov	x0, x21
	bl	mas_blk_inline_crypto_bio_split_post
	b	.LBB9_7
.LBB9_6:
	adrp	x0, .L.str.6
	adrp	x1, .L__func__.mas_blk_bio_split_in_bytes
	add	x0, x0, :lo12:.L.str.6
	add	x1, x1, :lo12:.L__func__.mas_blk_bio_split_in_bytes
	bl	printk
	mov	x22, xzr
.LBB9_7:
	cmp	x22, #0                 // =0
	csel	x8, x21, x22, eq
	ldr	w9, [x8, #36]
	str	w20, [x8, #32]
	cmp	w9, w19
	b.hs	.LBB9_9
// %bb.8:
	str	w19, [x8, #36]
.LBB9_9:
	mov	x0, x22
	ldp	x29, x30, [sp, #48]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]     // 16-byte Folded Reload
	ldr	x23, [sp], #64          // 8-byte Folded Reload
	ret
.Lfunc_end9:
	.size	mas_blk_bio_segment_bytes_split, .Lfunc_end9-mas_blk_bio_segment_bytes_split
                                        // -- End function
	.globl	mas_blk_bio_get_residual_byte // -- Begin function mas_blk_bio_get_residual_byte
	.p2align	2
	.type	mas_blk_bio_get_residual_byte,@function
mas_blk_bio_get_residual_byte:          // @mas_blk_bio_get_residual_byte
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	ldr	x8, [x0, #312]
	mov	x19, x1
	add	x29, sp, #16            // =16
	cbz	x8, .LBB10_8
// %bb.1:
	ldr	x8, [x0, #4088]
	add	x9, x0, #1824           // =1824
	add	x10, x8, #16            // =16
.LBB10_2:
	cmp	x8, #0                  // =0
	csel	x8, x9, x10, eq
	ldr	w20, [x8, #216]
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB10_6
// %bb.3:
	cbz	w20, .LBB10_5
// %bb.4:
	ldr	x8, [x19]
	lsr	x8, x8, #3
	udiv	x9, x8, x20
	neg	w8, w8
	madd	w8, w9, w20, w8
	add	w8, w20, w8
	lsl	w0, w8, #12
	b	.LBB10_7
.LBB10_5:
	adrp	x0, .L.str.1
	adrp	x1, .L__func__.mas_blk_bio_get_residual_byte
	add	x0, x0, :lo12:.L.str.1
	add	x1, x1, :lo12:.L__func__.mas_blk_bio_get_residual_byte
	mov	w2, wzr
	bl	printk
.LBB10_6:
	ldr	w0, [x19, #8]
.LBB10_7:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.LBB10_8:
	ldr	x8, [x0, #720]
	add	x9, x0, #1824           // =1824
	add	x10, x8, #120           // =120
	b	.LBB10_2
.Lfunc_end10:
	.size	mas_blk_bio_get_residual_byte, .Lfunc_end10-mas_blk_bio_get_residual_byte
                                        // -- End function
	.globl	mas_blk_bio_check_over_section // -- Begin function mas_blk_bio_check_over_section
	.p2align	2
	.type	mas_blk_bio_check_over_section,@function
mas_blk_bio_check_over_section:         // @mas_blk_bio_check_over_section
// %bb.0:
	stp	x22, x21, [sp, #-48]!   // 16-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	ldrb	w8, [x1, #16]
	add	x29, sp, #32            // =32
	cmp	w8, #1                  // =1
	b.ne	.LBB11_9
// %bb.1:
	mov	x20, x1
	mov	x19, x0
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB11_9
// %bb.2:
	ldr	x21, [x20, #40]
	ldr	x8, [x19, #312]
	ldr	w20, [x20, #48]
	cbz	x8, .LBB11_11
// %bb.3:
	ldr	x8, [x19, #4088]
	add	x9, x19, #1824          // =1824
	add	x10, x8, #16            // =16
.LBB11_4:
	cmp	x8, #0                  // =0
	csel	x8, x9, x10, eq
	ldr	w22, [x8, #216]
	mov	x0, x19
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB11_9
// %bb.5:
	cbz	w22, .LBB11_8
// %bb.6:
	lsr	x8, x21, #3
	udiv	x9, x8, x22
	neg	w8, w8
	madd	w8, w9, w22, w8
	add	w8, w22, w8
	cmp	w20, w8, lsl #12
	b.ls	.LBB11_9
// %bb.7:
	orr	w0, wzr, #0x1
	b	.LBB11_10
.LBB11_8:
	adrp	x0, .L.str.1
	adrp	x1, .L__func__.mas_blk_bio_get_residual_byte
	add	x0, x0, :lo12:.L.str.1
	add	x1, x1, :lo12:.L__func__.mas_blk_bio_get_residual_byte
	mov	w2, wzr
	bl	printk
.LBB11_9:
	mov	w0, wzr
.LBB11_10:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldp	x22, x21, [sp], #48     // 16-byte Folded Reload
	ret
.LBB11_11:
	ldr	x8, [x19, #720]
	add	x9, x19, #1824          // =1824
	add	x10, x8, #120           // =120
	b	.LBB11_4
.Lfunc_end11:
	.size	mas_blk_bio_check_over_section, .Lfunc_end11-mas_blk_bio_check_over_section
                                        // -- End function
	.globl	mas_blk_bio_merge_allow // -- Begin function mas_blk_bio_merge_allow
	.p2align	2
	.type	mas_blk_bio_merge_allow,@function
mas_blk_bio_merge_allow:                // @mas_blk_bio_merge_allow
// %bb.0:
	ldr	w8, [x0, #272]
	ldr	w9, [x1, #384]
	eor	w8, w9, w8, lsr #3
	tst	w8, #0x1
	cset	w0, eq
	ret
.Lfunc_end12:
	.size	mas_blk_bio_merge_allow, .Lfunc_end12-mas_blk_bio_merge_allow
                                        // -- End function
	.globl	mas_blk_bio_merge_done  // -- Begin function mas_blk_bio_merge_done
	.p2align	2
	.type	mas_blk_bio_merge_done,@function
mas_blk_bio_merge_done:                 // @mas_blk_bio_merge_done
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	mov	x0, x1
	mov	x1, x2
	mov	x29, sp
	bl	mas_blk_latency_for_merge
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end13:
	.size	mas_blk_bio_merge_done, .Lfunc_end13-mas_blk_bio_merge_done
                                        // -- End function
	.globl	mas_blk_account_io_completion // -- Begin function mas_blk_account_io_completion
	.p2align	2
	.type	mas_blk_account_io_completion,@function
mas_blk_account_io_completion:          // @mas_blk_account_io_completion
// %bb.0:
	cbz	x0, .LBB14_7
// %bb.1:
	ldr	x8, [x0, #64]
	ldr	x9, [x8, #312]
	cbz	x9, .LBB14_9
// %bb.2:
	ldr	x9, [x8, #4088]
	add	x8, x8, #1824           // =1824
	add	x10, x9, #16            // =16
.LBB14_3:
	cmp	x9, #0                  // =0
	ldrb	w9, [x0, #84]
	csel	x8, x8, x10, eq
	cmp	w9, #3                  // =3
	b.ne	.LBB14_5
// %bb.4:
	ldr	x9, [x8, #1584]
	add	x9, x9, w1, uxtw
	str	x9, [x8, #1584]
	ldrb	w9, [x0, #84]
.LBB14_5:
	cmp	w9, #1                  // =1
	b.ne	.LBB14_8
// %bb.6:
	ldr	x9, [x8, #1576]
	mov	w0, wzr
	add	x9, x9, w1, uxtw
	str	x9, [x8, #1576]
	ret
.LBB14_7:
	mov	w0, #-1
	ret
.LBB14_8:
	mov	w0, wzr
	ret
.LBB14_9:
	ldr	x9, [x8, #720]
	add	x8, x8, #1824           // =1824
	add	x10, x9, #120           // =120
	b	.LBB14_3
.Lfunc_end14:
	.size	mas_blk_account_io_completion, .Lfunc_end14-mas_blk_account_io_completion
                                        // -- End function
	.globl	mas_blk_generic_make_request_check // -- Begin function mas_blk_generic_make_request_check
	.p2align	2
	.type	mas_blk_generic_make_request_check,@function
mas_blk_generic_make_request_check:     // @mas_blk_generic_make_request_check
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	ldr	x8, [x0, #8]
	add	x29, sp, #16            // =16
	mov	x19, x0
	ldr	x20, [x8, #1120]
	bl	mas_blk_bio_set_opf_unistore
	mov	x0, x19
	mov	w1, wzr
	bl	mas_blk_latency_bio_check
	mov	x0, x20
	mov	x1, x19
	bl	mas_blk_flush_async_dispatch
	tbnz	w0, #0, .LBB15_3
// %bb.1:
	mov	x0, x20
	mov	x1, x19
	bl	mas_blk_busyidle_check_bio
	mov	w0, wzr
.LBB15_2:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.LBB15_3:
	mov	x0, x19
	strb	wzr, [x19, #26]
	bl	bio_endio
	orr	w0, wzr, #0x1
	b	.LBB15_2
.Lfunc_end15:
	.size	mas_blk_generic_make_request_check, .Lfunc_end15-mas_blk_generic_make_request_check
                                        // -- End function
	.globl	mas_blk_generic_make_request // -- Begin function mas_blk_generic_make_request
	.p2align	2
	.type	mas_blk_generic_make_request,@function
mas_blk_generic_make_request:           // @mas_blk_generic_make_request
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	orr	w1, wzr, #0x2
	mov	x29, sp
	bl	mas_blk_latency_bio_check
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end16:
	.size	mas_blk_generic_make_request, .Lfunc_end16-mas_blk_generic_make_request
                                        // -- End function
	.globl	mas_blk_start_plug      // -- Begin function mas_blk_start_plug
	.p2align	2
	.type	mas_blk_start_plug,@function
mas_blk_start_plug:                     // @mas_blk_start_plug
// %bb.0:
	add	x8, x0, #48             // =48
	str	x8, [x0, #48]
	stp	x8, xzr, [x0, #56]
	ret
.Lfunc_end17:
	.size	mas_blk_start_plug, .Lfunc_end17-mas_blk_start_plug
                                        // -- End function
	.globl	mas_blk_flush_plug_list // -- Begin function mas_blk_flush_plug_list
	.p2align	2
	.type	mas_blk_flush_plug_list,@function
mas_blk_flush_plug_list:                // @mas_blk_flush_plug_list
// %bb.0:
	mov	x8, x0
	ldr	x9, [x8, #48]!
	cmp	x8, x9
	b.eq	.LBB18_3
// %bb.1:
	ldr	x8, [x0, #64]
	cbz	x8, .LBB18_3
// %bb.2:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	and	w1, w1, #0x1
	mov	x29, sp
	blr	x8
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
.LBB18_3:
	ret
.Lfunc_end18:
	.size	mas_blk_flush_plug_list, .Lfunc_end18-mas_blk_flush_plug_list
                                        // -- End function
	.globl	mas_blk_bio_endio       // -- Begin function mas_blk_bio_endio
	.p2align	2
	.type	mas_blk_bio_endio,@function
mas_blk_bio_endio:                      // @mas_blk_bio_endio
// %bb.0:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	mov	x19, x0
	bl	mas_blk_busyidle_check_bio_endio
	orr	w1, wzr, #0x1
	mov	x0, x19
	bl	mas_blk_latency_bio_check
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldr	x19, [sp], #32          // 8-byte Folded Reload
	ret
.Lfunc_end19:
	.size	mas_blk_bio_endio, .Lfunc_end19-mas_blk_bio_endio
                                        // -- End function
	.globl	mas_blk_bio_free        // -- Begin function mas_blk_bio_free
	.p2align	2
	.type	mas_blk_bio_free,@function
mas_blk_bio_free:                       // @mas_blk_bio_free
// %bb.0:
	ldrb	w8, [x0, #144]
	tbnz	w8, #0, .LBB20_2
// %bb.1:
	ret
.LBB20_2:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	mov	x29, sp
	bl	mas_blk_busyidle_check_bio_endio
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end20:
	.size	mas_blk_bio_free, .Lfunc_end20-mas_blk_bio_free
                                        // -- End function
	.globl	mas_blk_request_init_from_bio // -- Begin function mas_blk_request_init_from_bio
	.p2align	2
	.type	mas_blk_request_init_from_bio,@function
mas_blk_request_init_from_bio:          // @mas_blk_request_init_from_bio
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	mov	x20, x1
	mov	w1, wzr
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	mov	x19, x0
	bl	mas_blk_latency_req_check
	mov	x0, x19
	mov	x1, x20
	str	wzr, [x19, #284]
	bl	mas_blk_request_init_from_bio_unistore
	ldrb	w8, [x20, #384]
	str	x19, [x20, #104]
	tbz	w8, #0, .LBB21_2
// %bb.1:
	ldr	x8, [x19, #272]
	orr	x8, x8, #0x8
	str	x8, [x19, #272]
.LBB21_2:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end21:
	.size	mas_blk_request_init_from_bio, .Lfunc_end21-mas_blk_request_init_from_bio
                                        // -- End function
	.globl	mas_blk_insert_cloned_request // -- Begin function mas_blk_insert_cloned_request
	.p2align	2
	.type	mas_blk_insert_cloned_request,@function
mas_blk_insert_cloned_request:          // @mas_blk_insert_cloned_request
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	mov	x29, sp
	bl	mas_blk_busyidle_check_request_bio
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end22:
	.size	mas_blk_insert_cloned_request, .Lfunc_end22-mas_blk_insert_cloned_request
                                        // -- End function
	.globl	mas_blk_request_execute_nowait // -- Begin function mas_blk_request_execute_nowait
	.p2align	2
	.type	mas_blk_request_execute_nowait,@function
mas_blk_request_execute_nowait:         // @mas_blk_request_execute_nowait
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	mov	x29, sp
	bl	mas_blk_busyidle_check_execute_request
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end23:
	.size	mas_blk_request_execute_nowait, .Lfunc_end23-mas_blk_request_execute_nowait
                                        // -- End function
	.globl	mas_blk_mq_rq_ctx_init  // -- Begin function mas_blk_mq_rq_ctx_init
	.p2align	2
	.type	mas_blk_mq_rq_ctx_init,@function
mas_blk_mq_rq_ctx_init:                 // @mas_blk_mq_rq_ctx_init
// %bb.0:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	ldr	x8, [x0, #1816]
	mov	x19, x2
	add	x29, sp, #16            // =16
	cbz	x8, .LBB24_3
// %bb.1:
	ldr	x8, [x8, #32]
	cbz	x8, .LBB24_3
// %bb.2:
	mov	x2, x19
	blr	x8
.LBB24_3:
	mov	x0, x19
	bl	mas_blk_inline_crypto_req_init
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldr	x19, [sp], #32          // 8-byte Folded Reload
	ret
.Lfunc_end24:
	.size	mas_blk_mq_rq_ctx_init, .Lfunc_end24-mas_blk_mq_rq_ctx_init
                                        // -- End function
	.globl	mas_blk_mq_request_start // -- Begin function mas_blk_mq_request_start
	.p2align	2
	.type	mas_blk_mq_request_start,@function
mas_blk_mq_request_start:               // @mas_blk_mq_request_start
// %bb.0:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	mov	w1, #5
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	mov	x19, x0
	bl	mas_blk_latency_req_check
	ldr	x8, [x19, #64]
	mov	w10, #28570
	movk	w10, #4, lsl #16
	ldr	x9, [x8, #1568]
	cmp	x9, x10
	b.eq	.LBB25_2
.LBB25_1:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldr	x19, [sp], #32          // 8-byte Folded Reload
	ret
.LBB25_2:
	ldrb	w9, [x19, #84]
	orr	w10, w9, #0x3
	cmp	w10, #35                // =35
	b.eq	.LBB25_1
// %bb.3:
	ldr	w10, [x19, #104]
	cbz	w10, .LBB25_1
// %bb.4:
	cmp	w9, #1                  // =1
	b.ne	.LBB25_11
// %bb.5:
	cmp	w10, #32, lsl #12       // =131072
	b.hi	.LBB25_8
// %bb.6:
	ldr	x0, [x8, #1752]
	cbz	x0, .LBB25_8
// %bb.7:
	bl	__udelay
.LBB25_8:
	ldr	x8, [x19, #64]
	ldr	x0, [x8, #1736]
	cbz	x0, .LBB25_11
// %bb.9:
	ldr	w8, [x19, #104]
	cmp	w8, #32, lsl #12        // =131072
	b.ls	.LBB25_11
// %bb.10:
	bl	__udelay
.LBB25_11:
	ldrb	w8, [x19, #84]
	cbnz	w8, .LBB25_1
// %bb.12:
	ldr	x8, [x19, #64]
	ldr	x0, [x8, #1744]
	cbz	x0, .LBB25_15
// %bb.13:
	ldr	w8, [x19, #104]
	cmp	w8, #32, lsl #12        // =131072
	b.hi	.LBB25_15
// %bb.14:
	bl	__udelay
.LBB25_15:
	ldr	x8, [x19, #64]
	ldr	x0, [x8, #1728]
	cbz	x0, .LBB25_1
// %bb.16:
	ldr	w8, [x19, #104]
	cmp	w8, #32, lsl #12        // =131072
	b.ls	.LBB25_1
// %bb.17:
	bl	__udelay
	b	.LBB25_1
.Lfunc_end25:
	.size	mas_blk_mq_request_start, .Lfunc_end25-mas_blk_mq_request_start
                                        // -- End function
	.globl	mas_blk_request_start   // -- Begin function mas_blk_request_start
	.p2align	2
	.type	mas_blk_request_start,@function
mas_blk_request_start:                  // @mas_blk_request_start
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	mov	w1, #5
	mov	x29, sp
	bl	mas_blk_latency_req_check
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end26:
	.size	mas_blk_request_start, .Lfunc_end26-mas_blk_request_start
                                        // -- End function
	.globl	mas_blk_requeue_request // -- Begin function mas_blk_requeue_request
	.p2align	2
	.type	mas_blk_requeue_request,@function
mas_blk_requeue_request:                // @mas_blk_requeue_request
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	mov	w1, #11
	mov	x29, sp
	bl	mas_blk_latency_req_check
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end27:
	.size	mas_blk_requeue_request, .Lfunc_end27-mas_blk_requeue_request
                                        // -- End function
	.globl	mas_blk_request_update  // -- Begin function mas_blk_request_update
	.p2align	2
	.type	mas_blk_request_update,@function
mas_blk_request_update:                 // @mas_blk_request_update
// %bb.0:
	str	x21, [sp, #-48]!        // 8-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	ldr	x8, [x0, #64]
	mov	w20, w2
	mov	x19, x0
	mov	w21, w1
	ldr	x9, [x8, #312]
	add	x29, sp, #32            // =32
	cbz	x9, .LBB28_14
// %bb.1:
	ldr	x9, [x8, #4088]
	add	x8, x8, #1824           // =1824
	add	x10, x9, #16            // =16
.LBB28_2:
	cmp	x9, #0                  // =0
	ldrb	w9, [x19, #84]
	csel	x8, x8, x10, eq
	cmp	w9, #3                  // =3
	b.eq	.LBB28_12
// %bb.3:
	cmp	w9, #1                  // =1
	b.eq	.LBB28_13
.LBB28_4:
	cmp	w9, #2                  // =2
	b.ne	.LBB28_6
.LBB28_5:
	and	w1, w21, #0xff
	mov	x0, x19
	bl	mas_blk_flush_update
.LBB28_6:
	mov	w1, #13
	mov	x0, x19
	bl	mas_blk_latency_req_check
	tst	w21, #0xff
	b.ne	.LBB28_9
// %bb.7:
	cbz	w20, .LBB28_9
// %bb.8:
	mov	x0, x19
	mov	w1, w20
	bl	mas_blk_update_expected_lba
.LBB28_9:
	tst	w21, #0xff
	b.ne	.LBB28_11
// %bb.10:
	mov	x0, x19
	bl	mas_blk_add_bio_to_buf_list
.LBB28_11:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldr	x21, [sp], #48          // 8-byte Folded Reload
	ret
.LBB28_12:
	ldr	x9, [x8, #1584]
	add	x9, x9, w20, uxtw
	str	x9, [x8, #1584]
	ldrb	w9, [x19, #84]
	cmp	w9, #1                  // =1
	b.ne	.LBB28_4
.LBB28_13:
	ldr	x9, [x8, #1576]
	add	x9, x9, w20, uxtw
	str	x9, [x8, #1576]
	ldrb	w9, [x19, #84]
	cmp	w9, #2                  // =2
	b.eq	.LBB28_5
	b	.LBB28_6
.LBB28_14:
	ldr	x9, [x8, #720]
	add	x8, x8, #1824           // =1824
	add	x10, x9, #120           // =120
	b	.LBB28_2
.Lfunc_end28:
	.size	mas_blk_request_update, .Lfunc_end28-mas_blk_request_update
                                        // -- End function
	.globl	mas_blk_mq_request_free // -- Begin function mas_blk_mq_request_free
	.p2align	2
	.type	mas_blk_mq_request_free,@function
mas_blk_mq_request_free:                // @mas_blk_mq_request_free
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	orr	w1, wzr, #0xe
	mov	x29, sp
	str	xzr, [x0, #464]
	bl	mas_blk_latency_req_check
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end29:
	.size	mas_blk_mq_request_free, .Lfunc_end29-mas_blk_mq_request_free
                                        // -- End function
	.globl	mas_blk_request_put     // -- Begin function mas_blk_request_put
	.p2align	2
	.type	mas_blk_request_put,@function
mas_blk_request_put:                    // @mas_blk_request_put
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	orr	w1, wzr, #0xe
	mov	x29, sp
	str	xzr, [x0, #464]
	bl	mas_blk_latency_req_check
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end30:
	.size	mas_blk_request_put, .Lfunc_end30-mas_blk_request_put
                                        // -- End function
	.globl	mas_blk_check_partition_done // -- Begin function mas_blk_check_partition_done
	.p2align	2
	.type	mas_blk_check_partition_done,@function
mas_blk_check_partition_done:           // @mas_blk_check_partition_done
// %bb.0:
	ldr	x8, [x0, #1120]
	and	w9, w1, #0x1
	strb	w9, [x8, #1560]
	ret
.Lfunc_end31:
	.size	mas_blk_check_partition_done, .Lfunc_end31-mas_blk_check_partition_done
                                        // -- End function
	.globl	mas_blk_allocated_queue_init // -- Begin function mas_blk_allocated_queue_init
	.p2align	2
	.type	mas_blk_allocated_queue_init,@function
mas_blk_allocated_queue_init:           // @mas_blk_allocated_queue_init
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	add	x8, x0, #1712           // =1712
	mov	w9, #122
	adrp	x1, .L.str.7
	adrp	x2, mas_blk_dev_lld_init.__key
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	mov	x19, x0
	add	x20, x0, #1824          // =1824
	str	x8, [x0, #1712]
	str	x8, [x0, #1720]
	strb	wzr, [x0, #1560]
	str	w9, [x0, #1824]
	str	wzr, [x0, #3420]
	add	x0, x0, #3424           // =3424
	add	x1, x1, :lo12:.L.str.7
	add	x2, x2, :lo12:mas_blk_dev_lld_init.__key
	add	x29, sp, #16            // =16
	bl	__raw_spin_lock_init
	adrp	x1, .L.str.9
	adrp	x2, mas_blk_dev_lld_init.__key.8
	orr	w8, wzr, #0x1
	add	x0, x19, #3456          // =3456
	add	x1, x1, :lo12:.L.str.9
	add	x2, x2, :lo12:mas_blk_dev_lld_init.__key.8
	str	w8, [x19, #3448]
	bl	__raw_spin_lock_init
	mov	x0, x20
	bl	mas_blk_dev_lld_init_unistore
	add	x20, x19, #2864         // =2864
	add	x8, x19, #3480          // =3480
	mov	x0, x20
	str	x8, [x19, #3480]
	str	x8, [x19, #3488]
	str	wzr, [x19, #1828]
	str	x19, [x19, #1832]
	bl	mas_blk_busyidle_state_init
	ldr	x8, [x19, #1848]
	cbz	w0, .LBB32_2
// %bb.1:
	and	x8, x8, #0xfffffffffffffff7
	str	x8, [x19, #1848]
.LBB32_2:
	tbz	w8, #5, .LBB32_4
// %bb.3:
	orr	w9, wzr, #0x1
	strb	w9, [x20]
.LBB32_4:
	orr	x8, x8, #0x40
	str	x8, [x19, #1848]
	str	xzr, [x19, #3512]
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end32:
	.size	mas_blk_allocated_queue_init, .Lfunc_end32-mas_blk_allocated_queue_init
                                        // -- End function
	.globl	mas_blk_sq_init_allocated_queue // -- Begin function mas_blk_sq_init_allocated_queue
	.p2align	2
	.type	mas_blk_sq_init_allocated_queue,@function
mas_blk_sq_init_allocated_queue:        // @mas_blk_sq_init_allocated_queue
// %bb.0:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	mov	x19, x0
	bl	mas_blk_dump_register_queue
	mov	x0, x19
	bl	mas_blk_queue_latency_init
	mov	x0, x19
	bl	mas_blk_queue_async_flush_init
	add	x0, x19, #1760          // =1760
	mov	w1, wzr
	mov	x2, xzr
	mov	x3, xzr
	str	xzr, [x19, #1752]
	str	xzr, [x19, #1744]
	str	xzr, [x19, #1736]
	str	xzr, [x19, #1728]
	bl	init_timer_key
	adrp	x8, _cfi_mas_blk_queue_usr_ctrl_recovery_timer_expire
	add	x8, x8, :lo12:_cfi_mas_blk_queue_usr_ctrl_recovery_timer_expire
	str	x8, [x19, #1784]
	str	x19, [x19, #1792]
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldr	x19, [sp], #32          // 8-byte Folded Reload
	ret
.Lfunc_end33:
	.size	mas_blk_sq_init_allocated_queue, .Lfunc_end33-mas_blk_sq_init_allocated_queue
                                        // -- End function
	.globl	mas_blk_mq_init_allocated_queue // -- Begin function mas_blk_mq_init_allocated_queue
	.p2align	2
	.type	mas_blk_mq_init_allocated_queue,@function
mas_blk_mq_init_allocated_queue:        // @mas_blk_mq_init_allocated_queue
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	ldr	x20, [x0, #4088]
	mov	x19, x0
	add	x29, sp, #16            // =16
	ldr	w8, [x20, #1724]
	cbz	w8, .LBB34_5
// %bb.1:
	mov	x9, xzr
.LBB34_2:                               // =>This Inner Loop Header: Depth=1
	ldr	x10, [x20, #1760]
	ldr	x10, [x10, x9, lsl #3]
	cbz	x10, .LBB34_4
// %bb.3:                               //   in Loop: Header=BB34_2 Depth=1
	str	x20, [x10, #24]
	ldr	w8, [x20, #1724]
.LBB34_4:                               //   in Loop: Header=BB34_2 Depth=1
	add	x9, x9, #1              // =1
	cmp	x9, w8, uxtw
	b.lo	.LBB34_2
.LBB34_5:
	mov	x0, x19
	bl	mas_blk_dump_register_queue
	mov	x0, x19
	bl	mas_blk_queue_latency_init
	mov	x0, x19
	bl	mas_blk_queue_async_flush_init
	add	x0, x19, #1760          // =1760
	mov	w1, wzr
	mov	x2, xzr
	mov	x3, xzr
	str	xzr, [x19, #1752]
	str	xzr, [x19, #1744]
	str	xzr, [x19, #1736]
	str	xzr, [x19, #1728]
	bl	init_timer_key
	adrp	x8, _cfi_mas_blk_queue_usr_ctrl_recovery_timer_expire
	add	x8, x8, :lo12:_cfi_mas_blk_queue_usr_ctrl_recovery_timer_expire
	str	x8, [x19, #1784]
	str	x19, [x19, #1792]
	ldr	x8, [x20, #1712]
	cbz	x8, .LBB34_10
// %bb.6:
	ldr	x8, [x8, #40]
	str	x8, [x19, #1816]
	cbz	x8, .LBB34_11
// %bb.7:
	ldr	x8, [x8, #8]
	cbz	x8, .LBB34_11
// %bb.8:
	mov	x0, x19
	blr	x8
	cbz	w0, .LBB34_11
// %bb.9:
	mov	w2, w0
	adrp	x0, .L.str.2
	adrp	x1, .L__func__.mas_blk_mq_init_allocated_queue
	add	x0, x0, :lo12:.L.str.2
	add	x1, x1, :lo12:.L__func__.mas_blk_mq_init_allocated_queue
	bl	printk
	b	.LBB34_11
.LBB34_10:
	str	xzr, [x19, #1816]
.LBB34_11:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end34:
	.size	mas_blk_mq_init_allocated_queue, .Lfunc_end34-mas_blk_mq_init_allocated_queue
                                        // -- End function
	.globl	mas_blk_mq_free_queue   // -- Begin function mas_blk_mq_free_queue
	.p2align	2
	.type	mas_blk_mq_free_queue,@function
mas_blk_mq_free_queue:                  // @mas_blk_mq_free_queue
// %bb.0:
	ldr	x8, [x0, #1816]
	cbz	x8, .LBB35_3
// %bb.1:
	ldr	x8, [x8, #16]
	cbz	x8, .LBB35_3
// %bb.2:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	mov	x29, sp
	blr	x8
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
.LBB35_3:
	ret
.Lfunc_end35:
	.size	mas_blk_mq_free_queue, .Lfunc_end35-mas_blk_mq_free_queue
                                        // -- End function
	.globl	mas_blk_cleanup_queue   // -- Begin function mas_blk_cleanup_queue
	.p2align	2
	.type	mas_blk_cleanup_queue,@function
mas_blk_cleanup_queue:                  // @mas_blk_cleanup_queue
// %bb.0:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	mov	x19, x0
	bl	mas_blk_dump_unregister_queue
	add	x0, x19, #1760          // =1760
	bl	del_timer_sync
	mov	x0, x19
	bl	mas_blk_flush_reduced_queue_unregister
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldr	x19, [sp], #32          // 8-byte Folded Reload
	ret
.Lfunc_end36:
	.size	mas_blk_cleanup_queue, .Lfunc_end36-mas_blk_cleanup_queue
                                        // -- End function
	.globl	mas_blk_allocated_tags_init // -- Begin function mas_blk_allocated_tags_init
	.p2align	2
	.type	mas_blk_allocated_tags_init,@function
mas_blk_allocated_tags_init:            // @mas_blk_allocated_tags_init
// %bb.0:
	str	x21, [sp, #-48]!        // 8-byte Folded Spill
	adrp	x1, .L.str.3
	adrp	x2, mas_blk_allocated_tags_init.__key
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	mov	x19, x0
	add	x0, x0, #40             // =40
	add	x1, x1, :lo12:.L.str.3
	add	x2, x2, :lo12:mas_blk_allocated_tags_init.__key
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	bl	__mutex_init
	add	x20, x19, #120          // =120
	add	x8, x19, #104           // =104
	mov	w2, #1696
	mov	x0, x20
	mov	w1, wzr
	str	x8, [x19, #104]
	str	x8, [x19, #112]
	bl	memset
	adrp	x1, .L.str.7
	adrp	x2, mas_blk_dev_lld_init.__key
	mov	w8, #122
	add	x0, x19, #1720          // =1720
	add	x1, x1, :lo12:.L.str.7
	add	x2, x2, :lo12:mas_blk_dev_lld_init.__key
	str	wzr, [x19, #1716]
	str	w8, [x19, #120]
	bl	__raw_spin_lock_init
	adrp	x1, .L.str.9
	adrp	x2, mas_blk_dev_lld_init.__key.8
	orr	w21, wzr, #0x1
	add	x0, x19, #1752          // =1752
	add	x1, x1, :lo12:.L.str.9
	add	x2, x2, :lo12:mas_blk_dev_lld_init.__key.8
	str	w21, [x19, #1744]
	bl	__raw_spin_lock_init
	mov	x0, x20
	bl	mas_blk_dev_lld_init_unistore
	add	x20, x19, #1160         // =1160
	add	x8, x19, #1776          // =1776
	mov	x0, x20
	str	x8, [x19, #1776]
	str	x8, [x19, #1784]
	str	w21, [x19, #124]
	str	x19, [x19, #128]
	bl	mas_blk_busyidle_state_init
	ldr	x8, [x19, #144]
	cbz	w0, .LBB37_2
// %bb.1:
	and	x8, x8, #0xfffffffffffffff7
	str	x8, [x19, #144]
.LBB37_2:
	tbz	w8, #5, .LBB37_4
// %bb.3:
	orr	w9, wzr, #0x1
	strb	w9, [x20]
.LBB37_4:
	orr	x8, x8, #0x40
	str	x8, [x19, #144]
	str	xzr, [x19, #1808]
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldr	x21, [sp], #48          // 8-byte Folded Reload
	ret
.Lfunc_end37:
	.size	mas_blk_allocated_tags_init, .Lfunc_end37-mas_blk_allocated_tags_init
                                        // -- End function
	.globl	blk_add_queue_tags      // -- Begin function blk_add_queue_tags
	.p2align	2
	.type	blk_add_queue_tags,@function
blk_add_queue_tags:                     // @blk_add_queue_tags
// %bb.0:
	str	x21, [sp, #-48]!        // 8-byte Folded Spill
	add	x21, x0, #40            // =40
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	mov	x20, x0
	mov	x0, x21
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	mov	x19, x1
	bl	mutex_lock
	ldr	x9, [x20, #112]
	add	x8, x19, #1, lsl #12    // =4096
	add	x10, x20, #104          // =104
	mov	x0, x21
	str	x8, [x20, #112]
	str	x10, [x19, #4096]
	str	x9, [x19, #4104]
	str	x8, [x9]
	bl	mutex_unlock
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldr	x21, [sp], #48          // 8-byte Folded Reload
	ret
.Lfunc_end38:
	.size	blk_add_queue_tags, .Lfunc_end38-blk_add_queue_tags
                                        // -- End function
	.globl	mas_blk_mq_allocated_tagset_init // -- Begin function mas_blk_mq_allocated_tagset_init
	.p2align	2
	.type	mas_blk_mq_allocated_tagset_init,@function
mas_blk_mq_allocated_tagset_init:       // @mas_blk_mq_allocated_tagset_init
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	mov	w8, #122
	mov	x20, x0
	adrp	x1, .L.str.7
	adrp	x2, mas_blk_dev_lld_init.__key
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	mov	x19, x0
	str	w8, [x20, #16]!
	str	wzr, [x0, #1612]
	add	x0, x0, #1616           // =1616
	add	x1, x1, :lo12:.L.str.7
	add	x2, x2, :lo12:mas_blk_dev_lld_init.__key
	add	x29, sp, #16            // =16
	bl	__raw_spin_lock_init
	adrp	x1, .L.str.9
	adrp	x2, mas_blk_dev_lld_init.__key.8
	orr	w8, wzr, #0x1
	add	x0, x19, #1648          // =1648
	add	x1, x1, :lo12:.L.str.9
	add	x2, x2, :lo12:mas_blk_dev_lld_init.__key.8
	str	w8, [x19, #1640]
	bl	__raw_spin_lock_init
	mov	x0, x20
	bl	mas_blk_dev_lld_init_unistore
	add	x20, x19, #1056         // =1056
	add	x8, x19, #1672          // =1672
	orr	w9, wzr, #0x2
	mov	x0, x20
	str	x8, [x19, #1672]
	str	x8, [x19, #1680]
	str	w9, [x19, #20]
	str	x19, [x19, #24]
	bl	mas_blk_busyidle_state_init
	ldr	x8, [x19, #40]
	cbz	w0, .LBB39_2
// %bb.1:
	and	x8, x8, #0xfffffffffffffff7
	str	x8, [x19, #40]
.LBB39_2:
	tbz	w8, #5, .LBB39_4
// %bb.3:
	orr	w9, wzr, #0x1
	strb	w9, [x20]
.LBB39_4:
	adrp	x10, mas_ufs_blk_tagset_ops
	orr	x9, x8, #0x40
	lsl	x8, x8, #59
	add	x10, x10, :lo12:mas_ufs_blk_tagset_ops
	and	x8, x10, x8, asr #63
	str	xzr, [x19, #1704]
	str	x9, [x19, #40]
	str	x8, [x19, #1712]
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end39:
	.size	mas_blk_mq_allocated_tagset_init, .Lfunc_end39-mas_blk_mq_allocated_tagset_init
                                        // -- End function
	.globl	mas_blk_queue_register  // -- Begin function mas_blk_queue_register
	.p2align	2
	.type	mas_blk_queue_register,@function
mas_blk_queue_register:                 // @mas_blk_queue_register
// %bb.0:
	str	x1, [x0, #1552]
	ret
.Lfunc_end40:
	.size	mas_blk_queue_register, .Lfunc_end40-mas_blk_queue_register
                                        // -- End function
	.section	.init.text,"ax",@progbits
	.globl	mas_blk_dev_init        // -- Begin function mas_blk_dev_init
	.p2align	2
	.type	mas_blk_dev_init,@function
mas_blk_dev_init:                       // @mas_blk_dev_init
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	mov	x29, sp
	bl	mas_blk_dump_init
	bl	mas_blk_latency_init
	bl	mas_blk_dsm_init
	mov	w0, wzr
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end41:
	.size	mas_blk_dev_init, .Lfunc_end41-mas_blk_dev_init
                                        // -- End function
	.text
	.globl	blk_write_throttle      // -- Begin function blk_write_throttle
	.p2align	2
	.type	blk_write_throttle,@function
blk_write_throttle:                     // @blk_write_throttle
// %bb.0:
	cbz	x0, .LBB42_4
// %bb.1:
	ldr	x8, [x0, #1816]
	cbz	x8, .LBB42_4
// %bb.2:
	ldr	x8, [x8, #152]
	cbz	x8, .LBB42_4
// %bb.3:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	cmp	w1, #0                  // =0
	cset	w1, ne
	mov	x29, sp
	blr	x8
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
.LBB42_4:
	ret
.Lfunc_end42:
	.size	blk_write_throttle, .Lfunc_end42-blk_write_throttle
                                        // -- End function
	.globl	blk_generic_freeze      // -- Begin function blk_generic_freeze
	.p2align	2
	.type	blk_generic_freeze,@function
blk_generic_freeze:                     // @blk_generic_freeze
// %bb.0:
	str	x21, [sp, #-48]!        // 8-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	cbz	x0, .LBB43_18
// %bb.1:
	mov	w19, w2
	cmp	w1, #1                  // =1
	b.eq	.LBB43_8
// %bb.2:
	cbnz	w1, .LBB43_18
// %bb.3:
	ldr	w8, [x0, #4]
	cmp	w8, #2                  // =2
	b.eq	.LBB43_11
// %bb.4:
	cbnz	w8, .LBB43_18
// %bb.5:
	ldr	x0, [x0, #8]
	ldr	x8, [x0, #312]
	cbz	x8, .LBB43_18
// %bb.6:
	tbnz	w19, #0, .LBB43_10
.LBB43_7:
	bl	blk_mq_unfreeze_queue
	b	.LBB43_18
.LBB43_8:
	ldr	x8, [x0, #312]
	cbz	x8, .LBB43_18
// %bb.9:
	tbz	w19, #0, .LBB43_7
.LBB43_10:
	bl	blk_mq_freeze_queue
	b	.LBB43_18
.LBB43_11:
	ldr	x8, [x0, #8]
	ldr	x20, [x8, #1832]
	add	x21, x8, #1832          // =1832
	cmp	x21, x20
	b.eq	.LBB43_18
.LBB43_12:                              // =>This Inner Loop Header: Depth=1
	subs	x0, x20, #1, lsl #12    // =4096
	b.eq	.LBB43_17
// %bb.13:                              //   in Loop: Header=BB43_12 Depth=1
	sub	x8, x20, #3784          // =3784
	ldr	x8, [x8]
	cbz	x8, .LBB43_17
// %bb.14:                              //   in Loop: Header=BB43_12 Depth=1
	tbz	w19, #0, .LBB43_16
// %bb.15:                              //   in Loop: Header=BB43_12 Depth=1
	bl	blk_mq_freeze_queue
	ldr	x20, [x20]
	cmp	x21, x20
	b.ne	.LBB43_12
	b	.LBB43_18
.LBB43_16:                              //   in Loop: Header=BB43_12 Depth=1
	bl	blk_mq_unfreeze_queue
.LBB43_17:                              //   in Loop: Header=BB43_12 Depth=1
	ldr	x20, [x20]
	cmp	x21, x20
	b.ne	.LBB43_12
.LBB43_18:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldr	x21, [sp], #48          // 8-byte Folded Reload
	ret
.Lfunc_end43:
	.size	blk_generic_freeze, .Lfunc_end43-blk_generic_freeze
                                        // -- End function
	.globl	blk_dev_write_order_preserved // -- Begin function blk_dev_write_order_preserved
	.p2align	2
	.type	blk_dev_write_order_preserved,@function
blk_dev_write_order_preserved:          // @blk_dev_write_order_preserved
// %bb.0:
	cbz	x0, .LBB44_6
// %bb.1:
	ldr	x8, [x0, #160]
	cbz	x8, .LBB44_7
// %bb.2:
	ldr	x8, [x8, #1120]
	cbz	x8, .LBB44_8
// %bb.3:
	ldr	x9, [x8, #312]
	cbz	x9, .LBB44_9
// %bb.4:
	ldr	x9, [x8, #4088]
	add	x8, x8, #1824           // =1824
	add	x10, x9, #16            // =16
.LBB44_5:
	cmp	x9, #0                  // =0
	csel	x8, x8, x10, eq
	ldrb	w8, [x8, #24]
	lsr	w0, w8, #7
.LBB44_6:
	ret
.LBB44_7:
	mov	w0, wzr
	ret
.LBB44_8:
	mov	w0, wzr
	ret
.LBB44_9:
	ldr	x9, [x8, #720]
	add	x8, x8, #1824           // =1824
	add	x10, x9, #120           // =120
	b	.LBB44_5
.Lfunc_end44:
	.size	blk_dev_write_order_preserved, .Lfunc_end44-blk_dev_write_order_preserved
                                        // -- End function
	.globl	blk_req_set_make_req_nr // -- Begin function blk_req_set_make_req_nr
	.p2align	2
	.type	blk_req_set_make_req_nr,@function
blk_req_set_make_req_nr:                // @blk_req_set_make_req_nr
// %bb.0:
	str	x21, [sp, #-48]!        // 8-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	ldr	x8, [x0, #64]
	mov	x19, x0
	add	x29, sp, #32            // =32
	ldr	x9, [x8, #312]
	cbz	x9, .LBB45_8
// %bb.1:
	ldr	x9, [x8, #4088]
	add	x8, x8, #1824           // =1824
	add	x10, x9, #16            // =16
.LBB45_2:
	cmp	x9, #0                  // =0
	csel	x21, x8, x10, eq
	ldr	x8, [x21, #24]
	tbz	w8, #8, .LBB45_4
// %bb.3:
	ldr	w8, [x19, #84]
	mov	w9, #59391
	movk	w9, #65343, lsl #16
	and	w8, w8, w9
	orr	w8, w8, #0x800
	str	w8, [x19, #84]
	ldr	x8, [x21, #24]
.LBB45_4:
	tst	x8, #0x180
	b.eq	.LBB45_7
// %bb.5:
	ldr	x8, [x19, #272]
	add	x20, x21, #1632         // =1632
	mov	x0, x20
	orr	x8, x8, #0x4
	str	x8, [x19, #272]
	bl	_raw_spin_lock_irqsave
	ldr	w8, [x21, #1624]
	mov	x1, x0
	cbz	w8, .LBB45_9
.LBB45_6:
	add	w9, w8, #1              // =1
	mov	x0, x20
	str	w9, [x21, #1624]
	str	w8, [x19, #464]
	bl	_raw_spin_unlock_irqrestore
.LBB45_7:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldr	x21, [sp], #48          // 8-byte Folded Reload
	ret
.LBB45_8:
	ldr	x9, [x8, #720]
	add	x8, x8, #1824           // =1824
	add	x10, x9, #120           // =120
	b	.LBB45_2
.LBB45_9:
	orr	w8, wzr, #0x1
	str	w8, [x21, #1624]
	b	.LBB45_6
.Lfunc_end45:
	.size	blk_req_set_make_req_nr, .Lfunc_end45-blk_req_set_make_req_nr
                                        // -- End function
	.globl	blk_req_get_order_nr    // -- Begin function blk_req_get_order_nr
	.p2align	2
	.type	blk_req_get_order_nr,@function
blk_req_get_order_nr:                   // @blk_req_get_order_nr
// %bb.0:
	stp	x22, x21, [sp, #-48]!   // 16-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	ldrb	w8, [x0, #272]
	mov	x19, x0
	add	x29, sp, #32            // =32
	tbnz	w8, #1, .LBB46_6
// %bb.1:
	ldr	x8, [x19, #64]
	mov	w20, w1
	ldr	x9, [x8, #312]
	cbz	x9, .LBB46_13
// %bb.2:
	ldr	x9, [x8, #4088]
	add	x8, x8, #1824           // =1824
	add	x10, x9, #16            // =16
.LBB46_3:
	ldr	w1, [x19, #468]
	cmp	x9, #0                  // =0
	csel	x22, x8, x10, eq
	cbz	w1, .LBB46_5
// %bb.4:
	adrp	x0, .L.str.5
	add	x0, x0, :lo12:.L.str.5
	bl	printk
	ldr	w21, [x19, #468]
	b	.LBB46_8
.LBB46_5:
	ldrb	w8, [x22, #24]
	tbnz	w8, #7, .LBB46_9
.LBB46_6:
	mov	w21, wzr
.LBB46_7:
	str	w21, [x19, #468]
.LBB46_8:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	mov	w0, w21
	ldp	x22, x21, [sp], #48     // 16-byte Folded Reload
	ret
.LBB46_9:
                                        // implicit-def: $x1
	tbnz	w20, #0, .LBB46_11
// %bb.10:
	add	x0, x22, #1600          // =1600
	bl	_raw_spin_lock_irqsave
	mov	x1, x0
.LBB46_11:
	ldr	w8, [x22, #1596]
	adds	w9, w8, #1              // =1
	add	w8, w8, #2              // =2
	csel	w21, w8, w9, hs
	str	w21, [x22, #1596]
	tbnz	w20, #0, .LBB46_7
// %bb.12:
	add	x0, x22, #1600          // =1600
	bl	_raw_spin_unlock_irqrestore
	b	.LBB46_7
.LBB46_13:
	ldr	x9, [x8, #720]
	add	x8, x8, #1824           // =1824
	add	x10, x9, #120           // =120
	b	.LBB46_3
.Lfunc_end46:
	.size	blk_req_get_order_nr, .Lfunc_end46-blk_req_get_order_nr
                                        // -- End function
	.globl	blk_queue_order_enable  // -- Begin function blk_queue_order_enable
	.p2align	2
	.type	blk_queue_order_enable,@function
blk_queue_order_enable:                 // @blk_queue_order_enable
// %bb.0:
	ldr	x8, [x0, #312]
	cbz	x8, .LBB47_3
// %bb.1:
	ldr	x8, [x0, #4088]
	add	x9, x0, #1824           // =1824
	add	x10, x8, #16            // =16
.LBB47_2:
	cmp	x8, #0                  // =0
	csel	x8, x9, x10, eq
	ldr	x9, [x8, #24]
	tst	w1, #0x1
	and	w10, w1, #0x1
	strb	w10, [x8, #1592]
	and	x11, x9, #0xffffffffffffff7f
	orr	x9, x9, #0x80
	csel	x9, x9, x11, ne
	str	x9, [x8, #24]
	ret
.LBB47_3:
	ldr	x8, [x0, #720]
	add	x9, x0, #1824           // =1824
	add	x10, x8, #120           // =120
	b	.LBB47_2
.Lfunc_end47:
	.size	blk_queue_order_enable, .Lfunc_end47-blk_queue_order_enable
                                        // -- End function
	.globl	blk_queue_query_order_enable // -- Begin function blk_queue_query_order_enable
	.p2align	2
	.type	blk_queue_query_order_enable,@function
blk_queue_query_order_enable:           // @blk_queue_query_order_enable
// %bb.0:
	ldr	x8, [x0, #312]
	cbz	x8, .LBB48_3
// %bb.1:
	ldr	x8, [x0, #4088]
	add	x9, x0, #1824           // =1824
	add	x10, x8, #16            // =16
.LBB48_2:
	cmp	x8, #0                  // =0
	csel	x8, x9, x10, eq
	ldrb	w8, [x8, #24]
	lsr	w0, w8, #7
	ret
.LBB48_3:
	ldr	x8, [x0, #720]
	add	x9, x0, #1824           // =1824
	add	x10, x8, #120           // =120
	b	.LBB48_2
.Lfunc_end48:
	.size	blk_queue_query_order_enable, .Lfunc_end48-blk_queue_query_order_enable
                                        // -- End function
	.globl	blk_order_nr_reset      // -- Begin function blk_order_nr_reset
	.p2align	2
	.type	blk_order_nr_reset,@function
blk_order_nr_reset:                     // @blk_order_nr_reset
// %bb.0:
	cbz	x0, .LBB49_3
// %bb.1:
	ldrb	w8, [x0, #41]
	str	wzr, [x0, #1612]
	tbz	w8, #0, .LBB49_3
// %bb.2:
	orr	w8, wzr, #0xff
	strh	w8, [x0, #276]
	str	xzr, [x0, #1048]
.LBB49_3:
	ret
.Lfunc_end49:
	.size	blk_order_nr_reset, .Lfunc_end49-blk_order_nr_reset
                                        // -- End function
	.globl	blk_queue_cp_enable     // -- Begin function blk_queue_cp_enable
	.p2align	2
	.type	blk_queue_cp_enable,@function
blk_queue_cp_enable:                    // @blk_queue_cp_enable
// %bb.0:
	ldr	x8, [x0, #312]
	cbz	x8, .LBB50_3
// %bb.1:
	ldr	x8, [x0, #4088]
	add	x9, x0, #1824           // =1824
	add	x10, x8, #16            // =16
.LBB50_2:
	cmp	x8, #0                  // =0
	csel	x8, x9, x10, eq
	ldr	x9, [x8, #24]
	tst	w1, #0x1
	and	x10, x9, #0xffffffffffffffbf
	orr	x9, x9, #0x40
	csel	x9, x9, x10, ne
	str	x9, [x8, #24]
	ret
.LBB50_3:
	ldr	x8, [x0, #720]
	add	x9, x0, #1824           // =1824
	add	x10, x8, #120           // =120
	b	.LBB50_2
.Lfunc_end50:
	.size	blk_queue_cp_enable, .Lfunc_end50-blk_queue_cp_enable
                                        // -- End function
	.globl	mas_queue_status_show   // -- Begin function mas_queue_status_show
	.p2align	2
	.type	mas_queue_status_show,@function
mas_queue_status_show:                  // @mas_queue_status_show
// %bb.0:
	stp	x22, x21, [sp, #-48]!   // 16-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	ldr	x8, [x0, #312]
	mov	x20, x2
	mov	x19, x0
	mov	x21, x1
	add	x29, sp, #32            // =32
	cbz	x8, .LBB51_3
// %bb.1:
	ldr	x8, [x19, #4088]
	add	x9, x19, #1824          // =1824
	add	x10, x8, #16            // =16
.LBB51_2:
	cmp	x8, #0                  // =0
	csel	x0, x9, x10, eq
	mov	x1, x21
	mov	w2, w20
	bl	mas_blk_dump_lld_status
	mov	w22, w0
	add	x1, x21, w0, sxtw
	sub	w2, w20, w0
	mov	x0, x19
	bl	mas_blk_dump_queue_status2
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	add	w8, w0, w22
	sxtw	x0, w8
	ldp	x22, x21, [sp], #48     // 16-byte Folded Reload
	ret
.LBB51_3:
	ldr	x8, [x19, #720]
	add	x9, x19, #1824          // =1824
	add	x10, x8, #120           // =120
	b	.LBB51_2
.Lfunc_end51:
	.size	mas_queue_status_show, .Lfunc_end51-mas_queue_status_show
                                        // -- End function
	.type	.L.str.1,@object        // @.str.1
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.1:
	.asciz	"\0013[BLK-IO]%s - section size = %u\n"
	.size	.L.str.1, 34

	.type	.L__func__.mas_blk_bio_get_residual_byte,@object // @__func__.mas_blk_bio_get_residual_byte
.L__func__.mas_blk_bio_get_residual_byte:
	.asciz	"mas_blk_bio_get_residual_byte"
	.size	.L__func__.mas_blk_bio_get_residual_byte, 30

	.type	.L.str.2,@object        // @.str.2
.L.str.2:
	.asciz	"\0013[BLK-IO]%s: mq_iosched_init_fn failed. err = %d\n"
	.size	.L.str.2, 51

	.type	.L__func__.mas_blk_mq_init_allocated_queue,@object // @__func__.mas_blk_mq_init_allocated_queue
.L__func__.mas_blk_mq_init_allocated_queue:
	.asciz	"mas_blk_mq_init_allocated_queue"
	.size	.L__func__.mas_blk_mq_init_allocated_queue, 32

	.type	mas_blk_allocated_tags_init.__key,@object // @mas_blk_allocated_tags_init.__key
	.local	mas_blk_allocated_tags_init.__key
	.comm	mas_blk_allocated_tags_init.__key,1,1
	.type	.L.str.3,@object        // @.str.3
.L.str.3:
	.asciz	"&tags->tag_list_lock"
	.size	.L.str.3, 21

	.type	__kstrtab_blk_dev_write_order_preserved,@object // @__kstrtab_blk_dev_write_order_preserved
	.section	__ksymtab_strings,"a",@progbits
__kstrtab_blk_dev_write_order_preserved:
	.asciz	"blk_dev_write_order_preserved"
	.size	__kstrtab_blk_dev_write_order_preserved, 30

	.type	__ksymtab_blk_dev_write_order_preserved,@object // @__ksymtab_blk_dev_write_order_preserved
	.section	"___ksymtab+blk_dev_write_order_preserved","a",@progbits
	.p2align	3
__ksymtab_blk_dev_write_order_preserved:
	.xword	blk_dev_write_order_preserved
	.xword	__kstrtab_blk_dev_write_order_preserved
	.size	__ksymtab_blk_dev_write_order_preserved, 16

	.type	.L.str.5,@object        // @.str.5
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.5:
	.asciz	"\0013[BLK-IO]old protocol_nr exist! %u\n"
	.size	.L.str.5, 37

	.type	__kstrtab_blk_req_get_order_nr,@object // @__kstrtab_blk_req_get_order_nr
	.section	__ksymtab_strings,"a",@progbits
__kstrtab_blk_req_get_order_nr:
	.asciz	"blk_req_get_order_nr"
	.size	__kstrtab_blk_req_get_order_nr, 21

	.type	__ksymtab_blk_req_get_order_nr,@object // @__ksymtab_blk_req_get_order_nr
	.section	"___ksymtab+blk_req_get_order_nr","a",@progbits
	.p2align	3
__ksymtab_blk_req_get_order_nr:
	.xword	blk_req_get_order_nr
	.xword	__kstrtab_blk_req_get_order_nr
	.size	__ksymtab_blk_req_get_order_nr, 16

	.type	__kstrtab_blk_queue_order_enable,@object // @__kstrtab_blk_queue_order_enable
	.section	__ksymtab_strings,"a",@progbits
__kstrtab_blk_queue_order_enable:
	.asciz	"blk_queue_order_enable"
	.size	__kstrtab_blk_queue_order_enable, 23

	.type	__ksymtab_blk_queue_order_enable,@object // @__ksymtab_blk_queue_order_enable
	.section	"___ksymtab+blk_queue_order_enable","a",@progbits
	.p2align	3
__ksymtab_blk_queue_order_enable:
	.xword	blk_queue_order_enable
	.xword	__kstrtab_blk_queue_order_enable
	.size	__ksymtab_blk_queue_order_enable, 16

	.type	__kstrtab_blk_queue_query_order_enable,@object // @__kstrtab_blk_queue_query_order_enable
	.section	__ksymtab_strings,"a",@progbits
__kstrtab_blk_queue_query_order_enable:
	.asciz	"blk_queue_query_order_enable"
	.size	__kstrtab_blk_queue_query_order_enable, 29

	.type	__ksymtab_blk_queue_query_order_enable,@object // @__ksymtab_blk_queue_query_order_enable
	.section	"___ksymtab+blk_queue_query_order_enable","a",@progbits
	.p2align	3
__ksymtab_blk_queue_query_order_enable:
	.xword	blk_queue_query_order_enable
	.xword	__kstrtab_blk_queue_query_order_enable
	.size	__ksymtab_blk_queue_query_order_enable, 16

	.type	__kstrtab_blk_order_nr_reset,@object // @__kstrtab_blk_order_nr_reset
	.section	__ksymtab_strings,"a",@progbits
__kstrtab_blk_order_nr_reset:
	.asciz	"blk_order_nr_reset"
	.size	__kstrtab_blk_order_nr_reset, 19

	.type	__ksymtab_blk_order_nr_reset,@object // @__ksymtab_blk_order_nr_reset
	.section	"___ksymtab+blk_order_nr_reset","a",@progbits
	.p2align	3
__ksymtab_blk_order_nr_reset:
	.xword	blk_order_nr_reset
	.xword	__kstrtab_blk_order_nr_reset
	.size	__ksymtab_blk_order_nr_reset, 16

	.type	.L.str.6,@object        // @.str.6
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.6:
	.asciz	"\0013[BLK-IO]%s - wrong bytes\n"
	.size	.L.str.6, 28

	.type	.L__func__.mas_blk_bio_split_in_bytes,@object // @__func__.mas_blk_bio_split_in_bytes
.L__func__.mas_blk_bio_split_in_bytes:
	.asciz	"mas_blk_bio_split_in_bytes"
	.size	.L__func__.mas_blk_bio_split_in_bytes, 27

	.type	mas_blk_dev_lld_init.__key,@object // @mas_blk_dev_lld_init.__key
	.local	mas_blk_dev_lld_init.__key
	.comm	mas_blk_dev_lld_init.__key,1,1
	.type	.L.str.7,@object        // @.str.7
.L.str.7:
	.asciz	"&(&blk_lld->write_num_lock)->rlock"
	.size	.L.str.7, 35

	.type	mas_blk_dev_lld_init.__key.8,@object // @mas_blk_dev_lld_init.__key.8
	.local	mas_blk_dev_lld_init.__key.8
	.comm	mas_blk_dev_lld_init.__key.8,1,1
	.type	.L.str.9,@object        // @.str.9
.L.str.9:
	.asciz	"&(&blk_lld->make_req_nr_lock)->rlock"
	.size	.L.str.9, 37


	.ident	"Android (27847 based on r353983c) clang version 9.0.3 (https://android.googlesource.com/toolchain/clang dbdb7a2acf33dc76cd6f506c323253b8d892ae05) (https://android.googlesource.com/toolchain/llvm f32df87f6769ededcd9e2a6c3c4c45c1baf361c2) (based on LLVM 9.0.3svn)"
	.section	".note.GNU-stack","",@progbits

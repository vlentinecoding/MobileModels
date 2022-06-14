	.text
	.file	"mas_blk_dump.c"
	.globl	mas_blk_dump_request    // -- Begin function mas_blk_dump_request
	.p2align	2
	.type	mas_blk_dump_request,@function
mas_blk_dump_request:                   // @mas_blk_dump_request
// %bb.0:
	stp	x28, x27, [sp, #-96]!   // 16-byte Folded Spill
	stp	x26, x25, [sp, #16]     // 16-byte Folded Spill
	stp	x24, x23, [sp, #32]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #48]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #64]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #80]     // 16-byte Folded Spill
	add	x29, sp, #80            // =80
	sub	sp, sp, #560            // =560
	adrp	x8, __stack_chk_guard
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	add	x22, sp, #8             // =8
	mov	x19, x0
	mov	w20, w1
	str	x8, [x22]
	ldr	w21, [x0, #84]
	ldr	x23, [x0, #232]
	and	w8, w21, #0xff
	cmp	w8, #7                  // =7
	b.hi	.LBB0_6
// %bb.1:
	adrp	x9, .LJTI0_0
	and	x8, x21, #0xff
	add	x9, x9, :lo12:.LJTI0_0
	adr	x10, .LBB0_2
	ldrb	w11, [x9, x8]
	add	x10, x10, x11, lsl #2
	br	x10
.LBB0_2:
	mov	w8, #82
	b	.LBB0_9
.LBB0_3:
	mov	w8, #87
	b	.LBB0_9
.LBB0_4:
	mov	w8, #70
	b	.LBB0_9
.LBB0_5:
	mov	w8, #68
	b	.LBB0_9
.LBB0_6:
	adrp	x0, .L.str.18
	adrp	x1, .L__func__.mas_blk_io_type_op
	add	x0, x0, :lo12:.L.str.18
	add	x1, x1, :lo12:.L__func__.mas_blk_io_type_op
	bl	printk
	mov	w8, wzr
	b	.LBB0_10
.LBB0_7:
	mov	w8, #69
	b	.LBB0_9
.LBB0_8:
	mov	w8, #84
.LBB0_9:
	strb	w8, [sp, #20]
	orr	w8, wzr, #0x1
.LBB0_10:
	add	x9, sp, #20             // =20
	orr	x10, x9, x8
	tbnz	w21, #18, .LBB0_12
// %bb.11:
	mov	w9, wzr
	mov	w11, #10
	tbnz	w21, #11, .LBB0_13
	b	.LBB0_14
.LBB0_12:
	mov	w9, #65
	strb	w9, [x10]
	orr	w9, wzr, #0x1
	mov	w11, #10
	tbz	w21, #11, .LBB0_14
.LBB0_13:
	mov	w12, #83
	strb	w12, [x10, w9, uxtw]
	add	w9, w9, #1              // =1
.LBB0_14:
	sub	w11, w11, w8
	tbz	w21, #12, .LBB0_17
// %bb.15:
	cmp	w9, w11
	b.ge	.LBB0_17
// %bb.16:
	mov	w12, #77
	strb	w12, [x10, w9, uxtw]
	add	w9, w9, #1              // =1
.LBB0_17:
	tbz	w21, #19, .LBB0_20
// %bb.18:
	cmp	w9, w11
	b.ge	.LBB0_20
// %bb.19:
	mov	w11, #70
	strb	w11, [x10, w9, uxtw]
	add	w9, w9, #1              // =1
.LBB0_20:
	mov	w10, wzr
	add	w8, w9, w8
	mov	w11, #10
	tbz	w23, #1, .LBB0_23
// %bb.21:
	sub	w9, w11, w8
	cmp	w9, #1                  // =1
	b.lt	.LBB0_23
// %bb.22:
	add	x9, sp, #20             // =20
	mov	w10, #76
	strb	w10, [x9, w8, uxtw]
	orr	w10, wzr, #0x1
.LBB0_23:
	add	w8, w10, w8
	add	x10, sp, #20            // =20
	mov	w9, wzr
	add	x10, x10, x8
	sub	w11, w11, w8
	tbz	w21, #22, .LBB0_26
// %bb.24:
	cmp	w11, #1                 // =1
	b.lt	.LBB0_26
// %bb.25:
	mov	w9, #72
	strb	w9, [x10]
	orr	w9, wzr, #0x1
.LBB0_26:
	tbz	w21, #28, .LBB0_29
// %bb.27:
	cmp	w9, w11
	b.ge	.LBB0_29
// %bb.28:
	mov	w12, #67
	strb	w12, [x10, w9, uxtw]
	add	w9, w9, #1              // =1
.LBB0_29:
	tbz	w21, #27, .LBB0_32
// %bb.30:
	cmp	w9, w11
	b.ge	.LBB0_32
// %bb.31:
	mov	w12, #90
	strb	w12, [x10, w9, uxtw]
	add	w9, w9, #1              // =1
.LBB0_32:
	tbz	w21, #23, .LBB0_35
// %bb.33:
	cmp	w9, w11
	b.ge	.LBB0_35
// %bb.34:
	mov	w12, #86
	strb	w12, [x10, w9, uxtw]
	add	w9, w9, #1              // =1
.LBB0_35:
	tbz	w21, #26, .LBB0_38
// %bb.36:
	cmp	w9, w11
	b.ge	.LBB0_38
// %bb.37:
	mov	w11, #84
	strb	w11, [x10, w9, uxtw]
	add	w9, w9, #1              // =1
.LBB0_38:
	add	w8, w9, w8
	add	x9, sp, #20             // =20
	strb	wzr, [x9, w8, uxtw]
	ldr	w8, [x19, #84]
	adrp	x10, .L.str.20
	adrp	x11, .L.str.19
	add	x10, x10, :lo12:.L.str.20
	and	w8, w8, #0xfe
	add	x11, x11, :lo12:.L.str.19
	cmp	w20, #1                 // =1
	adrp	x12, .L.str.23
	adrp	x9, .L.str.22
	orr	w8, w8, #0x1
	add	x12, x12, :lo12:.L.str.23
	add	x9, x9, :lo12:.L.str.22
	csel	x20, x11, x10, eq
	ldr	w7, [x19, #284]
	cmp	w8, #35                 // =35
	ldp	w5, w10, [x19, #104]
	csel	x9, x9, x12, eq
	cmp	w8, #33                 // =33
	adrp	x8, .L.str.21
	add	x8, x8, :lo12:.L.str.21
	adrp	x2, .L.str
	csel	x6, x8, x9, eq
	add	x2, x2, :lo12:.L.str
	add	x0, sp, #32             // =32
	add	x4, sp, #20             // =20
	orr	w1, wzr, #0x200
	mov	x3, x19
	str	w10, [sp]
	orr	w24, wzr, #0x200
	add	x25, sp, #32            // =32
	bl	snprintf
	ldr	x8, [x19, #272]
	mov	w9, w0
	adrp	x2, .L.str.1
	mov	w21, w0
	add	x0, x25, x9
	sub	x1, x24, x9
	and	x3, x8, #0x1
	and	x4, x8, #0x2
	add	x2, x2, :lo12:.L.str.1
	bl	snprintf
	ldr	x8, [x19, #240]
	add	w23, w0, w21
	add	x0, x25, x23
	sub	x1, x24, x23
	cbz	x8, .LBB0_40
// %bb.39:
	ldr	w3, [x19, #248]
	adrp	x2, .L.str.2
	add	x4, x19, #256           // =256
	add	x2, x2, :lo12:.L.str.2
	bl	snprintf
	b	.LBB0_41
.LBB0_40:
	adrp	x2, .L.str.3
	add	x2, x2, :lo12:.L.str.3
	bl	snprintf
.LBB0_41:
	add	x24, x19, #312          // =312
	adrp	x25, req_stage_cfg
	adrp	x19, .L.str.4
	mov	x21, xzr
	add	w23, w0, w23
	add	x25, x25, :lo12:req_stage_cfg
	add	x26, sp, #32            // =32
	orr	w27, wzr, #0x200
	add	x19, x19, :lo12:.L.str.4
.LBB0_42:                               // =>This Inner Loop Header: Depth=1
	ldr	x4, [x24, x21]
	cbz	x4, .LBB0_44
// %bb.43:                              //   in Loop: Header=BB0_42 Depth=1
	ldr	x3, [x25]
	mov	w8, w23
	add	x0, x26, x8
	sub	x1, x27, x8
	mov	x2, x19
	bl	snprintf
	add	w23, w0, w23
.LBB0_44:                               //   in Loop: Header=BB0_42 Depth=1
	add	x21, x21, #8            // =8
	cmp	x21, #120               // =120
	add	x25, x25, #16           // =16
	b.ne	.LBB0_42
// %bb.45:
	adrp	x0, .L.str.5
	add	x0, x0, :lo12:.L.str.5
	add	x2, sp, #32             // =32
	mov	x1, x20
	strb	wzr, [sp, #543]
	bl	printk
	adrp	x9, __stack_chk_guard
	ldr	x8, [x22]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB0_47
// %bb.46:
	add	sp, sp, #560            // =560
	ldp	x29, x30, [sp, #80]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #64]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #48]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #32]     // 16-byte Folded Reload
	ldp	x26, x25, [sp, #16]     // 16-byte Folded Reload
	ldp	x28, x27, [sp], #96     // 16-byte Folded Reload
	ret
.LBB0_47:
	bl	__stack_chk_fail
.Lfunc_end0:
	.size	mas_blk_dump_request, .Lfunc_end0-mas_blk_dump_request
	.section	.rodata,"a",@progbits
.LJTI0_0:
	.byte	(.LBB0_2-.LBB0_2)>>2
	.byte	(.LBB0_3-.LBB0_2)>>2
	.byte	(.LBB0_4-.LBB0_2)>>2
	.byte	(.LBB0_5-.LBB0_2)>>2
	.byte	(.LBB0_6-.LBB0_2)>>2
	.byte	(.LBB0_7-.LBB0_2)>>2
	.byte	(.LBB0_6-.LBB0_2)>>2
	.byte	(.LBB0_8-.LBB0_2)>>2
                                        // -- End function
	.text
	.globl	mas_blk_dump_bio        // -- Begin function mas_blk_dump_bio
	.p2align	2
	.type	mas_blk_dump_bio,@function
mas_blk_dump_bio:                       // @mas_blk_dump_bio
// %bb.0:
	str	x28, [sp, #-80]!        // 8-byte Folded Spill
	stp	x24, x23, [sp, #16]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #32]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #48]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #64]     // 16-byte Folded Spill
	add	x29, sp, #64            // =64
	sub	sp, sp, #544            // =544
	adrp	x8, __stack_chk_guard
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	mov	x23, sp
	mov	x19, x0
	mov	w20, w1
	str	x8, [x23]
	ldr	w21, [x0, #16]
	and	w8, w21, #0xff
	cmp	w8, #7                  // =7
	b.hi	.LBB1_6
// %bb.1:
	adrp	x9, .LJTI1_0
	and	x8, x21, #0xff
	add	x9, x9, :lo12:.LJTI1_0
	adr	x10, .LBB1_2
	ldrb	w11, [x9, x8]
	add	x10, x10, x11, lsl #2
	br	x10
.LBB1_2:
	mov	w8, #82
	b	.LBB1_9
.LBB1_3:
	mov	w8, #87
	b	.LBB1_9
.LBB1_4:
	mov	w8, #70
	b	.LBB1_9
.LBB1_5:
	mov	w8, #68
	b	.LBB1_9
.LBB1_6:
	adrp	x0, .L.str.18
	adrp	x1, .L__func__.mas_blk_io_type_op
	add	x0, x0, :lo12:.L.str.18
	add	x1, x1, :lo12:.L__func__.mas_blk_io_type_op
	bl	printk
	mov	w8, wzr
	b	.LBB1_10
.LBB1_7:
	mov	w8, #69
	b	.LBB1_9
.LBB1_8:
	mov	w8, #84
.LBB1_9:
	strb	w8, [sp, #12]
	orr	w8, wzr, #0x1
.LBB1_10:
	add	x9, sp, #12             // =12
	orr	x9, x9, x8
	tbnz	w21, #18, .LBB1_12
// %bb.11:
	mov	w10, wzr
	mov	w11, #10
	tbnz	w21, #11, .LBB1_13
	b	.LBB1_14
.LBB1_12:
	mov	w10, #65
	strb	w10, [x9]
	orr	w10, wzr, #0x1
	mov	w11, #10
	tbz	w21, #11, .LBB1_14
.LBB1_13:
	mov	w12, #83
	strb	w12, [x9, w10, uxtw]
	add	w10, w10, #1            // =1
.LBB1_14:
	sub	w11, w11, w8
	tbz	w21, #12, .LBB1_17
// %bb.15:
	cmp	w10, w11
	b.ge	.LBB1_17
// %bb.16:
	mov	w12, #77
	strb	w12, [x9, w10, uxtw]
	add	w10, w10, #1            // =1
.LBB1_17:
	tbz	w21, #19, .LBB1_20
// %bb.18:
	cmp	w10, w11
	b.ge	.LBB1_20
// %bb.19:
	mov	w11, #70
	strb	w11, [x9, w10, uxtw]
	add	w10, w10, #1            // =1
.LBB1_20:
	add	w8, w10, w8
	add	x10, sp, #12            // =12
	mov	w11, #10
	mov	w9, wzr
	add	x10, x10, x8
	sub	w11, w11, w8
	tbz	w21, #22, .LBB1_23
// %bb.21:
	cmp	w11, #1                 // =1
	b.lt	.LBB1_23
// %bb.22:
	mov	w9, #72
	strb	w9, [x10]
	orr	w9, wzr, #0x1
.LBB1_23:
	tbz	w21, #28, .LBB1_26
// %bb.24:
	cmp	w9, w11
	b.ge	.LBB1_26
// %bb.25:
	mov	w12, #67
	strb	w12, [x10, w9, uxtw]
	add	w9, w9, #1              // =1
.LBB1_26:
	tbz	w21, #27, .LBB1_29
// %bb.27:
	cmp	w9, w11
	b.ge	.LBB1_29
// %bb.28:
	mov	w12, #90
	strb	w12, [x10, w9, uxtw]
	add	w9, w9, #1              // =1
.LBB1_29:
	tbz	w21, #23, .LBB1_32
// %bb.30:
	cmp	w9, w11
	b.ge	.LBB1_32
// %bb.31:
	mov	w12, #86
	strb	w12, [x10, w9, uxtw]
	add	w9, w9, #1              // =1
.LBB1_32:
	tbz	w21, #26, .LBB1_35
// %bb.33:
	cmp	w9, w11
	b.ge	.LBB1_35
// %bb.34:
	mov	w11, #84
	strb	w11, [x10, w9, uxtw]
	add	w9, w9, #1              // =1
.LBB1_35:
	add	w8, w9, w8
	add	x9, sp, #12             // =12
	strb	wzr, [x9, w8, uxtw]
	ldr	w5, [x19, #48]
	adrp	x8, .L.str.20
	adrp	x9, .L.str.19
	adrp	x2, .L.str.6
	add	x8, x8, :lo12:.L.str.20
	add	x9, x9, :lo12:.L.str.19
	cmp	w20, #1                 // =1
	add	x2, x2, :lo12:.L.str.6
	add	x0, sp, #24             // =24
	add	x4, sp, #12             // =12
	orr	w1, wzr, #0x200
	mov	x3, x19
	csel	x21, x9, x8, eq
	add	x24, sp, #24            // =24
	bl	snprintf
	ldr	x8, [x19, #112]
	mov	w22, w0
	cbz	x8, .LBB1_37
// %bb.36:
	ldr	w3, [x19, #120]
	sxtw	x8, w22
	orr	w9, wzr, #0x200
	adrp	x2, .L.str.7
	add	x0, x24, x8
	sub	x1, x9, x8
	add	x4, x19, #128           // =128
	add	x2, x2, :lo12:.L.str.7
	bl	snprintf
	add	w22, w0, w22
.LBB1_37:
	sxtw	x8, w22
	orr	w9, wzr, #0x200
	adrp	x2, .L.str.3
	add	x0, x24, x8
	sub	x1, x9, x8
	add	x2, x2, :lo12:.L.str.3
	bl	snprintf
	ldr	x4, [x19, #152]
	add	w22, w0, w22
	cbz	x4, .LBB1_39
// %bb.38:
	adrp	x10, bio_stage_cfg
	ldr	x3, [x10, :lo12:bio_stage_cfg]
	sxtw	x8, w22
	add	x9, sp, #24             // =24
	add	x0, x9, x8
	orr	w9, wzr, #0x200
	adrp	x2, .L.str.8
	sub	x1, x9, x8
	add	x2, x2, :lo12:.L.str.8
	bl	snprintf
	add	w22, w0, w22
.LBB1_39:
	ldr	x4, [x19, #160]
	cbz	x4, .LBB1_41
// %bb.40:
	adrp	x10, bio_stage_cfg+16
	ldr	x3, [x10, :lo12:bio_stage_cfg+16]
	sxtw	x8, w22
	add	x9, sp, #24             // =24
	add	x0, x9, x8
	orr	w9, wzr, #0x200
	adrp	x2, .L.str.8
	sub	x1, x9, x8
	add	x2, x2, :lo12:.L.str.8
	bl	snprintf
	add	w22, w0, w22
.LBB1_41:
	ldr	x4, [x19, #168]
	cbz	x4, .LBB1_43
// %bb.42:
	adrp	x10, bio_stage_cfg+32
	ldr	x3, [x10, :lo12:bio_stage_cfg+32]
	sxtw	x8, w22
	add	x9, sp, #24             // =24
	add	x0, x9, x8
	orr	w9, wzr, #0x200
	adrp	x2, .L.str.8
	sub	x1, x9, x8
	add	x2, x2, :lo12:.L.str.8
	bl	snprintf
	add	w22, w0, w22
.LBB1_43:
	ldr	x4, [x19, #176]
	cbz	x4, .LBB1_45
// %bb.44:
	adrp	x10, bio_stage_cfg+48
	ldr	x3, [x10, :lo12:bio_stage_cfg+48]
	sxtw	x8, w22
	add	x9, sp, #24             // =24
	add	x0, x9, x8
	orr	w9, wzr, #0x200
	adrp	x2, .L.str.8
	sub	x1, x9, x8
	add	x2, x2, :lo12:.L.str.8
	bl	snprintf
	add	w22, w0, w22
.LBB1_45:
	add	x8, sp, #24             // =24
	strb	wzr, [x8, w22, sxtw]
	bl	ktime_get
	mov	x3, x0
	adrp	x0, .L.str.9
	add	x0, x0, :lo12:.L.str.9
	add	x2, sp, #24             // =24
	mov	x1, x21
	bl	printk
	ldr	x0, [x19, #104]
	cbz	x0, .LBB1_47
// %bb.46:
	mov	w1, w20
	bl	mas_blk_dump_request
.LBB1_47:
	adrp	x9, __stack_chk_guard
	ldr	x8, [x23]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB1_49
// %bb.48:
	add	sp, sp, #544            // =544
	ldp	x29, x30, [sp, #64]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #48]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #32]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #16]     // 16-byte Folded Reload
	ldr	x28, [sp], #80          // 8-byte Folded Reload
	ret
.LBB1_49:
	bl	__stack_chk_fail
.Lfunc_end1:
	.size	mas_blk_dump_bio, .Lfunc_end1-mas_blk_dump_bio
	.section	.rodata,"a",@progbits
.LJTI1_0:
	.byte	(.LBB1_2-.LBB1_2)>>2
	.byte	(.LBB1_3-.LBB1_2)>>2
	.byte	(.LBB1_4-.LBB1_2)>>2
	.byte	(.LBB1_5-.LBB1_2)>>2
	.byte	(.LBB1_6-.LBB1_2)>>2
	.byte	(.LBB1_7-.LBB1_2)>>2
	.byte	(.LBB1_6-.LBB1_2)>>2
	.byte	(.LBB1_8-.LBB1_2)>>2
                                        // -- End function
	.text
	.globl	mas_blk_dump_queue_status2 // -- Begin function mas_blk_dump_queue_status2
	.p2align	2
	.type	mas_blk_dump_queue_status2,@function
mas_blk_dump_queue_status2:             // @mas_blk_dump_queue_status2
// %bb.0:
	str	x28, [sp, #-96]!        // 8-byte Folded Spill
	stp	x26, x25, [sp, #16]     // 16-byte Folded Spill
	stp	x24, x23, [sp, #32]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #48]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #64]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #80]     // 16-byte Folded Spill
	add	x29, sp, #80            // =80
	sub	sp, sp, #528            // =528
	adrp	x9, .L.str.24
	adrp	x8, __stack_chk_guard
	add	x9, x9, :lo12:.L.str.24
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	ldur	x10, [x9, #7]
	ldr	x9, [x9]
	mov	w22, w2
	mov	x19, x1
	mov	x21, x0
	stur	x8, [x29, #-88]
	stur	x10, [sp, #15]
	str	x9, [sp, #8]
	cbz	x1, .LBB2_3
// %bb.1:
	cmp	w22, #1                 // =1
	b.lt	.LBB2_3
// %bb.2:
	adrp	x2, .L.str.25
	sxtw	x1, w22
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #8              // =8
	mov	x0, x19
	bl	snprintf
	mov	w20, w0
	b	.LBB2_4
.LBB2_3:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #8              // =8
	bl	printk
	mov	w20, wzr
.LBB2_4:
	ldr	w8, [x21, #1700]
	sub	w24, w22, w20
	cbz	w8, .LBB2_6
// %bb.5:
	ldr	w3, [x21, #1704]
	adrp	x2, .L.str.27
	add	x2, x2, :lo12:.L.str.27
	add	x0, sp, #8              // =8
	orr	w1, wzr, #0x200
	bl	snprintf
	b	.LBB2_7
.LBB2_6:
	adrp	x8, .L.str.28
	add	x8, x8, :lo12:.L.str.28
	ldur	x9, [x8, #22]
	ldp	x11, x10, [x8, #8]
	ldr	x8, [x8]
	stur	x9, [sp, #30]
	stp	x11, x10, [sp, #16]
	str	x8, [sp, #8]
.LBB2_7:
	cmp	w24, #1                 // =1
	add	x22, x19, w20, sxtw
	b.lt	.LBB2_10
// %bb.8:
	cbz	x22, .LBB2_10
// %bb.9:
	adrp	x2, .L.str.25
	sxtw	x1, w24
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #8              // =8
	mov	x0, x22
	bl	snprintf
	mov	w23, w0
	b	.LBB2_11
.LBB2_10:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #8              // =8
	bl	printk
	mov	w23, wzr
.LBB2_11:
	ldr	w8, [x21, #1696]
	adrp	x25, .L.str.30
	adrp	x26, .L.str.31
	add	x25, x25, :lo12:.L.str.30
	add	x26, x26, :lo12:.L.str.31
	cmp	w8, #0                  // =0
	adrp	x2, .L.str.29
	csel	x3, x26, x25, eq
	add	x2, x2, :lo12:.L.str.29
	add	x0, sp, #8              // =8
	orr	w1, wzr, #0x200
	bl	snprintf
	sub	w8, w24, w23
	cmp	w8, #1                  // =1
	b.lt	.LBB2_14
// %bb.12:
	add	x0, x22, w23, sxtw
	cbz	x0, .LBB2_14
// %bb.13:
	adrp	x2, .L.str.25
	sxtw	x1, w8
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #8              // =8
	bl	snprintf
	b	.LBB2_15
.LBB2_14:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #8              // =8
	bl	printk
	mov	w0, wzr
.LBB2_15:
	ldr	w8, [x21, #1536]
	adrp	x2, .L.str.32
	add	w21, w0, w23
	add	x2, x2, :lo12:.L.str.32
	cmp	w8, #0                  // =0
	csel	x3, x26, x25, eq
	add	x0, sp, #8              // =8
	orr	w1, wzr, #0x200
	bl	snprintf
	sub	w8, w24, w21
	cmp	w8, #1                  // =1
	b.lt	.LBB2_18
// %bb.16:
	add	x0, x22, w21, sxtw
	cbz	x0, .LBB2_18
// %bb.17:
	adrp	x2, .L.str.25
	sxtw	x1, w8
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #8              // =8
	bl	snprintf
	b	.LBB2_19
.LBB2_18:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #8              // =8
	bl	printk
	mov	w0, wzr
.LBB2_19:
	add	w8, w21, w20
	add	w0, w8, w0
	cbz	x19, .LBB2_21
// %bb.20:
	strb	wzr, [x19, w0, sxtw]
.LBB2_21:
	adrp	x9, __stack_chk_guard
	ldur	x8, [x29, #-88]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB2_23
// %bb.22:
	add	sp, sp, #528            // =528
	ldp	x29, x30, [sp, #80]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #64]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #48]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #32]     // 16-byte Folded Reload
	ldp	x26, x25, [sp, #16]     // 16-byte Folded Reload
	ldr	x28, [sp], #96          // 8-byte Folded Reload
	ret
.LBB2_23:
	bl	__stack_chk_fail
.Lfunc_end2:
	.size	mas_blk_dump_queue_status2, .Lfunc_end2-mas_blk_dump_queue_status2
                                        // -- End function
	.globl	mas_blk_dump_queue_status // -- Begin function mas_blk_dump_queue_status
	.p2align	2
	.type	mas_blk_dump_queue_status,@function
mas_blk_dump_queue_status:              // @mas_blk_dump_queue_status
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	ldr	x8, [x0, #1552]
	add	x29, sp, #16            // =16
	cbz	x8, .LBB3_6
// %bb.1:
	ldrb	w11, [x0, #1560]
	adrp	x9, .L.str.20
	adrp	x10, .L.str.19
	ldr	w4, [x8, #1016]
	ldr	w5, [x8, #1020]
	add	x9, x9, :lo12:.L.str.20
	add	x10, x10, :lo12:.L.str.19
	cmp	w1, #1                  // =1
	add	x2, x8, #12             // =12
	adrp	x12, .L.str.11
	adrp	x8, .L.str.12
	mov	x19, x0
	mov	w20, w1
	add	x12, x12, :lo12:.L.str.11
	csel	x1, x10, x9, eq
	add	x8, x8, :lo12:.L.str.12
	cmp	w11, #0                 // =0
	adrp	x0, .L.str.10
	csel	x3, x8, x12, eq
	add	x0, x0, :lo12:.L.str.10
	bl	printk
	cmp	w20, #1                 // =1
	b.ne	.LBB3_3
// %bb.2:
	mov	x0, x19
	mov	x1, xzr
	mov	w2, wzr
	bl	mas_blk_dump_queue_status2
.LBB3_3:
	ldr	x8, [x19, #1816]
	cbz	x8, .LBB3_6
// %bb.4:
	ldr	x8, [x8, #160]
	cbz	x8, .LBB3_6
// %bb.5:
	mov	x0, x19
	mov	w1, w20
	blr	x8
.LBB3_6:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end3:
	.size	mas_blk_dump_queue_status, .Lfunc_end3-mas_blk_dump_queue_status
                                        // -- End function
	.globl	mas_blk_dump_lld_status // -- Begin function mas_blk_dump_lld_status
	.p2align	2
	.type	mas_blk_dump_lld_status,@function
mas_blk_dump_lld_status:                // @mas_blk_dump_lld_status
// %bb.0:
	stp	x28, x27, [sp, #-96]!   // 16-byte Folded Spill
	stp	x26, x25, [sp, #16]     // 16-byte Folded Spill
	stp	x24, x23, [sp, #32]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #48]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #64]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #80]     // 16-byte Folded Spill
	add	x29, sp, #80            // =80
	sub	sp, sp, #560            // =560
	adrp	x9, .L.str.33
	adrp	x8, __stack_chk_guard
	add	x9, x9, :lo12:.L.str.33
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	ldur	x10, [x9, #5]
	ldr	x9, [x9]
	mov	w21, w2
	mov	x19, x1
	mov	x20, x0
	stur	x8, [x29, #-96]
	stur	x10, [sp, #37]
	str	x9, [sp, #32]
	cbz	x1, .LBB4_3
// %bb.1:
	cmp	w21, #1                 // =1
	b.lt	.LBB4_3
// %bb.2:
	adrp	x2, .L.str.25
	sxtw	x1, w21
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #32             // =32
	mov	x0, x19
	bl	snprintf
	mov	w22, w0
	b	.LBB4_4
.LBB4_3:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #32             // =32
	bl	printk
	mov	w22, wzr
.LBB4_4:
	ldr	x8, [x20, #24]
	adrp	x24, .L.str.30
	adrp	x25, .L.str.31
	add	x24, x24, :lo12:.L.str.30
	add	x25, x25, :lo12:.L.str.31
	tst	x8, #0x1
	adrp	x2, .L.str.34
	csel	x3, x25, x24, eq
	add	x2, x2, :lo12:.L.str.34
	add	x0, sp, #32             // =32
	orr	w1, wzr, #0x200
	bl	snprintf
	sub	w8, w21, w22
	cmp	w8, #1                  // =1
	b.lt	.LBB4_7
// %bb.5:
	add	x0, x19, w22, sxtw
	cbz	x0, .LBB4_7
// %bb.6:
	adrp	x2, .L.str.25
	sxtw	x1, w8
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #32             // =32
	bl	snprintf
	b	.LBB4_8
.LBB4_7:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #32             // =32
	bl	printk
	mov	w0, wzr
.LBB4_8:
	ldrb	w8, [x20, #24]
	add	w22, w0, w22
	sub	w23, w21, w22
	tbnz	w8, #1, .LBB4_12
// %bb.9:
	adrp	x8, .L.str.36
	add	x8, x8, :lo12:.L.str.36
	ldr	x10, [x8]
	ldp	x8, x9, [x8, #8]
	mov	w11, #25964
	movk	w11, #10, lsl #16
	stur	w11, [sp, #55]
	stp	x8, x9, [sp, #40]
	str	x10, [sp, #32]
	cmp	w23, #1                 // =1
	b.lt	.LBB4_13
.LBB4_10:
	add	x0, x19, w22, sxtw
	cbz	x0, .LBB4_13
// %bb.11:
	adrp	x2, .L.str.25
	sxtw	x1, w23
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #32             // =32
	bl	snprintf
	b	.LBB4_14
.LBB4_12:
	ldr	w3, [x20, #48]
	adrp	x2, .L.str.35
	add	x2, x2, :lo12:.L.str.35
	add	x0, sp, #32             // =32
	orr	w1, wzr, #0x200
	bl	snprintf
	cmp	w23, #1                 // =1
	b.ge	.LBB4_10
.LBB4_13:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #32             // =32
	bl	printk
	mov	w0, wzr
.LBB4_14:
	ldr	x8, [x20, #24]
	adrp	x2, .L.str.37
	add	w26, w0, w22
	add	x2, x2, :lo12:.L.str.37
	tst	x8, #0x8
	csel	x3, x25, x24, eq
	add	x0, sp, #32             // =32
	orr	w1, wzr, #0x200
	add	x22, x19, w26, sxtw
	sub	w27, w21, w26
	bl	snprintf
	cmp	w27, #1                 // =1
	b.lt	.LBB4_17
// %bb.15:
	cbz	x22, .LBB4_17
// %bb.16:
	adrp	x2, .L.str.25
	sxtw	x1, w27
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #32             // =32
	mov	x0, x22
	bl	snprintf
	mov	w23, w0
	b	.LBB4_18
.LBB4_17:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #32             // =32
	bl	printk
	mov	w23, wzr
.LBB4_18:
	ldr	w8, [x20, #1320]
	cmp	w8, #1                  // =1
	b.eq	.LBB4_21
// %bb.19:
	cbnz	w8, .LBB4_31
// %bb.20:
	adrp	x8, .L.str.38
	add	x8, x8, :lo12:.L.str.38
	b	.LBB4_22
.LBB4_21:
	adrp	x8, .L.str.39
	add	x8, x8, :lo12:.L.str.39
.LBB4_22:
	ldur	x9, [x8, #6]
	ldr	x8, [x8]
	stur	x9, [sp, #38]
	str	x8, [sp, #32]
	sub	w8, w27, w23
	cmp	w8, #1                  // =1
	b.lt	.LBB4_25
// %bb.23:
	add	x0, x22, w23, sxtw
	cbz	x0, .LBB4_25
// %bb.24:
	adrp	x2, .L.str.25
	sxtw	x1, w8
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #32             // =32
	bl	snprintf
	b	.LBB4_26
.LBB4_25:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #32             // =32
	bl	printk
	mov	w0, wzr
.LBB4_26:
	ldr	w3, [x20, #1248]
	adrp	x2, .L.str.40
	add	w23, w0, w23
	add	x2, x2, :lo12:.L.str.40
	add	x0, sp, #32             // =32
	orr	w1, wzr, #0x200
	bl	snprintf
	sub	w8, w27, w23
	cmp	w8, #1                  // =1
	b.lt	.LBB4_29
// %bb.27:
	add	x0, x22, w23, sxtw
	cbz	x0, .LBB4_29
// %bb.28:
	adrp	x2, .L.str.25
	sxtw	x1, w8
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #32             // =32
	bl	snprintf
	b	.LBB4_30
.LBB4_29:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #32             // =32
	bl	printk
	mov	w0, wzr
.LBB4_30:
	add	w23, w0, w23
.LBB4_31:
	ldr	x8, [x20, #24]
	add	w26, w23, w26
	tbz	w8, #8, .LBB4_50
// %bb.32:
	tst	x8, #0x8
	adrp	x2, .L.str.41
	csel	x3, x25, x24, eq
	add	x2, x2, :lo12:.L.str.41
	add	x0, sp, #32             // =32
	orr	w1, wzr, #0x200
	add	x22, x19, w26, sxtw
	sub	w27, w21, w26
	bl	snprintf
	cmp	w27, #1                 // =1
	b.lt	.LBB4_35
// %bb.33:
	cbz	x22, .LBB4_35
// %bb.34:
	adrp	x2, .L.str.25
	sxtw	x1, w27
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #32             // =32
	mov	x0, x22
	bl	snprintf
	mov	w23, w0
	b	.LBB4_36
.LBB4_35:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #32             // =32
	bl	printk
	mov	w23, wzr
.LBB4_36:
	ldr	w8, [x20, #1496]
	cmp	w8, #1                  // =1
	b.eq	.LBB4_39
// %bb.37:
	cbnz	w8, .LBB4_49
// %bb.38:
	adrp	x8, .L.str.42
	add	x8, x8, :lo12:.L.str.42
	b	.LBB4_40
.LBB4_39:
	adrp	x8, .L.str.43
	add	x8, x8, :lo12:.L.str.43
.LBB4_40:
	ldp	x8, x9, [x8]
	strb	wzr, [sp, #48]
	stp	x8, x9, [sp, #32]
	sub	w8, w27, w23
	cmp	w8, #1                  // =1
	b.lt	.LBB4_43
// %bb.41:
	add	x0, x22, w23, sxtw
	cbz	x0, .LBB4_43
// %bb.42:
	adrp	x2, .L.str.25
	sxtw	x1, w8
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #32             // =32
	bl	snprintf
	b	.LBB4_44
.LBB4_43:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #32             // =32
	bl	printk
	mov	w0, wzr
.LBB4_44:
	ldr	w3, [x20, #1428]
	adrp	x2, .L.str.44
	add	w23, w0, w23
	add	x2, x2, :lo12:.L.str.44
	add	x0, sp, #32             // =32
	orr	w1, wzr, #0x200
	bl	snprintf
	sub	w8, w27, w23
	cmp	w8, #1                  // =1
	b.lt	.LBB4_47
// %bb.45:
	add	x0, x22, w23, sxtw
	cbz	x0, .LBB4_47
// %bb.46:
	adrp	x2, .L.str.25
	sxtw	x1, w8
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #32             // =32
	bl	snprintf
	b	.LBB4_48
.LBB4_47:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #32             // =32
	bl	printk
	mov	w0, wzr
.LBB4_48:
	add	w23, w0, w23
.LBB4_49:
	add	w26, w23, w26
.LBB4_50:
	adrp	x8, .L.str.45
	add	x8, x8, :lo12:.L.str.45
	ldp	x9, x8, [x8]
	sub	w27, w21, w26
	add	x23, x19, w26, sxtw
	cmp	w27, #1                 // =1
	stp	x9, x8, [sp, #32]
	b.lt	.LBB4_54
// %bb.51:
	cbz	x23, .LBB4_54
// %bb.52:
	adrp	x2, .L.str.25
	sxtw	x1, w27
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #32             // =32
	mov	x0, x23
	bl	snprintf
	mov	w22, w0
	ldrb	w8, [x20, #24]
	tbz	w8, #4, .LBB4_55
.LBB4_53:
	mov	x8, #16717
	movk	x8, #8275, lsl #16
	movk	x8, #20813, lsl #32
	movk	x8, #10, lsl #48
	b	.LBB4_56
.LBB4_54:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #32             // =32
	bl	printk
	mov	w22, wzr
	ldrb	w8, [x20, #24]
	tbnz	w8, #4, .LBB4_53
.LBB4_55:
	adrp	x8, .L.str.47
	add	x8, x8, :lo12:.L.str.47
	ldp	x8, x9, [x8]
	mov	w10, #29285
	movk	w10, #10, lsl #16
	stur	w10, [sp, #47]
	str	x9, [sp, #40]
.LBB4_56:
	str	x8, [sp, #32]
	sub	w8, w27, w22
	cmp	w8, #1                  // =1
	b.lt	.LBB4_59
// %bb.57:
	add	x0, x23, w22, sxtw
	cbz	x0, .LBB4_59
// %bb.58:
	adrp	x2, .L.str.25
	sxtw	x1, w8
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #32             // =32
	bl	snprintf
	b	.LBB4_60
.LBB4_59:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #32             // =32
	bl	printk
	mov	w0, wzr
.LBB4_60:
	ldr	x9, [x20, #24]
	add	w8, w22, w26
	adrp	x2, .L.str.29
	add	w23, w8, w0
	tst	x9, #0x4
	csel	x3, x25, x24, eq
	add	x2, x2, :lo12:.L.str.29
	add	x0, sp, #32             // =32
	orr	w1, wzr, #0x200
	add	x22, x19, w23, sxtw
	sub	w26, w21, w23
	bl	snprintf
	cmp	w26, #1                 // =1
	b.lt	.LBB4_63
// %bb.61:
	cbz	x22, .LBB4_63
// %bb.62:
	adrp	x2, .L.str.25
	sxtw	x1, w26
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #32             // =32
	mov	x0, x22
	bl	snprintf
	mov	w21, w0
	b	.LBB4_64
.LBB4_63:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #32             // =32
	bl	printk
	mov	w21, wzr
.LBB4_64:
	ldr	x8, [x20, #24]
	adrp	x2, .L.str.48
	add	x2, x2, :lo12:.L.str.48
	add	x0, sp, #32             // =32
	tst	x8, #0x40
	csel	x3, x25, x24, eq
	orr	w1, wzr, #0x200
	bl	snprintf
	sub	w8, w26, w21
	cmp	w8, #1                  // =1
	b.lt	.LBB4_67
// %bb.65:
	add	x0, x22, w21, sxtw
	cbz	x0, .LBB4_67
// %bb.66:
	adrp	x2, .L.str.25
	sxtw	x1, w8
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #32             // =32
	bl	snprintf
	b	.LBB4_68
.LBB4_67:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #32             // =32
	bl	printk
	mov	w0, wzr
.LBB4_68:
	ldr	x8, [x20, #24]
	adrp	x2, .L.str.49
	add	w21, w0, w21
	add	x2, x2, :lo12:.L.str.49
	tst	x8, #0x80
	csel	x3, x25, x24, eq
	add	x0, sp, #32             // =32
	orr	w1, wzr, #0x200
	bl	snprintf
	sub	w8, w26, w21
	cmp	w8, #1                  // =1
	b.lt	.LBB4_71
// %bb.69:
	add	x0, x22, w21, sxtw
	cbz	x0, .LBB4_71
// %bb.70:
	adrp	x2, .L.str.25
	sxtw	x1, w8
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #32             // =32
	bl	snprintf
	b	.LBB4_72
.LBB4_71:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #32             // =32
	bl	printk
	mov	w0, wzr
.LBB4_72:
	ldrb	w8, [x20, #25]
	add	w24, w0, w21
	tbnz	w8, #0, .LBB4_74
// %bb.73:
	mov	w8, wzr
	b	.LBB4_93
.LBB4_74:
	ldr	w8, [x20, #216]
	cbz	w8, .LBB4_78
// %bb.75:
	ldrb	w8, [x20, #261]
	ldr	w4, [x20, #1596]
	ldrb	w5, [x20, #260]
	adrp	x9, .L.str.52
	adrp	x10, .L.str.53
	add	x9, x9, :lo12:.L.str.52
	add	x10, x10, :lo12:.L.str.53
	cmp	w8, #0                  // =0
	adrp	x2, .L.str.51
	adrp	x3, .L.str.30
	csel	x6, x10, x9, eq
	add	x2, x2, :lo12:.L.str.51
	add	x3, x3, :lo12:.L.str.30
	add	x0, sp, #32             // =32
	orr	w1, wzr, #0x200
	add	x21, x22, w24, sxtw
	sub	w25, w26, w24
	bl	snprintf
	cmp	w25, #1                 // =1
	b.lt	.LBB4_79
// %bb.76:
	cbz	x21, .LBB4_79
// %bb.77:
	adrp	x2, .L.str.25
	sxtw	x1, w25
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #32             // =32
	mov	x0, x21
	bl	snprintf
	mov	w22, w0
	b	.LBB4_80
.LBB4_78:
	adrp	x0, .L.str.50
	adrp	x1, .L__func__.mas_blk_lld_dump_opt_feature_unist
	add	x0, x0, :lo12:.L.str.50
	add	x1, x1, :lo12:.L__func__.mas_blk_lld_dump_opt_feature_unist
	mov	w2, wzr
	bl	printk
	mov	w8, wzr
	b	.LBB4_93
.LBB4_79:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #32             // =32
	bl	printk
	mov	w22, wzr
.LBB4_80:
	ldr	w8, [x20, #216]
	ldp	x9, x10, [x20, #392]
	ldp	x11, x12, [x20, #408]
	adrp	x2, .L.str.54
	add	x2, x2, :lo12:.L.str.54
	udiv	x3, x9, x8
	udiv	x5, x10, x8
	udiv	x7, x11, x8
	udiv	x13, x12, x8
	msub	x4, x3, x8, x9
	msub	x6, x5, x8, x10
	msub	x9, x7, x8, x11
	msub	x8, x13, x8, x12
	add	x0, sp, #32             // =32
	orr	w1, wzr, #0x200
	stp	x9, x13, [sp]
	str	x8, [sp, #16]
	bl	snprintf
	sub	w8, w25, w22
	cmp	w8, #1                  // =1
	b.lt	.LBB4_83
// %bb.81:
	add	x0, x21, w22, sxtw
	cbz	x0, .LBB4_83
// %bb.82:
	adrp	x2, .L.str.25
	sxtw	x1, w8
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #32             // =32
	bl	snprintf
	b	.LBB4_84
.LBB4_83:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #32             // =32
	bl	printk
	mov	w0, wzr
.LBB4_84:
	ldp	x3, x4, [x20, #424]
	ldp	x5, x6, [x20, #440]
	adrp	x2, .L.str.55
	add	w22, w0, w22
	add	x2, x2, :lo12:.L.str.55
	add	x0, sp, #32             // =32
	orr	w1, wzr, #0x200
	bl	snprintf
	sub	w8, w25, w22
	cmp	w8, #1                  // =1
	b.lt	.LBB4_87
// %bb.85:
	add	x0, x21, w22, sxtw
	cbz	x0, .LBB4_87
// %bb.86:
	adrp	x2, .L.str.25
	sxtw	x1, w8
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #32             // =32
	bl	snprintf
	b	.LBB4_88
.LBB4_87:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #32             // =32
	bl	printk
	mov	w0, wzr
.LBB4_88:
	ldp	x3, x4, [x20, #456]
	ldp	x5, x6, [x20, #472]
	adrp	x2, .L.str.56
	add	w20, w0, w22
	add	x2, x2, :lo12:.L.str.56
	add	x0, sp, #32             // =32
	orr	w1, wzr, #0x200
	bl	snprintf
	sub	w8, w25, w20
	cmp	w8, #1                  // =1
	b.lt	.LBB4_91
// %bb.89:
	add	x0, x21, w20, sxtw
	cbz	x0, .LBB4_91
// %bb.90:
	adrp	x2, .L.str.25
	sxtw	x1, w8
	add	x2, x2, :lo12:.L.str.25
	add	x3, sp, #32             // =32
	bl	snprintf
	b	.LBB4_92
.LBB4_91:
	adrp	x0, .L.str.26
	add	x0, x0, :lo12:.L.str.26
	add	x1, sp, #32             // =32
	bl	printk
	mov	w0, wzr
.LBB4_92:
	add	w8, w0, w20
.LBB4_93:
	add	w9, w24, w23
	add	w0, w9, w8
	cbz	x19, .LBB4_95
// %bb.94:
	strb	wzr, [x19, w0, sxtw]
.LBB4_95:
	adrp	x9, __stack_chk_guard
	ldur	x8, [x29, #-96]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB4_97
// %bb.96:
	add	sp, sp, #560            // =560
	ldp	x29, x30, [sp, #80]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #64]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #48]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #32]     // 16-byte Folded Reload
	ldp	x26, x25, [sp, #16]     // 16-byte Folded Reload
	ldp	x28, x27, [sp], #96     // 16-byte Folded Reload
	ret
.LBB4_97:
	bl	__stack_chk_fail
.Lfunc_end4:
	.size	mas_blk_dump_lld_status, .Lfunc_end4-mas_blk_dump_lld_status
                                        // -- End function
	.globl	mas_blk_panic_notify    // -- Begin function mas_blk_panic_notify
	.p2align	2
	.type	mas_blk_panic_notify,@function
mas_blk_panic_notify:                   // @mas_blk_panic_notify
// %bb.0:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	bl	ktime_get
	adrp	x19, .L__func__.mas_blk_panic_notify
	mov	x2, x0
	adrp	x0, .L.str.13
	add	x19, x19, :lo12:.L__func__.mas_blk_panic_notify
	add	x0, x0, :lo12:.L.str.13
	mov	x1, x19
	bl	printk
	bl	mas_blk_panic_dump
	adrp	x0, .L.str.14
	add	x0, x0, :lo12:.L.str.14
	mov	x1, x19
	bl	printk
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	mov	w0, wzr
	ldr	x19, [sp], #32          // 8-byte Folded Reload
	ret
.Lfunc_end5:
	.size	mas_blk_panic_notify, .Lfunc_end5-mas_blk_panic_notify
                                        // -- End function
	.p2align	2               // -- Begin function mas_blk_panic_dump
	.type	mas_blk_panic_dump,@function
mas_blk_panic_dump:                     // @mas_blk_panic_dump
// %bb.0:
	stp	x28, x27, [sp, #-96]!   // 16-byte Folded Spill
	stp	x26, x25, [sp, #16]     // 16-byte Folded Spill
	adrp	x26, dump_list
	add	x26, x26, :lo12:dump_list
	ldr	x27, [x26]
	stp	x24, x23, [sp, #32]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #48]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #64]     // 16-byte Folded Spill
	cmp	x27, x26
	stp	x29, x30, [sp, #80]     // 16-byte Folded Spill
	add	x29, sp, #80            // =80
	b.eq	.LBB6_22
// %bb.1:
	adrp	x20, .L.str.58
	adrp	x21, .L.str.10
	adrp	x22, .L.str.19
	adrp	x23, .L.str.57
	add	x20, x20, :lo12:.L.str.58
	add	x21, x21, :lo12:.L.str.10
	add	x22, x22, :lo12:.L.str.19
	add	x23, x23, :lo12:.L.str.57
.LBB6_2:                                // =>This Loop Header: Depth=1
                                        //     Child Loop BB6_11 Depth 2
                                        //     Child Loop BB6_16 Depth 2
	sub	x24, x27, #1712         // =1712
	mov	x0, x24
	bl	mas_blk_get_lld
	ldr	x8, [x0, #40]
	mov	x25, x0
	cbz	x8, .LBB6_4
// %bb.3:                               //   in Loop: Header=BB6_2 Depth=1
	mov	x0, x25
	mov	x1, xzr
	mov	w2, wzr
	bl	mas_blk_dump_lld_status
	ldr	x1, [x25, #1504]
	ldr	x2, [x25, #1512]
	mov	x0, x23
	bl	printk
	ldr	x8, [x25, #40]
	orr	w1, wzr, #0x1
	mov	x0, x24
	blr	x8
	str	xzr, [x25, #40]
.LBB6_4:                                //   in Loop: Header=BB6_2 Depth=1
	ldur	x8, [x27, #-160]
	cbz	x8, .LBB6_8
// %bb.5:                               //   in Loop: Header=BB6_2 Depth=1
	ldurb	w9, [x27, #-152]
	ldr	w4, [x8, #1016]
	ldr	w5, [x8, #1020]
	add	x2, x8, #12             // =12
	cmp	w9, #0                  // =0
	adrp	x8, .L.str.11
	adrp	x9, .L.str.12
	add	x8, x8, :lo12:.L.str.11
	add	x9, x9, :lo12:.L.str.12
	csel	x3, x9, x8, eq
	mov	x0, x21
	mov	x1, x22
	bl	printk
	mov	x0, x24
	mov	x1, xzr
	mov	w2, wzr
	bl	mas_blk_dump_queue_status2
	ldr	x8, [x27, #104]
	cbz	x8, .LBB6_8
// %bb.6:                               //   in Loop: Header=BB6_2 Depth=1
	ldr	x8, [x8, #160]
	cbz	x8, .LBB6_8
// %bb.7:                               //   in Loop: Header=BB6_2 Depth=1
	orr	w1, wzr, #0x1
	mov	x0, x24
	blr	x8
.LBB6_8:                                //   in Loop: Header=BB6_2 Depth=1
	ldr	x8, [x25, #1520]
	add	x19, x25, #1520         // =1520
	cmp	x19, x8
	b.eq	.LBB6_13
// %bb.9:                               //   in Loop: Header=BB6_2 Depth=1
	mov	x0, x20
	bl	printk
	ldr	x28, [x19]
	cmp	x19, x28
	b.ne	.LBB6_11
	b	.LBB6_13
.LBB6_10:                               //   in Loop: Header=BB6_11 Depth=2
	ldr	x28, [x28]
	cmp	x19, x28
	b.eq	.LBB6_13
.LBB6_11:                               //   Parent Loop BB6_2 Depth=1
                                        // =>  This Inner Loop Header: Depth=2
	ldur	x8, [x28, #-200]
	cmp	x8, x24
	b.ne	.LBB6_10
// %bb.12:                              //   in Loop: Header=BB6_11 Depth=2
	sub	x0, x28, #296           // =296
	orr	w1, wzr, #0x1
	bl	mas_blk_dump_bio
	b	.LBB6_10
.LBB6_13:                               //   in Loop: Header=BB6_2 Depth=1
	ldr	x8, [x25, #1536]
	add	x25, x25, #1536         // =1536
	cmp	x25, x8
	b.eq	.LBB6_18
// %bb.14:                              //   in Loop: Header=BB6_2 Depth=1
	adrp	x0, .L.str.59
	add	x0, x0, :lo12:.L.str.59
	bl	printk
	ldr	x19, [x25]
	cmp	x25, x19
	b.ne	.LBB6_16
	b	.LBB6_18
.LBB6_15:                               //   in Loop: Header=BB6_16 Depth=2
	ldr	x19, [x19]
	cmp	x25, x19
	b.eq	.LBB6_18
.LBB6_16:                               //   Parent Loop BB6_2 Depth=1
                                        // =>  This Inner Loop Header: Depth=2
	sub	x8, x19, #424           // =424
	ldr	x8, [x8]
	cmp	x8, x24
	b.ne	.LBB6_15
// %bb.17:                              //   in Loop: Header=BB6_16 Depth=2
	sub	x0, x19, #488           // =488
	orr	w1, wzr, #0x1
	bl	mas_blk_dump_request
	b	.LBB6_15
.LBB6_18:                               //   in Loop: Header=BB6_2 Depth=1
	ldr	x8, [x27, #104]
	cbz	x8, .LBB6_21
// %bb.19:                              //   in Loop: Header=BB6_2 Depth=1
	ldr	x8, [x8, #168]
	cbz	x8, .LBB6_21
// %bb.20:                              //   in Loop: Header=BB6_2 Depth=1
	orr	w1, wzr, #0x1
	mov	x0, x24
	blr	x8
.LBB6_21:                               //   in Loop: Header=BB6_2 Depth=1
	ldr	x27, [x27]
	cmp	x27, x26
	b.ne	.LBB6_2
.LBB6_22:
	ldp	x29, x30, [sp, #80]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #64]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #48]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #32]     // 16-byte Folded Reload
	ldp	x26, x25, [sp, #16]     // 16-byte Folded Reload
	ldp	x28, x27, [sp], #96     // 16-byte Folded Reload
	ret
.Lfunc_end6:
	.size	mas_blk_panic_dump, .Lfunc_end6-mas_blk_panic_dump
                                        // -- End function
	.globl	mas_blk_dump_unistore   // -- Begin function mas_blk_dump_unistore
	.p2align	2
	.type	mas_blk_dump_unistore,@function
mas_blk_dump_unistore:                  // @mas_blk_dump_unistore
// %bb.0:
	sub	sp, sp, #48             // =48
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	mov	x20, x1
	mov	x19, x0
	bl	blk_queue_query_unistore_enable
	tbz	w0, #0, .LBB7_5
// %bb.1:
	adrp	x0, .L.str.15
	adrp	x1, .L__func__.mas_blk_dump_unistore
	add	x0, x0, :lo12:.L.str.15
	add	x1, x1, :lo12:.L__func__.mas_blk_dump_unistore
	mov	x2, x20
	bl	printk
	mov	x0, x19
	bl	mas_blk_get_lld
	cbz	x0, .LBB7_4
// %bb.2:
	ldr	w8, [x0, #216]
	mov	x19, x0
	cbz	w8, .LBB7_4
// %bb.3:
	ldp	x9, x10, [x19, #392]
	ldp	x11, x12, [x19, #408]
	adrp	x0, .L.str.16
	add	x0, x0, :lo12:.L.str.16
	udiv	x1, x9, x8
	udiv	x3, x10, x8
	udiv	x5, x11, x8
	udiv	x7, x12, x8
	msub	x2, x1, x8, x9
	msub	x4, x3, x8, x10
	msub	x6, x5, x8, x11
	msub	x8, x7, x8, x12
	str	x8, [sp]
	bl	printk
	ldp	x1, x2, [x19, #456]
	ldp	x3, x4, [x19, #472]
	adrp	x0, .L.str.17
	add	x0, x0, :lo12:.L.str.17
	bl	printk
.LBB7_4:
	bl	mas_blk_panic_dump
.LBB7_5:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	add	sp, sp, #48             // =48
	ret
.Lfunc_end7:
	.size	mas_blk_dump_unistore, .Lfunc_end7-mas_blk_dump_unistore
                                        // -- End function
	.globl	mas_blk_dump_register_queue // -- Begin function mas_blk_dump_register_queue
	.p2align	2
	.type	mas_blk_dump_register_queue,@function
mas_blk_dump_register_queue:            // @mas_blk_dump_register_queue
// %bb.0:
	str	x21, [sp, #-48]!        // 8-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	mov	x19, x0
	bl	mas_blk_get_lld
	ldrb	w8, [x0, #24]
	tbz	w8, #0, .LBB8_3
// %bb.1:
	ldr	x8, [x19, #1712]
	add	x21, x19, #1712         // =1712
	cmp	x21, x8
	b.ne	.LBB8_3
// %bb.2:
	adrp	x20, dump_list_lock
	add	x20, x20, :lo12:dump_list_lock
	mov	x0, x20
	bl	_raw_spin_lock
	adrp	x8, dump_list
	add	x8, x8, :lo12:dump_list
	ldr	x9, [x8, #8]
	mov	x0, x20
	str	x21, [x8, #8]
	str	x8, [x19, #1712]
	str	x9, [x19, #1720]
	str	x21, [x9]
	bl	_raw_spin_unlock
.LBB8_3:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldr	x21, [sp], #48          // 8-byte Folded Reload
	ret
.Lfunc_end8:
	.size	mas_blk_dump_register_queue, .Lfunc_end8-mas_blk_dump_register_queue
                                        // -- End function
	.globl	mas_blk_dump_unregister_queue // -- Begin function mas_blk_dump_unregister_queue
	.p2align	2
	.type	mas_blk_dump_unregister_queue,@function
mas_blk_dump_unregister_queue:          // @mas_blk_dump_unregister_queue
// %bb.0:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	mov	x19, x0
	adrp	x0, dump_list_lock
	add	x0, x0, :lo12:dump_list_lock
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	bl	_raw_spin_lock
	adrp	x9, dump_list
	add	x9, x9, :lo12:dump_list
	mov	x8, x9
.LBB9_1:                                // =>This Inner Loop Header: Depth=1
	ldr	x8, [x8]
	cmp	x8, x9
	b.eq	.LBB9_4
// %bb.2:                               //   in Loop: Header=BB9_1 Depth=1
	sub	x10, x8, #1712          // =1712
	cmp	x10, x19
	b.ne	.LBB9_1
// %bb.3:
	ldp	x9, x10, [x8]
	mov	x11, #256
	movk	x11, #57005, lsl #48
	add	x12, x11, #256          // =256
	str	x10, [x9, #8]
	str	x9, [x10]
	stp	x11, x12, [x8]
.LBB9_4:
	adrp	x0, dump_list_lock
	add	x0, x0, :lo12:dump_list_lock
	bl	_raw_spin_unlock
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldr	x19, [sp], #32          // 8-byte Folded Reload
	ret
.Lfunc_end9:
	.size	mas_blk_dump_unregister_queue, .Lfunc_end9-mas_blk_dump_unregister_queue
                                        // -- End function
	.section	.init.text,"ax",@progbits
	.globl	mas_blk_dump_init       // -- Begin function mas_blk_dump_init
	.p2align	2
	.type	mas_blk_dump_init,@function
mas_blk_dump_init:                      // @mas_blk_dump_init
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	adrp	x0, panic_notifier_list
	adrp	x1, mas_blk_panic_nb
	add	x0, x0, :lo12:panic_notifier_list
	add	x1, x1, :lo12:mas_blk_panic_nb
	mov	x29, sp
	bl	atomic_notifier_chain_register
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end10:
	.size	mas_blk_dump_init, .Lfunc_end10-mas_blk_dump_init
                                        // -- End function
	.text
	.globl	blk_mq_tagset_dump_register // -- Begin function blk_mq_tagset_dump_register
	.p2align	2
	.type	blk_mq_tagset_dump_register,@function
blk_mq_tagset_dump_register:            // @blk_mq_tagset_dump_register
// %bb.0:
	ldr	x8, [x0, #40]
	str	x1, [x0, #56]
	orr	x8, x8, #0x1
	str	x8, [x0, #40]
	ret
.Lfunc_end11:
	.size	blk_mq_tagset_dump_register, .Lfunc_end11-blk_mq_tagset_dump_register
                                        // -- End function
	.globl	blk_queue_dump_register // -- Begin function blk_queue_dump_register
	.p2align	2
	.type	blk_queue_dump_register,@function
blk_queue_dump_register:                // @blk_queue_dump_register
// %bb.0:
	str	x21, [sp, #-48]!        // 8-byte Folded Spill
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	mov	x20, x1
	mov	x19, x0
	bl	mas_blk_get_lld
	ldr	x8, [x0, #24]
	str	x20, [x0, #40]
	orr	x8, x8, #0x1
	str	x8, [x0, #24]
	mov	x0, x19
	bl	mas_blk_get_lld
	ldrb	w8, [x0, #24]
	tbz	w8, #0, .LBB12_3
// %bb.1:
	ldr	x8, [x19, #1712]
	add	x21, x19, #1712         // =1712
	cmp	x21, x8
	b.ne	.LBB12_3
// %bb.2:
	adrp	x20, dump_list_lock
	add	x20, x20, :lo12:dump_list_lock
	mov	x0, x20
	bl	_raw_spin_lock
	adrp	x8, dump_list
	add	x8, x8, :lo12:dump_list
	ldr	x9, [x8, #8]
	mov	x0, x20
	str	x21, [x8, #8]
	str	x8, [x19, #1712]
	str	x9, [x19, #1720]
	str	x21, [x9]
	bl	_raw_spin_unlock
.LBB12_3:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldr	x21, [sp], #48          // 8-byte Folded Reload
	ret
.Lfunc_end12:
	.size	blk_queue_dump_register, .Lfunc_end12-blk_queue_dump_register
                                        // -- End function
	.type	.L.str,@object          // @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	"rq: 0x%pK, type:%s len:%u %s requeue_reason: %d tag: %d "
	.size	.L.str, 57

	.type	.L.str.1,@object        // @.str.1
.L.str.1:
	.asciz	"rq_hoq_flag: %llx rq_cp_flag: %llx "
	.size	.L.str.1, 36

	.type	.L.str.2,@object        // @.str.2
.L.str.2:
	.asciz	"disp_pid: %d %s\n"
	.size	.L.str.2, 17

	.type	.L.str.3,@object        // @.str.3
.L.str.3:
	.asciz	"\n"
	.size	.L.str.3, 2

	.type	.L.str.4,@object        // @.str.4
.L.str.4:
	.asciz	"%s: %lld "
	.size	.L.str.4, 10

	.type	.L.str.5,@object        // @.str.5
.L.str.5:
	.asciz	"\0013[BLK-IO]%s: %s\n"
	.size	.L.str.5, 18

	.type	.L.str.6,@object        // @.str.6
.L.str.6:
	.asciz	"bio: 0x%pK, type:%s len:%u "
	.size	.L.str.6, 28

	.type	.L.str.7,@object        // @.str.7
.L.str.7:
	.asciz	"disp_pid: %d %s"
	.size	.L.str.7, 16

	.type	.L.str.8,@object        // @.str.8
.L.str.8:
	.asciz	"<%s:%lld> "
	.size	.L.str.8, 11

	.type	.L.str.9,@object        // @.str.9
.L.str.9:
	.asciz	"\0013[BLK-IO]%s %s,current: %lld\n"
	.size	.L.str.9, 31

	.type	.L.str.10,@object       // @.str.10
.L.str.10:
	.asciz	"\0013[BLK-IO]%s: bdev %s, ptable %s, r_inflt: %d w_inflt: %d"
	.size	.L.str.10, 58

	.type	.L.str.11,@object       // @.str.11
.L.str.11:
	.asciz	"exist"
	.size	.L.str.11, 6

	.type	.L.str.12,@object       // @.str.12
.L.str.12:
	.asciz	"none"
	.size	.L.str.12, 5

	.type	.L.str.13,@object       // @.str.13
.L.str.13:
	.asciz	"\0013[BLK-IO]%s: ++ current = %lld\n"
	.size	.L.str.13, 33

	.type	.L__func__.mas_blk_panic_notify,@object // @__func__.mas_blk_panic_notify
.L__func__.mas_blk_panic_notify:
	.asciz	"mas_blk_panic_notify"
	.size	.L__func__.mas_blk_panic_notify, 21

	.type	.L.str.14,@object       // @.str.14
.L.str.14:
	.asciz	"\0013[BLK-IO]%s: --\n"
	.size	.L.str.14, 18

	.type	.L.str.15,@object       // @.str.15
.L.str.15:
	.asciz	"\0013[BLK-IO]%s - %s\n"
	.size	.L.str.15, 19

	.type	.L__func__.mas_blk_dump_unistore,@object // @__func__.mas_blk_dump_unistore
.L__func__.mas_blk_dump_unistore:
	.asciz	"mas_blk_dump_unistore"
	.size	.L__func__.mas_blk_dump_unistore, 22

	.type	.L.str.16,@object       // @.str.16
.L.str.16:
	.asciz	"\0013[BLK-IO]expected_lba: 0x%llx - 0x%llx, 0x%llx - 0x%llx, 0x%llx - 0x%llx, 0x%llx - 0x%llx\n"
	.size	.L.str.16, 92

	.type	.L.str.17,@object       // @.str.17
.L.str.17:
	.asciz	"\0013[BLK-IO]old_section: 0x%llx, 0x%llx, 0x%llx, 0x%llx\n"
	.size	.L.str.17, 55

	.type	dump_list_lock,@object  // @dump_list_lock
	.data
	.p2align	3
dump_list_lock:
	.zero	4
	.word	3735899821              // 0xdead4ead
	.word	4294967295              // 0xffffffff
	.zero	4
	.xword	-1
	.size	dump_list_lock, 24

	.type	dump_list,@object       // @dump_list
	.p2align	3
dump_list:
	.xword	dump_list
	.xword	dump_list
	.size	dump_list, 16

	.type	mas_blk_panic_nb,@object // @mas_blk_panic_nb
	.p2align	3
mas_blk_panic_nb:
	.xword	__cfi_mas_blk_panic_notify
	.xword	0
	.word	0                       // 0x0
	.zero	4
	.size	mas_blk_panic_nb, 24

	.type	.L.str.18,@object       // @.str.18
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.18:
	.asciz	"\0014[BLK-IO]%s: Unknown OP!\n"
	.size	.L.str.18, 27

	.type	.L__func__.mas_blk_io_type_op,@object // @__func__.mas_blk_io_type_op
.L__func__.mas_blk_io_type_op:
	.asciz	"mas_blk_io_type_op"
	.size	.L__func__.mas_blk_io_type_op, 19

	.type	.L.str.19,@object       // @.str.19
.L.str.19:
	.asciz	"dump"
	.size	.L.str.19, 5

	.type	.L.str.20,@object       // @.str.20
.L.str.20:
	.asciz	"io_latency"
	.size	.L.str.20, 11

	.type	.L.str.21,@object       // @.str.21
.L.str.21:
	.asciz	"TYPE_BLOCK_PC"
	.size	.L.str.21, 14

	.type	.L.str.22,@object       // @.str.22
.L.str.22:
	.asciz	"TYPE_DRV_PRIV"
	.size	.L.str.22, 14

	.type	.L.str.23,@object       // @.str.23
.L.str.23:
	.asciz	"TYPE_FS"
	.size	.L.str.23, 8

	.type	.L.str.24,@object       // @.str.24
.L.str.24:
	.asciz	"Queue Status:\n"
	.size	.L.str.24, 15

	.type	.L.str.25,@object       // @.str.25
.L.str.25:
	.asciz	"%s"
	.size	.L.str.25, 3

	.type	.L.str.26,@object       // @.str.26
.L.str.26:
	.asciz	"\0013[BLK-IO]%s"
	.size	.L.str.26, 13

	.type	.L.str.27,@object       // @.str.27
.L.str.27:
	.asciz	"\tIO Latency Warning: enable\tThreshold(ms): %u\n"
	.size	.L.str.27, 47

	.type	.L.str.28,@object       // @.str.28
.L.str.28:
	.asciz	"\tIO Latency Warning: disable\n"
	.size	.L.str.28, 30

	.type	.L.str.29,@object       // @.str.29
.L.str.29:
	.asciz	"\tFlush Reduce: %s\n"
	.size	.L.str.29, 19

	.type	.L.str.30,@object       // @.str.30
.L.str.30:
	.asciz	"enable"
	.size	.L.str.30, 7

	.type	.L.str.31,@object       // @.str.31
.L.str.31:
	.asciz	"disable"
	.size	.L.str.31, 8

	.type	.L.str.32,@object       // @.str.32
.L.str.32:
	.asciz	"\tInline Crypto: %s\n"
	.size	.L.str.32, 20

	.type	.L.str.33,@object       // @.str.33
.L.str.33:
	.asciz	"LLD Status:\n"
	.size	.L.str.33, 13

	.type	.L.str.34,@object       // @.str.34
.L.str.34:
	.asciz	"\tPanic Dump: %s\n"
	.size	.L.str.34, 17

	.type	.L.str.35,@object       // @.str.35
.L.str.35:
	.asciz	"\tLatency Warning:enable\tThreshold(ms):%u\n"
	.size	.L.str.35, 42

	.type	.L.str.36,@object       // @.str.36
.L.str.36:
	.asciz	"\tLatency Warning: disable\n"
	.size	.L.str.36, 27

	.type	.L.str.37,@object       // @.str.37
.L.str.37:
	.asciz	"\tBusy/Idle Notifier: %s\t"
	.size	.L.str.37, 25

	.type	.L.str.38,@object       // @.str.38
.L.str.38:
	.asciz	"\tState: BUSY\t"
	.size	.L.str.38, 14

	.type	.L.str.39,@object       // @.str.39
.L.str.39:
	.asciz	"\tState: IDLE\t"
	.size	.L.str.39, 14

	.type	.L.str.40,@object       // @.str.40
.L.str.40:
	.asciz	"\tIO Count: %d\n"
	.size	.L.str.40, 15

	.type	.L.str.41,@object       // @.str.41
.L.str.41:
	.asciz	"\tFGIO/Busy/Idle Notifier: %s\t"
	.size	.L.str.41, 30

	.type	.L.str.42,@object       // @.str.42
.L.str.42:
	.asciz	"\tFG State: BUSY\t"
	.size	.L.str.42, 17

	.type	.L.str.43,@object       // @.str.43
.L.str.43:
	.asciz	"\tFG State: IDLE\t"
	.size	.L.str.43, 17

	.type	.L.str.44,@object       // @.str.44
.L.str.44:
	.asciz	"\tFG IO Count: %d\n"
	.size	.L.str.44, 18

	.type	.L.str.45,@object       // @.str.45
.L.str.45:
	.asciz	"\tIO Scheduler: "
	.size	.L.str.45, 16

	.type	.L.str.47,@object       // @.str.47
.L.str.47:
	.asciz	"Default Scheduler\n"
	.size	.L.str.47, 19

	.type	.L.str.48,@object       // @.str.48
.L.str.48:
	.asciz	"\tUFS command priority: %s\n"
	.size	.L.str.48, 27

	.type	.L.str.49,@object       // @.str.49
.L.str.49:
	.asciz	"\tUFS order preserving: %s\n"
	.size	.L.str.49, 27

	.type	.L.str.50,@object       // @.str.50
.L.str.50:
	.asciz	"\0013[BLK-IO]%s - section size error: %u\n"
	.size	.L.str.50, 39

	.type	.L__func__.mas_blk_lld_dump_opt_feature_unist,@object // @__func__.mas_blk_lld_dump_opt_feature_unist
.L__func__.mas_blk_lld_dump_opt_feature_unist:
	.asciz	"mas_blk_lld_dump_opt_feature_unist"
	.size	.L__func__.mas_blk_lld_dump_opt_feature_unist, 35

	.type	.L.str.51,@object       // @.str.51
.L.str.51:
	.asciz	"Write Order: %s Order Num: %uLAST STREAM TYPE: %u, fsync ind: %s\n"
	.size	.L.str.51, 66

	.type	.L.str.52,@object       // @.str.52
.L.str.52:
	.asciz	"true"
	.size	.L.str.52, 5

	.type	.L.str.53,@object       // @.str.53
.L.str.53:
	.asciz	"false"
	.size	.L.str.53, 6

	.type	.L.str.54,@object       // @.str.54
.L.str.54:
	.asciz	"expected_lba: 0x%llx, 0x%llx, 0x%llx, 0x%llx, 0x%llx, 0x%llx, 0x%llx, 0x%llx\n"
	.size	.L.str.54, 78

	.type	.L.str.55,@object       // @.str.55
.L.str.55:
	.asciz	"expected_lba_time: %llu, %llu, %llu, %llu.\n"
	.size	.L.str.55, 44

	.type	.L.str.56,@object       // @.str.56
.L.str.56:
	.asciz	"old_section:0x%llx, 0x%llx, 0x%llx, 0x%llx\n"
	.size	.L.str.56, 44

	.type	.L.str.57,@object       // @.str.57
.L.str.57:
	.asciz	"\0013[BLK-IO]bio_count = %llu req_count = %llu\n"
	.size	.L.str.57, 45

	.type	.L.str.58,@object       // @.str.58
.L.str.58:
	.asciz	"\0013[BLK-IO]bio_list:\n"
	.size	.L.str.58, 21

	.type	.L.str.59,@object       // @.str.59
.L.str.59:
	.asciz	"\0013[BLK-IO]req_list:\n"
	.size	.L.str.59, 21


	.ident	"Android (27847 based on r353983c) clang version 9.0.3 (https://android.googlesource.com/toolchain/clang dbdb7a2acf33dc76cd6f506c323253b8d892ae05) (https://android.googlesource.com/toolchain/llvm f32df87f6769ededcd9e2a6c3c4c45c1baf361c2) (based on LLVM 9.0.3svn)"
	.section	".note.GNU-stack","",@progbits

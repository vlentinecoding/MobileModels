	.text
	.file	"pagecache_manage.c"
	.globl	pch_lowmem_check        // -- Begin function pch_lowmem_check
	.p2align	2
	.type	pch_lowmem_check,@function
pch_lowmem_check:                       // @pch_lowmem_check
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	bl	first_online_pgdat
	cbz	x0, .LBB0_2
// %bb.1:
	ldr	x8, [x0, #16]
	mov	w1, wzr
	mov	w3, wzr
	mov	w4, wzr
	add	x2, x8, #2              // =2
	mov	x19, x0
	bl	zone_watermark_ok
	ldr	x2, [x19, #8]
	mvn	w8, w0
	mov	x0, x19
	mov	w1, wzr
	mov	w3, wzr
	mov	w4, wzr
	and	w20, w8, #0x1
	bl	zone_watermark_ok
	tst	w0, #0x1
	orr	w8, wzr, #0x2
	csel	w0, w20, w8, ne
.LBB0_2:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end0:
	.size	pch_lowmem_check, .Lfunc_end0-pch_lowmem_check
                                        // -- End function
	.globl	pch_lowmem_check2       // -- Begin function pch_lowmem_check2
	.p2align	2
	.type	pch_lowmem_check2,@function
pch_lowmem_check2:                      // @pch_lowmem_check2
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	adrp	x8, vm_node_stat+120
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x8, x8, :lo12:vm_node_stat+120
	ldr	x9, [x8]
	ldr	x8, [x8, #32]
	add	x29, sp, #16            // =16
	bic	x19, x9, x9, asr #63
	bic	x20, x8, x8, asr #63
	bl	total_swapcache_pages
	adrp	x9, vm_zone_stat
	add	x9, x9, :lo12:vm_zone_stat
	adrp	x10, totalreserve_pages
	ldr	x11, [x9]
	ldr	x9, [x9, #96]
	ldr	x10, [x10, :lo12:totalreserve_pages]
	sub	x8, x19, x0
	bic	x11, x11, x11, asr #63
	bic	x9, x9, x9, asr #63
	sub	x10, x11, x10
	sub	x9, x10, x9
	sub	x8, x8, x20
	cmp	x9, #1536               // =1536
	orr	x9, x9, x8
	cset	w10, lo
	cmp	x8, #1536               // =1536
	cset	w8, lo
	cmp	x9, #2048               // =2048
	and	w8, w10, w8
	cset	w10, lo
	cmp	x9, #1, lsl #12         // =4096
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	orr	w8, w8, w10
	cset	w10, lo
	cmp	x9, #4, lsl #12         // =16384
	orr	w8, w8, w10
	cset	w9, lo
	orr	w0, w8, w9
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end1:
	.size	pch_lowmem_check2, .Lfunc_end1-pch_lowmem_check2
                                        // -- End function
	.globl	pch_get_page            // -- Begin function pch_get_page
	.p2align	2
	.type	pch_get_page,@function
pch_get_page:                           // @pch_get_page
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	mov	x29, sp
	bl	pagecache_get_page
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end2:
	.size	pch_get_page, .Lfunc_end2-pch_get_page
                                        // -- End function
	.globl	pch_shrink_read_pages   // -- Begin function pch_shrink_read_pages
	.p2align	2
	.type	pch_shrink_read_pages,@function
pch_shrink_read_pages:                  // @pch_shrink_read_pages
// %bb.0:
	stp	x22, x21, [sp, #-48]!   // 16-byte Folded Spill
	adrp	x22, pch_info
	ldr	x8, [x22, :lo12:pch_info]
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	mov	x19, x0
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	cbz	x8, .LBB3_8
// %bb.1:
	ldr	w9, [x8, #200]
	cbz	w9, .LBB3_8
// %bb.2:
	ldr	w9, [x8, #212]
	cmp	x9, x19
	b.hs	.LBB3_4
// %bb.3:
	ldr	w19, [x8, #208]
.LBB3_4:
	bl	first_online_pgdat
	cbz	x0, .LBB3_8
// %bb.5:
	ldr	x8, [x0, #16]
	mov	w1, wzr
	mov	w3, wzr
	mov	w4, wzr
	add	x2, x8, #2              // =2
	mov	x20, x0
	bl	zone_watermark_ok
	ldr	x2, [x20, #8]
	mov	w21, w0
	mov	x0, x20
	mov	w1, wzr
	mov	w3, wzr
	mov	w4, wzr
	bl	zone_watermark_ok
	tbz	w21, #0, .LBB3_7
// %bb.6:
	tbnz	w0, #0, .LBB3_8
.LBB3_7:
	ldr	x8, [x22, :lo12:pch_info]
	ldr	w8, [x8, #220]
	cmp	x19, x8
	csel	x19, x8, x19, hi
.LBB3_8:
	cmp	x19, #512               // =512
	orr	w8, wzr, #0x200
	csel	x0, x19, x8, lo
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldp	x22, x21, [sp], #48     // 16-byte Folded Reload
	ret
.Lfunc_end3:
	.size	pch_shrink_read_pages, .Lfunc_end3-pch_shrink_read_pages
                                        // -- End function
	.globl	pch_shrink_mmap_pages   // -- Begin function pch_shrink_mmap_pages
	.p2align	2
	.type	pch_shrink_mmap_pages,@function
pch_shrink_mmap_pages:                  // @pch_shrink_mmap_pages
// %bb.0:
	stp	x22, x21, [sp, #-48]!   // 16-byte Folded Spill
	adrp	x22, pch_info
	ldr	x8, [x22, :lo12:pch_info]
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	mov	x19, x0
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	cbz	x8, .LBB4_6
// %bb.1:
	ldr	w9, [x8, #200]
	cbz	w9, .LBB4_6
// %bb.2:
	ldr	w8, [x8, #208]
	cmp	x8, x19
	csel	x19, x8, x19, lo
	bl	first_online_pgdat
	cbz	x0, .LBB4_6
// %bb.3:
	ldr	x8, [x0, #16]
	mov	w1, wzr
	mov	w3, wzr
	mov	w4, wzr
	add	x2, x8, #2              // =2
	mov	x20, x0
	bl	zone_watermark_ok
	ldr	x2, [x20, #8]
	mov	w21, w0
	mov	x0, x20
	mov	w1, wzr
	mov	w3, wzr
	mov	w4, wzr
	bl	zone_watermark_ok
	tbz	w21, #0, .LBB4_5
// %bb.4:
	tbnz	w0, #0, .LBB4_6
.LBB4_5:
	ldr	x8, [x22, :lo12:pch_info]
	ldr	w8, [x8, #220]
	cmp	x19, x8
	csel	x19, x8, x19, hi
.LBB4_6:
	cmp	x19, #512               // =512
	orr	w8, wzr, #0x200
	csel	x0, x19, x8, lo
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldp	x22, x21, [sp], #48     // 16-byte Folded Reload
	ret
.Lfunc_end4:
	.size	pch_shrink_mmap_pages, .Lfunc_end4-pch_shrink_mmap_pages
                                        // -- End function
	.globl	pch_shrink_mmap_async_pages // -- Begin function pch_shrink_mmap_async_pages
	.p2align	2
	.type	pch_shrink_mmap_async_pages,@function
pch_shrink_mmap_async_pages:            // @pch_shrink_mmap_async_pages
// %bb.0:
	str	x21, [sp, #-48]!        // 8-byte Folded Spill
	adrp	x8, pch_info
	ldr	x8, [x8, :lo12:pch_info]
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	mov	x19, x0
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	cbz	x8, .LBB5_6
// %bb.1:
	ldr	w8, [x8, #200]
	cbz	w8, .LBB5_6
// %bb.2:
	bl	first_online_pgdat
	cbz	x0, .LBB5_5
// %bb.3:
	ldr	x8, [x0, #16]
	mov	w1, wzr
	mov	w3, wzr
	mov	w4, wzr
	add	x2, x8, #2              // =2
	mov	x20, x0
	bl	zone_watermark_ok
	ldr	x2, [x20, #8]
	mov	w21, w0
	mov	x0, x20
	mov	w1, wzr
	mov	w3, wzr
	mov	w4, wzr
	bl	zone_watermark_ok
	mov	w8, w0
	mov	x0, xzr
	tbz	w21, #0, .LBB5_7
// %bb.4:
	tbz	w8, #0, .LBB5_7
.LBB5_5:
	mov	x0, x19
	b	.LBB5_7
.LBB5_6:
	cmp	x19, #512               // =512
	orr	w8, wzr, #0x200
	csel	x0, x19, x8, lo
.LBB5_7:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	ldr	x21, [sp], #48          // 8-byte Folded Reload
	ret
.Lfunc_end5:
	.size	pch_shrink_mmap_async_pages, .Lfunc_end5-pch_shrink_mmap_async_pages
                                        // -- End function
	.globl	pch_read_around         // -- Begin function pch_read_around
	.p2align	2
	.type	pch_read_around,@function
pch_read_around:                        // @pch_read_around
// %bb.0:
	sub	sp, sp, #208            // =208
	adrp	x8, __stack_chk_guard
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	stp	x29, x30, [sp, #192]    // 16-byte Folded Spill
	add	x29, sp, #192           // =192
	stp	x28, x27, [sp, #112]    // 16-byte Folded Spill
	stp	x26, x25, [sp, #128]    // 16-byte Folded Spill
	stp	x24, x23, [sp, #144]    // 16-byte Folded Spill
	stp	x22, x21, [sp, #160]    // 16-byte Folded Spill
	stp	x20, x19, [sp, #176]    // 16-byte Folded Spill
	stur	x8, [x29, #-88]
	mov	x21, x0
	ldr	w0, [x0, #16]
	mov	x23, x3
	mov	x22, x2
	mov	x19, x1
	bl	pch_shrink_mmap_pages
	ldr	x9, [x19]
	add	x10, sp, #72            // =72
	sub	x8, x23, x0, lsr #1
	stp	x10, x10, [sp, #72]
	add	x10, sp, #56            // =56
	bic	x8, x8, x8, asr #63
	stp	x10, x10, [sp, #56]
	str	x8, [sp, #48]           // 8-byte Folded Spill
	sub	x10, x23, x8
	ldr	x9, [x9, #80]
	ldr	w8, [x19, #208]
	cmp	x0, #127                // =127
	mov	x24, x0
	csel	x20, xzr, x10, hi
	str	x19, [sp, #24]          // 8-byte Folded Spill
	stp	xzr, xzr, [sp, #88]
	cbz	x9, .LBB6_20
// %bb.1:
	lsl	x19, x24, #1
	cbz	x19, .LBB6_20
// %bb.2:
	mov	w10, #4864
	sub	x9, x9, #1              // =1
	orr	w8, w8, w10
	str	w8, [sp, #36]           // 4-byte Folded Spill
	lsr	x8, x9, #12
	ldr	x9, [sp, #48]           // 8-byte Folded Reload
	ldr	x12, [sp, #24]          // 8-byte Folded Reload
	sub	w11, w24, #1            // =1
	stp	x22, x21, [sp]          // 16-byte Folded Spill
	sub	w8, w8, w9
	cmp	w11, w8
	mov	x22, xzr
	mov	w28, wzr
	str	x24, [sp, #16]          // 8-byte Folded Spill
	add	x12, x12, #8            // =8
	mov	w21, #-1
	csel	w24, w11, w8, lt
	str	x12, [sp, #40]          // 8-byte Folded Spill
	b	.LBB6_5
.LBB6_3:                                //   in Loop: Header=BB6_5 Depth=1
	str	x26, [x1, #16]
	ldr	x8, [sp, #56]
	ldr	x10, [sp, #16]          // 8-byte Folded Reload
	add	x9, x1, #32             // =32
	str	x9, [x8, #8]
	cmp	x10, x25
	str	x8, [x1, #32]
	add	x8, sp, #56             // =56
	str	x8, [x1, #40]
	str	x9, [sp, #56]
	b.ne	.LBB6_11
// %bb.4:                               //   in Loop: Header=BB6_5 Depth=1
	mov	w0, #19
	bl	set_bit
	b	.LBB6_11
.LBB6_5:                                // =>This Inner Loop Header: Depth=1
	tst	w22, #0x1
	csinc	w8, w21, w22, ne
	cmp	w8, #0                  // =0
	cinc	w8, w8, lt
	add	w8, w20, w8, asr #1
	cmp	w8, w24
	ccmp	w8, #0, #8, le
	csinv	w23, w8, wzr, ge
	tbnz	w23, #31, .LBB6_12
// %bb.6:                               //   in Loop: Header=BB6_5 Depth=1
	ldr	x8, [sp, #48]           // 8-byte Folded Reload
	sxtw	x25, w23
	add	x26, x8, x25
	bl	__rcu_read_lock
	ldr	x0, [sp, #40]           // 8-byte Folded Reload
	mov	x1, x26
	bl	radix_tree_lookup
	mov	x27, x0
	bl	__rcu_read_unlock
	cbz	x27, .LBB6_8
// %bb.7:                               //   in Loop: Header=BB6_5 Depth=1
	tbz	w27, #1, .LBB6_12
.LBB6_8:                                //   in Loop: Header=BB6_5 Depth=1
	ldr	w0, [sp, #36]           // 4-byte Folded Reload
	mov	w1, wzr
	mov	w2, wzr
	mov	x3, xzr
	bl	__alloc_pages_nodemask
	cbz	x0, .LBB6_13
// %bb.9:                               //   in Loop: Header=BB6_5 Depth=1
	mov	x1, x0
	cbz	x20, .LBB6_3
// %bb.10:                              //   in Loop: Header=BB6_5 Depth=1
	lsr	w8, w23, #6
	lsl	x8, x8, #3
	add	x13, sp, #88            // =88
	ldr	x9, [x13, x8]
	orr	w10, wzr, #0x1
	ldr	x11, [sp, #72]
	lsl	x10, x10, x23
	orr	x9, x9, x10
	add	x12, x1, #32            // =32
	str	x9, [x13, x8]
	add	x8, sp, #72             // =72
	str	x12, [x11, #8]
	stp	x11, x8, [x1, #32]
	str	x12, [sp, #72]
.LBB6_11:                               //   in Loop: Header=BB6_5 Depth=1
	add	w28, w28, #1            // =1
.LBB6_12:                               //   in Loop: Header=BB6_5 Depth=1
	add	x22, x22, #1            // =1
	cmp	x22, x19
	sub	w21, w21, #1            // =1
	b.lo	.LBB6_5
.LBB6_13:
	ldp	x1, x21, [sp]           // 16-byte Folded Reload
	ldr	x24, [sp, #16]          // 8-byte Folded Reload
	cbz	w28, .LBB6_20
// %bb.14:
	cbz	x20, .LBB6_19
// %bb.15:
	mov	x11, #256
	mov	x8, xzr
	add	x9, sp, #88             // =88
	orr	w10, wzr, #0x1
	movk	x11, #57005, lsl #48
	add	x12, sp, #56            // =56
	add	x13, sp, #72            // =72
.LBB6_16:                               // =>This Inner Loop Header: Depth=1
	ubfx	x14, x8, #6, #26
	ldr	x14, [x9, x14, lsl #3]
	lsl	x15, x10, x8
	tst	x15, x14
	b.eq	.LBB6_18
// %bb.17:                              //   in Loop: Header=BB6_16 Depth=1
	ldr	x14, [sp, #72]
	ldr	x17, [sp, #48]          // 8-byte Folded Reload
	ldp	x15, x16, [x14]
	add	x17, x17, x8
	str	x16, [x15, #8]
	str	x15, [x16]
	str	x11, [x14]
	stur	x17, [x14, #-16]
	ldr	x15, [sp, #56]
	str	x14, [x15, #8]
	stp	x15, x12, [x14]
	str	x14, [sp, #56]
	ldr	x14, [sp, #72]
	cmp	x13, x14
	b.eq	.LBB6_19
.LBB6_18:                               //   in Loop: Header=BB6_16 Depth=1
	add	x8, x8, #1              // =1
	cmp	x8, #128                // =128
	b.lo	.LBB6_16
.LBB6_19:
	ldr	x0, [sp, #24]           // 8-byte Folded Reload
	ldr	w4, [sp, #36]           // 4-byte Folded Reload
	add	x2, sp, #56             // =56
	mov	w3, w28
	bl	read_pages
.LBB6_20:
	ldr	x8, [sp, #48]           // 8-byte Folded Reload
	str	w24, [x21, #8]
	adrp	x9, __stack_chk_guard
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	str	x8, [x21]
	ldur	x8, [x29, #-88]
	cmp	x9, x8
	b.ne	.LBB6_22
// %bb.21:
	ldp	x29, x30, [sp, #192]    // 16-byte Folded Reload
	ldp	x20, x19, [sp, #176]    // 16-byte Folded Reload
	ldp	x22, x21, [sp, #160]    // 16-byte Folded Reload
	ldp	x24, x23, [sp, #144]    // 16-byte Folded Reload
	ldp	x26, x25, [sp, #128]    // 16-byte Folded Reload
	ldp	x28, x27, [sp, #112]    // 16-byte Folded Reload
	add	sp, sp, #208            // =208
	ret
.LBB6_22:
	bl	__stack_chk_fail
.Lfunc_end6:
	.size	pch_read_around, .Lfunc_end6-pch_read_around
                                        // -- End function
	.globl	pch_mmap_readextend     // -- Begin function pch_mmap_readextend
	.p2align	2
	.type	pch_mmap_readextend,@function
pch_mmap_readextend:                    // @pch_mmap_readextend
// %bb.0:
	str	x25, [sp, #-80]!        // 8-byte Folded Spill
	adrp	x25, pch_info
	ldr	x8, [x25, :lo12:pch_info]
	stp	x24, x23, [sp, #16]     // 16-byte Folded Spill
	stp	x22, x21, [sp, #32]     // 16-byte Folded Spill
	stp	x20, x19, [sp, #48]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #64]     // 16-byte Folded Spill
	add	x29, sp, #64            // =64
	cbz	x8, .LBB7_13
// %bb.1:
	ldr	w8, [x8, #200]
	cbz	w8, .LBB7_13
// %bb.2:
	mov	x20, x3
	mov	x19, x2
	mov	x21, x1
	mov	x22, x0
	bl	first_online_pgdat
	cbz	x0, .LBB7_5
// %bb.3:
	ldr	x8, [x0, #16]
	mov	w1, wzr
	mov	w3, wzr
	mov	w4, wzr
	add	x2, x8, #2              // =2
	mov	x23, x0
	bl	zone_watermark_ok
	ldr	x2, [x23, #8]
	mov	w24, w0
	mov	x0, x23
	mov	w1, wzr
	mov	w3, wzr
	mov	w4, wzr
	bl	zone_watermark_ok
	tbz	w24, #0, .LBB7_13
// %bb.4:
	tbz	w0, #0, .LBB7_13
.LBB7_5:
	ldr	x23, [x19, #320]
	ldr	x0, [x23]
	ldr	x24, [x0, #80]
	cmp	x24, #1                 // =1
	b.lt	.LBB7_13
// %bb.6:
	tbnz	w22, #16, .LBB7_13
// %bb.7:
	tbnz	w22, #15, .LBB7_13
// %bb.8:
	ldr	w8, [x21, #16]
	cbz	w8, .LBB7_13
// %bb.9:
	ldr	w8, [x21, #24]
	cmp	w8, #100                // =100
	b.hi	.LBB7_13
// %bb.10:
	orr	w1, wzr, #0x2
	bl	inode_congested
	cbnz	w0, .LBB7_13
// %bb.11:
	ldr	w8, [x21, #8]
	sub	w9, w24, #1             // =1
	lsr	w9, w9, #12
	add	x2, x20, x8, lsr #1
	cmp	x9, x2
	b.le	.LBB7_13
// %bb.12:
	ldr	x8, [x25, :lo12:pch_info]
	mov	x0, x23
	mov	x1, x19
	mov	x4, xzr
	ldr	w3, [x8, #208]
	bl	__do_page_cache_readahead
.LBB7_13:
	ldp	x29, x30, [sp, #64]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #48]     // 16-byte Folded Reload
	ldp	x22, x21, [sp, #32]     // 16-byte Folded Reload
	ldp	x24, x23, [sp, #16]     // 16-byte Folded Reload
	ldr	x25, [sp], #80          // 8-byte Folded Reload
	ret
.Lfunc_end7:
	.size	pch_mmap_readextend, .Lfunc_end7-pch_mmap_readextend
                                        // -- End function
	.globl	pch_timeout_work        // -- Begin function pch_timeout_work
	.p2align	2
	.type	pch_timeout_work,@function
pch_timeout_work:                       // @pch_timeout_work
// %bb.0:
	sub	sp, sp, #192            // =192
	adrp	x8, __stack_chk_guard
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	stp	x29, x30, [sp, #176]    // 16-byte Folded Spill
	add	x29, sp, #176           // =176
	str	x21, [sp, #144]         // 8-byte Folded Spill
	stp	x20, x19, [sp, #160]    // 16-byte Folded Spill
	stur	x8, [x29, #-40]
	adrp	x20, pch_info
	ldr	x8, [x20, :lo12:pch_info]
	add	x0, x8, #144            // =144
	bl	down
	ldr	x8, [x20, :lo12:pch_info]
	orr	w0, wzr, #0x1
	add	x1, x8, #192            // =192
	bl	set_bit
	ldr	x8, [x20, :lo12:pch_info]
	ldrb	w9, [x8, #228]
	tbz	w9, #0, .LBB8_16
// %bb.1:
	ldr	x8, [x8, #56]
	adrp	x0, .L.str.20
	add	x0, x0, :lo12:.L.str.20
	mov	x2, sp
	ldr	x8, [x8, #16]
	mov	w1, #5
	ldr	x21, [x8, #248]
	stp	xzr, xzr, [sp, #120]
	stp	xzr, xzr, [sp, #104]
	stp	xzr, xzr, [sp, #88]
	stp	xzr, xzr, [sp, #72]
	stp	xzr, xzr, [sp, #56]
	stp	xzr, xzr, [sp, #40]
	stp	xzr, xzr, [sp, #24]
	str	xzr, [sp, #16]
	bl	kern_path
	mov	w19, w0
	cbnz	w0, .LBB8_3
// %bb.2:
	mov	x0, sp
	add	x1, sp, #16             // =16
	bl	vfs_statfs
	mov	w19, w0
	mov	x0, sp
	bl	path_put
	cbz	w19, .LBB8_7
.LBB8_3:
	cbz	w19, .LBB8_8
.LBB8_4:
	cmp	w19, #1                 // =1
	b.ne	.LBB8_6
// %bb.5:
	ldr	x8, [x20, :lo12:pch_info]
	mov	x9, #17179869188
	orr	w10, wzr, #0x10
	str	x9, [x8, #208]
	str	w10, [x8, #216]
	b	.LBB8_13
.LBB8_6:
	adrp	x0, .L.str.1
	adrp	x1, .L__func__.pch_timeout_work
	add	x0, x0, :lo12:.L.str.1
	add	x1, x1, :lo12:.L__func__.pch_timeout_work
	mov	w2, w19
	bl	printk
	b	.LBB8_13
.LBB8_7:
	ldr	x8, [x20, :lo12:pch_info]
	ldr	x9, [sp, #48]
	ldr	x10, [sp, #32]
	mov	w11, #100
	ldr	w8, [x8, #224]
	mul	x11, x9, x11
	udiv	x10, x11, x10
	cmp	w8, w10
	cset	w8, hs
	cmp	x9, #64, lsl #12        // =262144
	cset	w9, ls
	and	w19, w9, w8
	cbnz	w19, .LBB8_4
.LBB8_8:
	ldr	x8, [x20, :lo12:pch_info]
	orr	w10, wzr, #0x10
	ldr	w9, [x8, #204]
	str	w10, [x8, #216]
	cmp	w9, #1                  // =1
	b.eq	.LBB8_11
// %bb.9:
	cbnz	w9, .LBB8_13
// %bb.10:
	mov	x9, #34359738376
	b	.LBB8_12
.LBB8_11:
	mov	x9, #68719476752
.LBB8_12:
	str	x9, [x8, #208]
.LBB8_13:
	add	x0, sp, #16             // =16
	mov	x1, sp
	bl	global_dirty_limits
	adrp	x8, vm_node_stat+128
	add	x8, x8, :lo12:vm_node_stat+128
	ldr	x9, [x8]
	ldr	x10, [x8, #56]
	ldr	x8, [x8, #8]
	ldr	x11, [sp]
	bic	x9, x9, x9, asr #63
	bic	x10, x10, x10, asr #63
	add	x9, x10, x9
	bic	x8, x8, x8, asr #63
	add	x8, x9, x8
	cmp	x8, x11
	b.lo	.LBB8_15
// %bb.14:
	ldr	x8, [x21, #104]
	tbz	w8, #1, .LBB8_18
.LBB8_15:
	ldr	x8, [x20, :lo12:pch_info]
.LBB8_16:
	add	x1, x8, #192            // =192
	orr	w0, wzr, #0x1
	bl	clear_bit
	ldr	x8, [x20, :lo12:pch_info]
	add	x0, x8, #144            // =144
	bl	up
	adrp	x9, __stack_chk_guard
	ldur	x8, [x29, #-40]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB8_19
// %bb.17:
	ldp	x29, x30, [sp, #176]    // 16-byte Folded Reload
	ldp	x20, x19, [sp, #160]    // 16-byte Folded Reload
	ldr	x21, [sp, #144]         // 8-byte Folded Reload
	add	sp, sp, #192            // =192
	ret
.LBB8_18:
	add	x0, x21, #96            // =96
	bl	wb_start_background_writeback
	b	.LBB8_15
.LBB8_19:
	bl	__stack_chk_fail
.Lfunc_end8:
	.size	pch_timeout_work, .Lfunc_end8-pch_timeout_work
                                        // -- End function
	.globl	pch_timeout_timer       // -- Begin function pch_timeout_timer
	.p2align	2
	.type	pch_timeout_timer,@function
pch_timeout_timer:                      // @pch_timeout_timer
// %bb.0:
	adrp	x8, pch_info
	ldr	x8, [x8, :lo12:pch_info]
	ldr	x9, [x8, #192]
	tbnz	w9, #1, .LBB9_2
// %bb.1:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	adrp	x9, system_wq
	ldr	x1, [x9, :lo12:system_wq]
	add	x2, x8, #112            // =112
	orr	w0, wzr, #0x8
	mov	x29, sp
	bl	queue_work_on
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
.LBB9_2:
	ret
.Lfunc_end9:
	.size	pch_timeout_timer, .Lfunc_end9-pch_timeout_timer
                                        // -- End function
	.globl	mount_fs_register_pch   // -- Begin function mount_fs_register_pch
	.p2align	2
	.type	mount_fs_register_pch,@function
mount_fs_register_pch:                  // @mount_fs_register_pch
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	adrp	x8, pch_info
	ldr	x1, [x8, :lo12:pch_info]
	mov	x29, sp
	cbz	x1, .LBB10_5
// %bb.1:
	ldr	x8, [x0, #8]
	mov	w10, #8208
	movk	w10, #62197, lsl #16
	ldr	x9, [x8, #96]
	cmp	x9, x10
	b.ne	.LBB10_7
// %bb.2:
	ldr	w9, [x1, #228]
	tbnz	w9, #2, .LBB10_11
// %bb.3:
	adrp	x8, g_pch_print_level
	ldr	w8, [x8, :lo12:g_pch_print_level]
	cmp	w8, #2                  // =2
	b.lo	.LBB10_10
// %bb.4:
	adrp	x0, .L.str.3
	add	x0, x0, :lo12:.L.str.3
	b	.LBB10_9
.LBB10_5:
	adrp	x8, g_pch_print_level
	ldr	w8, [x8, :lo12:g_pch_print_level]
	cbz	w8, .LBB10_10
// %bb.6:
	adrp	x0, .L.str.2
	add	x0, x0, :lo12:.L.str.2
	b	.LBB10_9
.LBB10_7:
	adrp	x8, g_pch_print_level
	ldr	w8, [x8, :lo12:g_pch_print_level]
	cmp	w8, #2                  // =2
	b.lo	.LBB10_10
// %bb.8:
	adrp	x0, .L.str.4
	add	x0, x0, :lo12:.L.str.4
.LBB10_9:
	adrp	x1, .L__func__.mount_fs_register_pch
	add	x1, x1, :lo12:.L__func__.mount_fs_register_pch
	bl	printk
.LBB10_10:
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.LBB10_11:
	ldr	x10, [x1, #56]
	and	w9, w9, #0xfffffffb
	orr	w9, w9, #0x1
	str	w9, [x1, #228]
	cbnz	x10, .LBB10_10
// %bb.12:
	ldr	x0, [x8, #240]
	cbz	x0, .LBB10_10
// %bb.13:
	adrp	x8, .L.str.5
	add	x8, x8, :lo12:.L.str.5
	ldp	x9, x8, [x8]
	adrp	x10, pch_io_idle_notify_handler
	add	x10, x10, :lo12:pch_io_idle_notify_handler
	str	x0, [x1, #56]
	str	x10, [x1, #32]
	stp	x9, x8, [x1]
	bl	blk_busyidle_event_subscribe
	cbz	w0, .LBB10_10
// %bb.14:
	adrp	x0, .L.str.6
	add	x0, x0, :lo12:.L.str.6
	b	.LBB10_9
.Lfunc_end10:
	.size	mount_fs_register_pch, .Lfunc_end10-mount_fs_register_pch
                                        // -- End function
	.p2align	2               // -- Begin function pch_io_idle_notify_handler
	.type	pch_io_idle_notify_handler,@function
pch_io_idle_notify_handler:             // @pch_io_idle_notify_handler
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	mov	x29, sp
	cbz	w1, .LBB11_3
// %bb.1:
	cmp	w1, #1                  // =1
	b.ne	.LBB11_4
// %bb.2:
	adrp	x8, pch_info
	adrp	x9, jiffies
	ldr	x8, [x8, :lo12:pch_info]
	ldr	x9, [x9, :lo12:jiffies]
	add	x0, x8, #64             // =64
	add	x1, x9, #1250           // =1250
	bl	mod_timer
	b	.LBB11_4
.LBB11_3:
	adrp	x8, pch_info
	ldr	x8, [x8, :lo12:pch_info]
	add	x0, x8, #64             // =64
	bl	del_timer
.LBB11_4:
	mov	w0, wzr
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end11:
	.size	pch_io_idle_notify_handler, .Lfunc_end11-pch_io_idle_notify_handler
                                        // -- End function
	.globl	umounting_fs_register_pch // -- Begin function umounting_fs_register_pch
	.p2align	2
	.type	umounting_fs_register_pch,@function
umounting_fs_register_pch:              // @umounting_fs_register_pch
// %bb.0:
	str	x19, [sp, #-32]!        // 8-byte Folded Spill
	adrp	x19, pch_info
	ldr	x8, [x19, :lo12:pch_info]
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	cbz	x8, .LBB12_5
// %bb.1:
	ldr	x9, [x0, #96]
	mov	w10, #8208
	movk	w10, #62197, lsl #16
	cmp	x9, x10
	b.ne	.LBB12_7
// %bb.2:
	ldr	w9, [x8, #228]
	tbnz	w9, #0, .LBB12_11
// %bb.3:
	adrp	x8, g_pch_print_level
	ldr	w8, [x8, :lo12:g_pch_print_level]
	cmp	w8, #2                  // =2
	b.lo	.LBB12_10
// %bb.4:
	adrp	x0, .L.str.8
	add	x0, x0, :lo12:.L.str.8
	b	.LBB12_9
.LBB12_5:
	adrp	x8, g_pch_print_level
	ldr	w8, [x8, :lo12:g_pch_print_level]
	cbz	w8, .LBB12_10
// %bb.6:
	adrp	x0, .L.str.2
	add	x0, x0, :lo12:.L.str.2
	b	.LBB12_9
.LBB12_7:
	adrp	x8, g_pch_print_level
	ldr	w8, [x8, :lo12:g_pch_print_level]
	cmp	w8, #2                  // =2
	b.lo	.LBB12_10
// %bb.8:
	adrp	x0, .L.str.4
	add	x0, x0, :lo12:.L.str.4
.LBB12_9:
	adrp	x1, .L__func__.umounting_fs_register_pch
	add	x1, x1, :lo12:.L__func__.umounting_fs_register_pch
	bl	printk
.LBB12_10:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldr	x19, [sp], #32          // 8-byte Folded Reload
	ret
.LBB12_11:
	ldr	x10, [x8, #56]
	and	w9, w9, #0xfffffffc
	orr	w9, w9, #0x2
	str	w9, [x8, #228]
	cbz	x10, .LBB12_10
// %bb.12:
	add	x0, x8, #64             // =64
	bl	del_timer_sync
	ldr	x8, [x19, :lo12:pch_info]
	add	x0, x8, #112            // =112
	bl	flush_work
	ldr	x8, [x19, :lo12:pch_info]
	add	x0, x8, #144            // =144
	bl	down
	ldr	x0, [x19, :lo12:pch_info]
	bl	blk_busyidle_event_unsubscribe
	cbz	w0, .LBB12_14
// %bb.13:
	adrp	x0, .L.str.7
	adrp	x1, .L__func__.umounting_fs_register_pch
	add	x0, x0, :lo12:.L.str.7
	add	x1, x1, :lo12:.L__func__.umounting_fs_register_pch
	bl	printk
.LBB12_14:
	ldr	x8, [x19, :lo12:pch_info]
	add	x0, x8, #144            // =144
	bl	up
	b	.LBB12_10
.Lfunc_end12:
	.size	umounting_fs_register_pch, .Lfunc_end12-umounting_fs_register_pch
                                        // -- End function
	.globl	umounted_fs_register_pch // -- Begin function umounted_fs_register_pch
	.p2align	2
	.type	umounted_fs_register_pch,@function
umounted_fs_register_pch:               // @umounted_fs_register_pch
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	ldr	x8, [x0, #96]
	mov	w9, #8208
	movk	w9, #62197, lsl #16
	mov	x29, sp
	cmp	x8, x9
	b.ne	.LBB13_4
// %bb.1:
	adrp	x8, pch_info
	ldr	x8, [x8, :lo12:pch_info]
	ldr	w9, [x8, #228]
	tbnz	w9, #1, .LBB13_8
// %bb.2:
	adrp	x8, g_pch_print_level
	ldr	w8, [x8, :lo12:g_pch_print_level]
	cmp	w8, #2                  // =2
	b.lo	.LBB13_7
// %bb.3:
	adrp	x0, .L.str.3
	add	x0, x0, :lo12:.L.str.3
	b	.LBB13_6
.LBB13_4:
	adrp	x8, g_pch_print_level
	ldr	w8, [x8, :lo12:g_pch_print_level]
	cmp	w8, #2                  // =2
	b.lo	.LBB13_7
// %bb.5:
	adrp	x0, .L.str.4
	add	x0, x0, :lo12:.L.str.4
.LBB13_6:
	adrp	x1, .L__func__.umounted_fs_register_pch
	add	x1, x1, :lo12:.L__func__.umounted_fs_register_pch
	bl	printk
.LBB13_7:
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.LBB13_8:
	and	w9, w9, #0xfffffff9
	orr	w9, w9, #0x4
	str	w9, [x8, #228]
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end13:
	.size	umounted_fs_register_pch, .Lfunc_end13-umounted_fs_register_pch
                                        // -- End function
	.globl	pch_emui_bootstat_proc_show // -- Begin function pch_emui_bootstat_proc_show
	.p2align	2
	.type	pch_emui_bootstat_proc_show,@function
pch_emui_bootstat_proc_show:            // @pch_emui_bootstat_proc_show
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	adrp	x8, pch_info
	ldr	x8, [x8, :lo12:pch_info]
	adrp	x1, .L.str.9
	add	x1, x1, :lo12:.L.str.9
	mov	x29, sp
	ldr	w2, [x8, #200]
	bl	seq_printf
	mov	w0, wzr
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end14:
	.size	pch_emui_bootstat_proc_show, .Lfunc_end14-pch_emui_bootstat_proc_show
                                        // -- End function
	.globl	pch_emui_bootstat_proc_open // -- Begin function pch_emui_bootstat_proc_open
	.p2align	2
	.type	pch_emui_bootstat_proc_open,@function
pch_emui_bootstat_proc_open:            // @pch_emui_bootstat_proc_open
// %bb.0:
	stp	x29, x30, [sp, #-16]!   // 16-byte Folded Spill
	mov	x0, x1
	adrp	x1, __cfi_pch_emui_bootstat_proc_show
	add	x1, x1, :lo12:__cfi_pch_emui_bootstat_proc_show
	mov	x2, xzr
	mov	x29, sp
	bl	single_open
	ldp	x29, x30, [sp], #16     // 16-byte Folded Reload
	ret
.Lfunc_end15:
	.size	pch_emui_bootstat_proc_open, .Lfunc_end15-pch_emui_bootstat_proc_open
                                        // -- End function
	.globl	pch_emui_bootstat_proc_write // -- Begin function pch_emui_bootstat_proc_write
	.p2align	2
	.type	pch_emui_bootstat_proc_write,@function
pch_emui_bootstat_proc_write:           // @pch_emui_bootstat_proc_write
// %bb.0:
	sub	sp, sp, #48             // =48
	adrp	x8, __stack_chk_guard
	ldr	x8, [x8, :lo12:__stack_chk_guard]
	mov	x0, #-1
	stp	x20, x19, [sp, #16]     // 16-byte Folded Spill
	stp	x29, x30, [sp, #32]     // 16-byte Folded Spill
	add	x29, sp, #32            // =32
	str	x8, [sp, #8]
	cbz	x1, .LBB16_9
// %bb.1:
	mov	x19, x2
	cbz	x2, .LBB16_9
// %bb.2:
	adrp	x20, pch_info
	ldr	x8, [x20, :lo12:pch_info]
	ldr	w8, [x8, #200]
	cmp	w8, #1                  // =1
	b.eq	.LBB16_9
// %bb.3:
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
	cbz	x10, .LBB16_11
// %bb.4:
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
	cbnz	x0, .LBB16_12
// %bb.5:
	ldrb	w8, [sp, #4]
	cmp	w8, #49                 // =49
	b.ne	.LBB16_7
// %bb.6:
	ldr	x8, [x20, :lo12:pch_info]
	orr	w9, wzr, #0x1
	mov	x0, x19
	str	w9, [x8, #200]
	b	.LBB16_9
.LBB16_7:
	adrp	x0, .L.str.10
	adrp	x1, .L__func__.pch_emui_bootstat_proc_write
	add	x0, x0, :lo12:.L.str.10
	add	x1, x1, :lo12:.L__func__.pch_emui_bootstat_proc_write
	bl	printk
.LBB16_8:
	mov	x0, #-1
.LBB16_9:
	adrp	x9, __stack_chk_guard
	ldr	x8, [sp, #8]
	ldr	x9, [x9, :lo12:__stack_chk_guard]
	cmp	x9, x8
	b.ne	.LBB16_14
// %bb.10:
	ldp	x29, x30, [sp, #32]     // 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]     // 16-byte Folded Reload
	add	sp, sp, #48             // =48
	ret
.LBB16_11:
	orr	w2, wzr, #0x1
	b	.LBB16_13
.LBB16_12:
	mov	x2, x0
.LBB16_13:
	orr	w8, wzr, #0x1
	sub	x8, x8, x2
	add	x9, sp, #4              // =4
	add	x0, x9, x8
	mov	w1, wzr
	bl	memset
	b	.LBB16_8
.LBB16_14:
	bl	__stack_chk_fail
.Lfunc_end16:
	.size	pch_emui_bootstat_proc_write, .Lfunc_end16-pch_emui_bootstat_proc_write
                                        // -- End function
	.section	.init.text,"ax",@progbits
	.globl	hisi_page_cache_manage_init // -- Begin function hisi_page_cache_manage_init
	.p2align	2
	.type	hisi_page_cache_manage_init,@function
hisi_page_cache_manage_init:            // @hisi_page_cache_manage_init
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	adrp	x19, g_pch_print_level
	ldr	w8, [x19, :lo12:g_pch_print_level]
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	cmp	w8, #2                  // =2
	b.lo	.LBB17_2
// %bb.1:
	adrp	x0, .L.str.11
	adrp	x1, .L__func__.hisi_page_cache_manage_init
	add	x0, x0, :lo12:.L.str.11
	add	x1, x1, :lo12:.L__func__.hisi_page_cache_manage_init
	bl	printk
.LBB17_2:
	adrp	x8, kmalloc_caches+64
	ldr	x0, [x8, :lo12:kmalloc_caches+64]
	mov	w1, #32960
	movk	w1, #320, lsl #16
	mov	w2, #232
	bl	kmem_cache_alloc_trace
	adrp	x20, pch_info
	str	x0, [x20, :lo12:pch_info]
	cbz	x0, .LBB17_6
// %bb.3:
	mov	x9, #16
	orr	w8, wzr, #0x1
	movk	x9, #8, lsl #32
	str	w8, [x0, #200]
	stp	x9, x8, [x0, #216]
	bl	get_bootdevice_type
	cbz	w0, .LBB17_7
// %bb.4:
	cmp	w0, #1                  // =1
	b.ne	.LBB17_10
// %bb.5:
	orr	w8, wzr, #0x10
	b	.LBB17_8
.LBB17_6:
	mov	w0, #-12
	b	.LBB17_14
.LBB17_7:
	orr	w8, wzr, #0x8
.LBB17_8:
	ldr	x9, [x20, :lo12:pch_info]
	mov	x1, xzr
	str	w0, [x9, #204]
	adrp	x0, .L.str.16
	add	x0, x0, :lo12:.L.str.16
	stp	w8, w8, [x9, #208]
	bl	proc_mkdir
	cbz	x0, .LBB17_11
// %bb.9:
	adrp	x0, .L.str.15
	adrp	x3, pch_emui_boot_done_fops
	add	x0, x0, :lo12:.L.str.15
	add	x3, x3, :lo12:pch_emui_boot_done_fops
	mov	w1, #432
	mov	x2, xzr
	bl	proc_create
	b	.LBB17_12
.LBB17_10:
	adrp	x0, .L.str.12
	adrp	x1, .L__func__.hisi_page_cache_manage_init
	add	x0, x0, :lo12:.L.str.12
	add	x1, x1, :lo12:.L__func__.hisi_page_cache_manage_init
	bl	printk
	ldr	x0, [x20, :lo12:pch_info]
	bl	kfree
	str	xzr, [x20, :lo12:pch_info]
	mov	w0, #-1
	b	.LBB17_14
.LBB17_11:
	adrp	x0, .L.str.22
	adrp	x1, .L__func__.pch_emui_boot_check_init
	add	x0, x0, :lo12:.L.str.22
	add	x1, x1, :lo12:.L__func__.pch_emui_boot_check_init
	bl	printk
.LBB17_12:
	ldr	x8, [x20, :lo12:pch_info]
	mov	w1, wzr
	mov	x2, xzr
	mov	x3, xzr
	add	x0, x8, #64             // =64
	bl	init_timer_key
	ldr	x8, [x20, :lo12:pch_info]
	adrp	x9, __cfi_pch_timeout_timer
	add	x9, x9, :lo12:__cfi_pch_timeout_timer
	mov	x10, #68719476704
	stp	x9, x8, [x8, #88]
	add	x9, x8, #120            // =120
	str	x10, [x8, #112]
	str	x9, [x8, #120]
	orr	w10, wzr, #0x1
	adrp	x11, __cfi_pch_timeout_work
	str	w10, [x8, #168]
	ldr	w10, [x8, #228]
	add	x11, x11, :lo12:__cfi_pch_timeout_work
	stp	x9, x11, [x8, #128]
	mov	x9, #-1
	mov	x12, #-45395
	str	x9, [x8, #160]
	add	x9, x8, #176            // =176
	movk	x12, #57005, lsl #16
	stp	x9, x9, [x8, #176]
	orr	w9, w10, #0x4
	stur	x12, [x8, #148]
	str	wzr, [x8, #144]
	str	w9, [x8, #228]
	ldr	w8, [x19, :lo12:g_pch_print_level]
	mov	w0, wzr
	cmp	w8, #2                  // =2
	b.lo	.LBB17_14
// %bb.13:
	adrp	x0, .L.str.13
	adrp	x1, .L__func__.hisi_page_cache_manage_init
	add	x0, x0, :lo12:.L.str.13
	add	x1, x1, :lo12:.L__func__.hisi_page_cache_manage_init
	bl	printk
	mov	w0, wzr
.LBB17_14:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end17:
	.size	hisi_page_cache_manage_init, .Lfunc_end17-hisi_page_cache_manage_init
                                        // -- End function
	.section	.exit.text,"ax",@progbits
	.globl	hisi_page_cache_manage_exit // -- Begin function hisi_page_cache_manage_exit
	.p2align	2
	.type	hisi_page_cache_manage_exit,@function
hisi_page_cache_manage_exit:            // @hisi_page_cache_manage_exit
// %bb.0:
	stp	x20, x19, [sp, #-32]!   // 16-byte Folded Spill
	adrp	x19, g_pch_print_level
	ldr	w8, [x19, :lo12:g_pch_print_level]
	stp	x29, x30, [sp, #16]     // 16-byte Folded Spill
	add	x29, sp, #16            // =16
	cmp	w8, #2                  // =2
	b.lo	.LBB18_2
// %bb.1:
	adrp	x0, .L.str.14
	adrp	x1, .L__func__.hisi_page_cache_manage_exit
	add	x0, x0, :lo12:.L.str.14
	add	x1, x1, :lo12:.L__func__.hisi_page_cache_manage_exit
	bl	printk
.LBB18_2:
	adrp	x0, .L.str.15
	add	x0, x0, :lo12:.L.str.15
	mov	x1, xzr
	bl	remove_proc_entry
	adrp	x0, .L.str.16
	add	x0, x0, :lo12:.L.str.16
	mov	x1, xzr
	bl	remove_proc_entry
	adrp	x20, pch_info
	ldr	x0, [x20, :lo12:pch_info]
	bl	kfree
	ldr	w8, [x19, :lo12:g_pch_print_level]
	str	xzr, [x20, :lo12:pch_info]
	cmp	w8, #2                  // =2
	b.lo	.LBB18_4
// %bb.3:
	adrp	x0, .L.str.17
	adrp	x1, .L__func__.hisi_page_cache_manage_exit
	add	x0, x0, :lo12:.L.str.17
	add	x1, x1, :lo12:.L__func__.hisi_page_cache_manage_exit
	bl	printk
.LBB18_4:
	ldp	x29, x30, [sp, #16]     // 16-byte Folded Reload
	ldp	x20, x19, [sp], #32     // 16-byte Folded Reload
	ret
.Lfunc_end18:
	.size	hisi_page_cache_manage_exit, .Lfunc_end18-hisi_page_cache_manage_exit
                                        // -- End function
	.type	g_pch_print_level,@object // @g_pch_print_level
	.bss
	.globl	g_pch_print_level
	.p2align	2
g_pch_print_level:
	.word	0                       // 0x0
	.size	g_pch_print_level, 4

	.type	pch_info,@object        // @pch_info
	.local	pch_info
	.comm	pch_info,8,8
	.type	.L.str.1,@object        // @.str.1
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.1:
	.asciz	"\0013%s: err userdata_state = %d.\n"
	.size	.L.str.1, 32

	.type	.L__func__.pch_timeout_work,@object // @__func__.pch_timeout_work
.L__func__.pch_timeout_work:
	.asciz	"pch_timeout_work"
	.size	.L__func__.pch_timeout_work, 17

	.type	.L.str.2,@object        // @.str.2
.L.str.2:
	.asciz	"\0013%s: lack of pch_info, return.\n"
	.size	.L.str.2, 33

	.type	.L__func__.mount_fs_register_pch,@object // @__func__.mount_fs_register_pch
.L__func__.mount_fs_register_pch:
	.asciz	"mount_fs_register_pch"
	.size	.L__func__.mount_fs_register_pch, 22

	.type	.L.str.3,@object        // @.str.3
.L.str.3:
	.asciz	"\0013%s: data remount, return.\n"
	.size	.L.str.3, 29

	.type	.L.str.4,@object        // @.str.4
.L.str.4:
	.asciz	"\0013%s: other partitions, return.\n"
	.size	.L.str.4, 33

	.type	.L.str.5,@object        // @.str.5
.L.str.5:
	.asciz	"pagecache_helper"
	.size	.L.str.5, 17

	.type	.L.str.6,@object        // @.str.6
.L.str.6:
	.asciz	"\0013%s: blk busy/idle init failed.\n"
	.size	.L.str.6, 34

	.type	.L__func__.umounting_fs_register_pch,@object // @__func__.umounting_fs_register_pch
.L__func__.umounting_fs_register_pch:
	.asciz	"umounting_fs_register_pch"
	.size	.L__func__.umounting_fs_register_pch, 26

	.type	.L.str.7,@object        // @.str.7
.L.str.7:
	.asciz	"\0013%s: blk busy/idle clear failed.\n"
	.size	.L.str.7, 35

	.type	.L.str.8,@object        // @.str.8
.L.str.8:
	.asciz	"\0013%s: data re-umount, return.\n"
	.size	.L.str.8, 31

	.type	.L__func__.umounted_fs_register_pch,@object // @__func__.umounted_fs_register_pch
.L__func__.umounted_fs_register_pch:
	.asciz	"umounted_fs_register_pch"
	.size	.L__func__.umounted_fs_register_pch, 25

	.type	.L.str.9,@object        // @.str.9
.L.str.9:
	.asciz	"%d\n"
	.size	.L.str.9, 4

	.type	.L.str.10,@object       // @.str.10
.L.str.10:
	.asciz	"\0013%s: input error.\n"
	.size	.L.str.10, 20

	.type	.L__func__.pch_emui_bootstat_proc_write,@object // @__func__.pch_emui_bootstat_proc_write
.L__func__.pch_emui_bootstat_proc_write:
	.asciz	"pch_emui_bootstat_proc_write"
	.size	.L__func__.pch_emui_bootstat_proc_write, 29

	.type	.L.str.11,@object       // @.str.11
.L.str.11:
	.asciz	"\0013%s: module init ++.\n"
	.size	.L.str.11, 23

	.type	.L__func__.hisi_page_cache_manage_init,@object // @__func__.hisi_page_cache_manage_init
.L__func__.hisi_page_cache_manage_init:
	.asciz	"hisi_page_cache_manage_init"
	.size	.L__func__.hisi_page_cache_manage_init, 28

	.type	.L.str.12,@object       // @.str.12
.L.str.12:
	.asciz	"\0013%s: device not supported.\n"
	.size	.L.str.12, 29

	.type	.L.str.13,@object       // @.str.13
.L.str.13:
	.asciz	"\0013%s: module init --.\n"
	.size	.L.str.13, 23

	.type	.L.str.14,@object       // @.str.14
.L.str.14:
	.asciz	"\0013%s: module exit ++.\n"
	.size	.L.str.14, 23

	.type	.L__func__.hisi_page_cache_manage_exit,@object // @__func__.hisi_page_cache_manage_exit
.L__func__.hisi_page_cache_manage_exit:
	.asciz	"hisi_page_cache_manage_exit"
	.size	.L__func__.hisi_page_cache_manage_exit, 28

	.type	.L.str.15,@object       // @.str.15
.L.str.15:
	.asciz	"pch_status/pch_enabled"
	.size	.L.str.15, 23

	.type	.L.str.16,@object       // @.str.16
.L.str.16:
	.asciz	"pch_status"
	.size	.L.str.16, 11

	.type	.L.str.17,@object       // @.str.17
.L.str.17:
	.asciz	"\0013%s: module exit --.\n"
	.size	.L.str.17, 23

	.type	__initcall___cfi_hisi_page_cache_manage_init6,@object // @__initcall___cfi_hisi_page_cache_manage_init6
	.section	.initcall6.init,"aw",@progbits
	.p2align	3
__initcall___cfi_hisi_page_cache_manage_init6:
	.xword	__cfi_hisi_page_cache_manage_init
	.size	__initcall___cfi_hisi_page_cache_manage_init6, 8

	.type	__exitcall___cfi_hisi_page_cache_manage_exit,@object // @__exitcall___cfi_hisi_page_cache_manage_exit
	.section	.exitcall.exit,"aw",@progbits
	.p2align	3
__exitcall___cfi_hisi_page_cache_manage_exit:
	.xword	__cfi_hisi_page_cache_manage_exit
	.size	__exitcall___cfi_hisi_page_cache_manage_exit, 8

	.type	.L.str.20,@object       // @.str.20
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.20:
	.asciz	"/data"
	.size	.L.str.20, 6

	.type	.L.str.22,@object       // @.str.22
.L.str.22:
	.asciz	"\0013%s: proc mk error.\n"
	.size	.L.str.22, 22

	.type	.L__func__.pch_emui_boot_check_init,@object // @__func__.pch_emui_boot_check_init
.L__func__.pch_emui_boot_check_init:
	.asciz	"pch_emui_boot_check_init"
	.size	.L__func__.pch_emui_boot_check_init, 25

	.type	pch_emui_boot_done_fops,@object // @pch_emui_boot_done_fops
	.section	.rodata,"a",@progbits
	.p2align	3
pch_emui_boot_done_fops:
	.xword	0
	.xword	seq_lseek
	.xword	seq_read
	.xword	__cfi_pch_emui_bootstat_proc_write
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	0
	.xword	__cfi_pch_emui_bootstat_proc_open
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
	.size	pch_emui_boot_done_fops, 240


	.ident	"Android (27847 based on r353983c) clang version 9.0.3 (https://android.googlesource.com/toolchain/clang dbdb7a2acf33dc76cd6f506c323253b8d892ae05) (https://android.googlesource.com/toolchain/llvm f32df87f6769ededcd9e2a6c3c4c45c1baf361c2) (based on LLVM 9.0.3svn)"
	.section	".note.GNU-stack","",@progbits

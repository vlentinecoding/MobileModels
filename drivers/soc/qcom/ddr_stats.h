

#ifndef DDR_STATS_H
#define DDR_STATS_H

/**
 * struct stats_entry - DDR info.
 * @name:		ddr frequency.
 * @count:		scheduled cnt of the freq.
 * @duration:	run time in ms of the freq.
 */
struct stats_entry {
	uint32_t name;
	uint32_t count;
	uint64_t duration;
};

#endif // DDR_STATS_H

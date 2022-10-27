/*
 * Copyright (C) 2021 Honor Device Co.Ltd
 * License terms: GNU General Public License (GPL) version 2
 *
 */

#ifndef PIL_Q6V5_SUBSYS_LOG
#define PIL_Q6V5_SUBSYS_LOG

void report_subsys_crash_log(const char *name, const char *reason, size_t reason_len);
void create_subsys_crash_log_queue(void);
void destroy_subsys_crash_log_queue(void);

#endif

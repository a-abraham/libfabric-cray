/*
 * Copyright (c) 2015 Los Alamos National Security, LLC. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifndef _GNIX_UTIL_H_
#define _GNIX_UTIL_H_

#include <stdio.h>
#include "rdma/fi_log.h"

extern struct fi_provider gnix_prov;

#define GNIX_WARN(subsystem, ...)                                              \
	FI_WARN(&gnix_prov, subsystem, __VA_ARGS__)
#define GNIX_TRACE(subsystem, ...)                                             \
	FI_TRACE(&gnix_prov, subsystem, __VA_ARGS__)
#define GNIX_INFO(subsystem, ...)                                              \
	FI_INFO(&gnix_prov, subsystem, __VA_ARGS__)
#define GNIX_DEBUG(subsystem, ...)                                             \
	FI_DBG(&gnix_prov, subsystem, __VA_ARGS__)

/*
 * TODO: This currently breaks the logging semantics. Discuss how to handle
 * error output.
 */
#define GNIX_ERR(subsystem, ...)                                               \
	fi_log(&gnix_prov, FI_LOG_WARN, subsystem, __func__, __LINE__,         \
	       __VA_ARGS__);

/*
 * prototypes
 */
int gnixu_get_rdma_credentials(void *addr, uint8_t *ptag, uint32_t *cookie);
int gnixu_to_fi_errno(int err);

int _gnix_task_is_not_app(void);
int _gnix_job_enable_unassigned_cpus(void);
int _gnix_job_disable_unassigned_cpus(void);
int _gnix_job_enable_affinity_apply(void);
int _gnix_job_disable_affinity_apply(void);

void _gnix_alps_cleanup(void);
int _gnix_job_fma_limit(uint32_t dev_id, uint8_t ptag, uint32_t *limit);
int _gnix_pes_on_node(uint32_t *num_pes);
int _gnix_nics_per_rank(uint32_t *nics_per_rank);

#endif

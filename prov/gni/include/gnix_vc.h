/*
 * Copyright (c) 2015 Cray Inc.  All rights reserved.
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

#ifndef _GNIX_VC_H_
#define _GNIX_VC_H_

#ifdef __cplusplus
extern "C" {
#endif

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "gnix.h"

/*
 * mode bits
 */
#define GNIX_VC_MODE_IN_WQ		(1U)
#define GNIX_VC_MODE_IN_HT		(1U << 1)
#define GNIX_VC_MODE_DG_POSTED		(1U << 2)
#define GNIX_VC_MODE_PENDING_MSGS	(1U << 3)

/*
 * defines for connection state for gnix VC
 */
enum gnix_vc_conn_state {
	GNIX_VC_CONN_NONE = 1,
	GNIX_VC_CONNECTING,
	GNIX_VC_CONNECTED,
	GNIX_VC_CONN_TERMINATING,
	GNIX_VC_CONN_TERMINATED,
	GNIX_VC_CONN_ERROR
};

enum gnix_vc_conn_req_type {
	GNIX_VC_CONN_REQ_CONN = 100,
	GNIX_VC_CONN_REQ_LISTEN
};

/**
 * Virual Connection (VC) struct
 *
 * @var send_queue           linked list of pending send requests to be
 *                           delivered to peer_address
 * @var entry                used internally for managing linked lists
 *                           of vc structs that require O(1) insertion/removal
 * @var peer_addr            address of peer with which this VC is connected
 * @var ep                   libfabric endpoint with which this VC is
 *                           associated
 * @var smsg_mbox            pointer to GNI SMSG mailbox used by this VC
 *                           to exchange SMSG messages with its peer
 * @var dgram                pointer to dgram - used in connection setup
 * @var gni_ep               GNI endpoint for this VC
 * @var outstanding_fab_reqs Count of outstanding libfabric level requests
 *                           associated with this endpoint.
 * @var conn_state           Connection state of this VC
 * @var vc_id                ID of this vc. Allows for rapid O(1) lookup
 *                           of the VC when using GNI_CQ_GET_INST_ID to get
 *                           the inst_id of a GNI CQE.
 * @var modes                Used internally to track current state of
 *                           the VC not pertaining to the connection state.
 */
struct gnix_vc {
	struct slist send_queue;
	struct dlist_entry entry;
	struct gnix_address peer_addr;
	struct gnix_fid_ep *ep;
	void *smsg_mbox;
	struct gnix_datagram *dgram;
	gni_ep_handle_t gni_ep;
	atomic_t outstanding_fab_reqs;
	enum gnix_vc_conn_state conn_state;
	int vc_id;
	int modes;
};

/*
 * prototypes
 */

/**
 * @brief Allocates a virtual channel(vc) struct
 *
 * @param[in]  ep_priv    pointer to previously allocated gnix_fid_ep object
 * @param[in]  dest_addr  remote peer address for this VC
 * @param[out] vc         location in which the address of the allocated vc
 *                        struct is to be returned.
 * @return FI_SUCCESS on success, -FI_ENOMEM if allocation of vc struct fails,
 */
int _gnix_vc_alloc(struct gnix_fid_ep *ep_priv, fi_addr_t dest_addr,
			struct gnix_vc **vc);

/**
 * @brief Initiates non-blocking connect of a vc with its peer
 *
 * @param[in]  vc   pointer to previously allocated vc struct
 *
 * @return FI_SUCCESS on success, -FI_EINVAL if an invalid field in the vc
 *         struct is encountered, -ENOMEM if insufficient memory to initiate
 *         connection request.
 */
int _gnix_vc_connect(struct gnix_vc *vc);


/**
 * @brief Sets up an accepting vc - one accepting vc can accept a
 *        single incoming connection request
 *
 * @param[in]  vc   pointer to previously allocated vc struct with
 *                  FI_ADDR_UNSPEC value supplied for the fi_addr_t
 *                  argument
 *
 * @return FI_SUCCESS on success, -FI_EINVAL if an invalid field in the vc
 *         struct is encountered, -ENOMEM if insufficient memory to initiate
 *         accept request.
 */
int _gnix_vc_accept(struct gnix_vc *vc);

/**
 * @brief Initiates a non-blocking disconnect of a vc from its peer
 *
 * @param[in]  vc   pointer to previously allocated and connected vc struct
 *
 * @return FI_SUCCESS on success, -FI_EINVAL if an invalid field in the vc
 *         struct is encountered, -ENOMEM if insufficient memory to initiate
 *         connection request.
 */
int _gnix_vc_disconnect(struct gnix_vc *vc);


/**
 * @brief Destroys a previously allocated vc and cleans up resources
 *        associated with the vc
 *
 * @param[in]  vc   pointer to previously allocated vc struct
 *
 * @return FI_SUCCESS on success, -FI_EINVAL if an invalid field in the vc
 *         struct is encountered.
 */
int _gnix_vc_destroy(struct gnix_vc *vc);

/**
 * @brief Add a vc to the work queue of its associated nic
 *
 * @param[in] vc  pointer to previously allocated vc struct
 *
 * @return FI_SUCCESS on success, -ENOMEM if insufficient memory
 * 	   allocate memory to enqueue work request
 */
int _gnix_vc_add_to_wq(struct gnix_vc *vc);

/*
 * inline functions
 */

/**
 * @brief Return connection state of a vc
 *
 * @param[in]  vc     pointer to previously allocated vc struct
 * @return connection state of vc
 */
static inline enum gnix_vc_conn_state _gnix_vc_state(struct gnix_vc *vc)
{
	assert(vc);
	return vc->conn_state;
}

#endif /* _GNIX_VC_H_ */
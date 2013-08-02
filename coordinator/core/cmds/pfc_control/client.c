/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * client.c - PFC daemon protocol client.
 */

#include "pfc_control.h"

/*
 * Internal prototypes.
 */
static int	client_send(cproto_sess_t *sess, pfc_ptr_t arg);
static int	client_receive(cproto_sess_t *sess, pfc_ptr_t arg);

/*
 * Control protocol command operations.
 */
static const pfc_ctrl_ops_t	client_ops = {
	.send		= client_send,
	.receive	= client_receive,
};

/*
 * int
 * client_create(cproto_sess_t *PFC_RESTRICT sess,
 *		 pfc_iowait_t *PFC_RESTRICT iwp)
 *	Create PFC daemon control protocol session.
 *
 * Calling/Exit State:
 *	Upon successful completion, the session is established on the context
 *	specified by `sess', and zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
client_create(cproto_sess_t *PFC_RESTRICT sess, pfc_iowait_t *PFC_RESTRICT iwp)
{
	pfc_timespec_t	timeout;

	timeout.tv_sec = ctrl_timeout;
	timeout.tv_nsec = 0;

	return pfc_ctrl_client_create(sess, &timeout, iwp);
}

/*
 * ctrlcmd_ret_t
 * client_execute(cproto_sess_t *PFC_RESTRICT sess,
 *		  const ctrlcmd_spec_t *PFC_RESTRICT spec)
 *	Execute subcommand on the specified control protocol session.
 *
 * Calling/Exit State:
 *	Upon successful completion, CMDRET_COMPLETE is returned.
 *	Otherwise CMDRET_FAIL is returned.
 */
ctrlcmd_ret_t
client_execute(cproto_sess_t *PFC_RESTRICT sess,
	       const ctrlcmd_spec_t *PFC_RESTRICT spec)
{
	int	err;

	err = pfc_ctrl_client_execute(sess, spec->cs_command, &client_ops,
				      (pfc_ptr_t)spec);
	if (PFC_EXPECT_FALSE(err != 0)) {
		return CMDRET_FAIL;
	}

	return CMDRET_COMPLETE;
}

/*
 * static int
 * client_send(cproto_sess_t *sess, pfc_ptr_t arg)
 *	Send optional arguments after protocol command.
 *	`arg' must be a pointer to ctrlcmd_spec_t associated with the current
 *	subcommand.
 */
static int
client_send(cproto_sess_t *sess, pfc_ptr_t arg)
{
	const ctrlcmd_spec_t	*spec = (const ctrlcmd_spec_t *)arg;
	int	err;

	/* Send optional arguments if needed. */
	if (spec->cs_send != NULL) {
		err = spec->cs_send(spec, sess);
	}
	else {
		err = 0;
	}

	return err;
}

/*
 * static int
 * client_receive(cproto_sess_t *sess, pfc_ptr_t arg)
 *	Receive additional response after protocol command response.
 *	`arg' must be a pointer to ctrlcmd_spec_t associated with the current
 *	subcommand.
 */
static int
client_receive(cproto_sess_t *sess, pfc_ptr_t arg)
{
	const ctrlcmd_spec_t	*spec = (const ctrlcmd_spec_t *)arg;
	int	err;

	/* Read additional response if needed. */
	if (spec->cs_receive != NULL) {
		err = spec->cs_receive(spec, sess);
	}
	else {
		err = 0;
	}

	return err;
}

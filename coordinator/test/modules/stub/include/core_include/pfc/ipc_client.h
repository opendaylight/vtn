/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef  PFC_IPC_CLIENT_H_
#define  PFC_IPC_CLIENT_H_

/*
 * Common definition for PFC Inter-Process Communication framework client.
 */

#include <pfc/base.h>
#include <pfc/ipc.h>
#include <pfc/clock.h>
#include <pfc/hostaddr.h>
#include <netinet/in.h>

PFC_C_BEGIN_DECL

/*
 * Invalid connection handle.
 */
#define  PFC_IPCCONN_INVALID  PFC_CONST_U(0)

/*
 * Handle of alternative connection pool.
 */
typedef uint32_t    pfc_ipccpool_t;

/*
 * Invalid connection pool handle.
 */
#define  PFC_IPCCPOOL_INVALID  PFC_CONST_U(0)

/*
 * Global connection pool handle.
 */
#define  PFC_IPCCPOOL_GLOBAL  PFC_CONST_U(1)

/*
 * Maximum capacity of the connection pool.
 */
#define  PFC_IPCCPOOL_MAX_CAPACITY  PFC_CONST_U(0x7fffffff)

/*
 * Flags for pfc_ipcclnt_cpool_close().
 */

/* Force to close connection in the pool. */
#define  PFC_IPCPLF_C_FORCE    PFC_CONST_U(0x1)

/*
 * Flags for pfc_ipcclnt_sess_create4() and pfc_ipcclnt_sess_altcreate5().
 */

/* Enable session-specific cancellation. */
#define  PFC_IPCSSF_CANCELABLE    PFC_CONST_U(0x00000001)

/*
 * Disable global cancellation by the call of pfc_ipcclnt_cancel(PFC_FALSE).
 * This flag implies PFC_IPCSF_CANCELABLE.
 */
#define  PFC_IPCSSF_NOGLOBCANCEL    PFC_CONST_U(0x00000002)

/*
 * IPC channel monitor option.
 */
static int result_;

typedef struct {
  /*
   * Interval, in milliseconds, between keep-alive probes on the remote
   * IPC monitor session. Default value is 30 seconds.
   */
  uint32_t  mon_keep_interval;

  /*
   * Timeout, in milliseconds, of keep-alive probe on the remote IPC
   * monitor session. Default is 10 seconds.
   */
  uint32_t  mon_keep_timeout;
} pfc_ipcmonopt_t;

/*
 * IPC event object.
 */
struct __pfc_ipcevent;
typedef struct __pfc_ipcevent  pfc_ipcevent_t;

/*
 * Prototype of IPC event handler function.
 */
typedef void  (*pfc_ipcevfunc_t)(pfc_ipcevent_t *event, pfc_ptr_t arg);

/*
 * Prototype of destructor of IPC event handler's argument.
 */
typedef void  (*pfc_ipcevdtor_t)(pfc_ptr_t arg);

/*
 * IPC event handler's attributes.
 */
#ifdef  PFC_LP64
#define  __PFC_IPCEVATTR_SIZE    PFC_CONST_U(56)
#else  /* !PFC_LP64 */
#define  __PFC_IPCEVATTR_SIZE    PFC_CONST_U(32)
#endif  /* PFC_LP64 */

typedef union {
  uint8_t    attr[__PFC_IPCEVATTR_SIZE];
  uint64_t  align;
} pfc_ipcevattr_t;

/*
 * Identifier of IPC event handler.
 */
typedef uint32_t  pfc_ipcevhdlr_t;

/*
 * Invalid ID of IPC event handler.
 */
#define  PFC_IPCEVHDLR_INVALID    ((pfc_ipcevhdlr_t)0)

/*
 * Event type for IPC channel state change event.
 */
#define  PFC_IPCCHSTATE_UP  PFC_CONST_U(0)  /* IPC channel is up */
#define  PFC_IPCCHSTATE_DOWN  PFC_CONST_U(1)  /* IPC channel is down */
#define  PFC_IPCCHSTATE_NOTIFY  PFC_CONST_U(2)  /* state notification */


#define  PFC_IPCCHDOWN_REFUSED  PFC_CONST_U(1)  /* connection refused */
#define  PFC_IPCCHDOWN_RESET  PFC_CONST_U(2)  /* reset by peer */
#define  PFC_IPCCHDOWN_HANGUP  PFC_CONST_U(3)  /* hang up */
#define  PFC_IPCCHDOWN_TIMEDOUT  PFC_CONST_U(4)  /* timed out */
#define  PFC_IPCCHDOWN_AUTHFAIL  PFC_CONST_U(5)  /* authentication failed */
#define  PFC_IPCCHDOWN_TOOMANY  PFC_CONST_U(6)  /* too many connections */
#define  PFC_IPCCHDOWN_ERROR  PFC_CONST_U(7)  /* closed due to error */

#define  PFC_IPCCHNOTIFY_DOWN  PFC_CONST_U(0)  /* IPC channel is down */
#define  PFC_IPCCHNOTIFY_UP  PFC_CONST_U(1)  /* IPC channel is up */

static inline void
pfc_ipcevent_mask_empty(pfc_ipcevmask_t *mask) {
  *mask = PFC_IPC_EVENT_MASK_EMPTY;
}

static inline void
pfc_ipcevent_mask_fill(pfc_ipcevmask_t *mask) {
  *mask = PFC_IPC_EVENT_MASK_FILL;
}

static inline int
pfc_ipcevent_mask_add(pfc_ipcevmask_t *mask, pfc_ipcevtype_t type) {
  if (PFC_EXPECT_FALSE(!PFC_IPC_EVTYPE_IS_VALID(type))) {
    return EINVAL;
  }

  *mask |= PFC_IPC_EVENT_MASK_BIT(type);

  return 0;
}

static inline int
pfc_ipcevent_mask_remove(pfc_ipcevmask_t *mask, pfc_ipcevtype_t type) {
  if (PFC_EXPECT_FALSE(!PFC_IPC_EVTYPE_IS_VALID(type))) {
    return EINVAL;
  }

  *mask &= ~PFC_IPC_EVENT_MASK_BIT(type);

  return 0;
}

static inline pfc_bool_t
pfc_ipcevent_mask_test(const pfc_ipcevmask_t *mask, pfc_ipcevtype_t type) {
  if (PFC_EXPECT_FALSE(!PFC_IPC_EVTYPE_IS_VALID(type))) {
    return PFC_FALSE;
  }

  return (*mask & PFC_IPC_EVENT_MASK_BIT(type)) ? PFC_TRUE : PFC_FALSE;
}

#define  PFC_IPCCLNT_OUTPUT_STRUCT(sess, stname, data)      \
    __pfc_ipcclnt_output_struct((sess), (const uint8_t *)(data),  \
                                __PFC_IPC_STRUCT_SIZE(stname),  \
                                __PFC_IPC_STRUCT_STRINGIFY(stname),  \
                                __PFC_IPC_STRUCT_SIGNATURE(stname))

#define  PFC_IPCCLNT_GETRES_STRUCT(sess, index, stname, datap)    \
    __pfc_ipcclnt_getres_struct((sess), (index), \
                                reinterpret_cast<uint8_t *> (datap), \
                                __PFC_IPC_STRUCT_SIZE(stname),  \
                                __PFC_IPC_STRUCT_STRINGIFY(stname),  \
                                __PFC_IPC_STRUCT_SIGNATURE(stname))

extern int pfc_ipcclnt_sess_create(pfc_ipcsess_t **PFC_RESTRICT sessp,
                                   const char *PFC_RESTRICT name,
                                   pfc_ipcid_t service);
extern int pfc_ipcclnt_sess_create4(pfc_ipcsess_t **PFC_RESTRICT sessp,
                                    const char *PFC_RESTRICT name,
                                    pfc_ipcid_t service, uint32_t flags);
extern int pfc_ipcclnt_sess_reset(pfc_ipcsess_t *PFC_RESTRICT sess,
                                  const char *PFC_RESTRICT name,
                                  pfc_ipcid_t service);
extern int pfc_ipcclnt_sess_settimeout(pfc_ipcsess_t *PFC_RESTRICT sess,
                                       const pfc_timespec_t *PFC_RESTRICT
                                       timeout);
extern int pfc_ipcclnt_sess_invoke(pfc_ipcsess_t *PFC_RESTRICT sess,
                                   pfc_ipcresp_t *PFC_RESTRICT respp);
extern int pfc_ipcclnt_sess_cancel(pfc_ipcsess_t *sess,
                                   pfc_bool_t discard);
extern int pfc_ipcclnt_sess_destroy(pfc_ipcsess_t *sess);
extern void  __pfc_ipcclnt_sess_destroy(pfc_ipcsess_t *sess);

extern int pfc_ipcclnt_output_int8(pfc_ipcsess_t *sess, int8_t data);
extern int pfc_ipcclnt_output_uint8(pfc_ipcsess_t *sess, uint8_t data);
extern int pfc_ipcclnt_output_int16(pfc_ipcsess_t *sess, int16_t data);
extern int pfc_ipcclnt_output_uint16(pfc_ipcsess_t *sess, uint16_t data);
extern int pfc_ipcclnt_output_int32(pfc_ipcsess_t *sess, int32_t data);
extern int pfc_ipcclnt_output_uint32(pfc_ipcsess_t *sess, uint32_t data);
extern int pfc_ipcclnt_output_int64(pfc_ipcsess_t *sess, int64_t data);
extern int pfc_ipcclnt_output_uint64(pfc_ipcsess_t *sess, uint64_t data);
extern int pfc_ipcclnt_output_float(pfc_ipcsess_t *sess, float data);
extern int pfc_ipcclnt_output_double(pfc_ipcsess_t *sess, double data);
extern int pfc_ipcclnt_output_ipv4(pfc_ipcsess_t *PFC_RESTRICT sess,
                                   struct in_addr *PFC_RESTRICT data);
extern int pfc_ipcclnt_output_ipv6(pfc_ipcsess_t *PFC_RESTRICT sess,
                                   struct in6_addr *PFC_RESTRICT data);
extern int pfc_ipcclnt_output_string(pfc_ipcsess_t *PFC_RESTRICT sess,
                                     const char *PFC_RESTRICT data);
extern int pfc_ipcclnt_output_binary(pfc_ipcsess_t *PFC_RESTRICT sess,
                                     const uint8_t *PFC_RESTRICT data,
                                     uint32_t length);
extern int pfc_ipcclnt_output_null(pfc_ipcsess_t *sess);
extern int pfc_ipcclnt_output_stdef(pfc_ipcsess_t *PFC_RESTRICT sess,
                                    const pfc_ipcstdef_t *PFC_RESTRICT
                                    defp, pfc_cptr_t data);

extern int pfc_ipcclnt_getres_int8(pfc_ipcsess_t *PFC_RESTRICT sess,
                                   uint32_t index,
                                   int8_t *PFC_RESTRICT datap);
extern int pfc_ipcclnt_getres_uint8(pfc_ipcsess_t *PFC_RESTRICT sess,
                                    uint32_t index,
                                    uint8_t *PFC_RESTRICT datap);
extern int pfc_ipcclnt_getres_int16(pfc_ipcsess_t *PFC_RESTRICT sess,
                                    uint32_t index,
                                    int16_t *PFC_RESTRICT datap);
extern int pfc_ipcclnt_getres_uint16(pfc_ipcsess_t *PFC_RESTRICT sess,
                                     uint32_t index,
                                     uint16_t *PFC_RESTRICT datap);
extern int pfc_ipcclnt_getres_int32(pfc_ipcsess_t *PFC_RESTRICT sess,
                                    uint32_t index,
                                    int32_t *PFC_RESTRICT datap);
extern int pfc_ipcclnt_getres_uint32(pfc_ipcsess_t *PFC_RESTRICT sess,
                                     uint32_t index,
                                     uint32_t *PFC_RESTRICT datap);
extern int pfc_ipcclnt_getres_int64(pfc_ipcsess_t *PFC_RESTRICT sess,
                                    uint32_t index,
                                    int64_t *PFC_RESTRICT datap);
extern int pfc_ipcclnt_getres_uint64(pfc_ipcsess_t *PFC_RESTRICT sess,
                                     uint32_t index,
                                     uint64_t *PFC_RESTRICT datap);
extern int pfc_ipcclnt_getres_float(pfc_ipcsess_t *PFC_RESTRICT sess,
                                    uint32_t index,
                                    float *PFC_RESTRICT datap);
extern int pfc_ipcclnt_getres_double(pfc_ipcsess_t *PFC_RESTRICT sess,
                                     uint32_t index,
                                     double *PFC_RESTRICT datap);
extern int pfc_ipcclnt_getres_ipv4(pfc_ipcsess_t *PFC_RESTRICT sess,
                                   uint32_t index,
                                   struct in_addr *PFC_RESTRICT datap);
extern int pfc_ipcclnt_getres_ipv6(pfc_ipcsess_t *PFC_RESTRICT sess,
                                   uint32_t index,
                                   struct in6_addr *PFC_RESTRICT datap);
extern int pfc_ipcclnt_getres_string(pfc_ipcsess_t *PFC_RESTRICT sess,
                                     uint32_t index,
                                     const char **PFC_RESTRICT datapp);
extern int pfc_ipcclnt_getres_binary(pfc_ipcsess_t *PFC_RESTRICT sess,
                                     uint32_t index,
                                     const uint8_t **PFC_RESTRICT datapp,
                                     uint32_t *PFC_RESTRICT lengthp);
extern int pfc_ipcclnt_getres_stdef(pfc_ipcsess_t *PFC_RESTRICT sess,
                                    uint32_t index,
                                    const pfc_ipcstdef_t *PFC_RESTRICT
                                    defp, pfc_ptr_t datap);

extern uint32_t  pfc_ipcclnt_getrescount(pfc_ipcsess_t *sess);
extern int pfc_ipcclnt_getrestype(pfc_ipcsess_t *PFC_RESTRICT sess,
                                  uint32_t index,
                                  pfc_ipctype_t *PFC_RESTRICT typep);
extern int pfc_ipcclnt_getres_structname(pfc_ipcsess_t *PFC_RESTRICT sess,
                                         uint32_t index,
                                         const char **PFC_RESTRICT namepp);

extern int pfc_ipcclnt_setdefault(const char *name);

extern pfc_bool_t  pfc_ipcclnt_isdisabled(void);

extern int pfc_ipcclnt_cpool_create(pfc_ipccpool_t *poolp,
                                    uint32_t capacity);
extern int pfc_ipcclnt_cpool_destroy(pfc_ipccpool_t pool);
extern int pfc_ipcclnt_cpool_open(pfc_ipccpool_t pool,
                                  const char *PFC_RESTRICT name,
                                  pfc_ipcconn_t *PFC_RESTRICT connp);
extern int pfc_ipcclnt_cpool_close(pfc_ipccpool_t pool,
                                   pfc_ipcconn_t conn, uint32_t flags);
extern uint32_t  pfc_ipcclnt_cpool_getsize(pfc_ipccpool_t pool);
extern uint32_t  pfc_ipcclnt_cpool_getcapacity(pfc_ipccpool_t pool);

extern int pfc_ipcclnt_sess_altcreate(pfc_ipcsess_t **PFC_RESTRICT sessp,
                                      pfc_ipcconn_t conn,
                                      const char *PFC_RESTRICT name,
                                      pfc_ipcid_t service);
extern int pfc_ipcclnt_sess_altcreate5(pfc_ipcsess_t **PFC_RESTRICT sessp,
                                       pfc_ipcconn_t conn,
                                       const char *PFC_RESTRICT name,
                                       pfc_ipcid_t service,
                                       uint32_t flags);

extern int pfc_ipcclnt_forward(pfc_ipcsess_t *PFC_RESTRICT dsess,
                               pfc_ipcsess_t *PFC_RESTRICT ssess,
                               uint32_t begin, uint32_t end);
extern int pfc_ipcclnt_forward_fromsrv(pfc_ipcsess_t *PFC_RESTRICT dsess,
                                       pfc_ipcsrv_t *PFC_RESTRICT ssrv,
                                       uint32_t begin, uint32_t end);
extern int pfc_ipcclnt_forward_tosrv(pfc_ipcsrv_t *PFC_RESTRICT dsrv,
                                     pfc_ipcsess_t *PFC_RESTRICT ssess,
                                     uint32_t begin, uint32_t end);

extern int pfc_ipcclnt_monitor(const char *PFC_RESTRICT name,
                               pfc_bool_t *PFC_RESTRICT connected,
                               const pfc_timespec_t *PFC_RESTRICT
                               conn_timeout);
extern int pfc_ipcclnt_monitor4(const char *PFC_RESTRICT name,
                                pfc_bool_t *PFC_RESTRICT connected,
                                const pfc_timespec_t *PFC_RESTRICT
                                conn_timeout,
                                const pfc_ipcmonopt_t *PFC_RESTRICT opts);
extern int pfc_ipcclnt_monitor_cancel(const char *name);
extern void  pfc_ipcclnt_monitor_cancelall(void);

extern int pfc_ipcclnt_hostset_create(const char *name);
extern int pfc_ipcclnt_hostset_destroy(const char *name);
extern int pfc_ipcclnt_hostset_exists(const char *name);
extern int pfc_ipcclnt_hostset_add(const char *PFC_RESTRICT name,
                                   const pfc_hostaddr_t *PFC_RESTRICT
                                   addr);
extern int pfc_ipcclnt_hostset_remove(const char *PFC_RESTRICT name,
                                      const pfc_hostaddr_t *PFC_RESTRICT
                                      addr);
extern int pfc_ipcclnt_hostset_contains(const char *PFC_RESTRICT name,
                                        const pfc_hostaddr_t *PFC_RESTRICT
                                        addr);

extern int pfc_ipcevent_attr_init(pfc_ipcevattr_t *attr);
extern void  pfc_ipcevent_attr_destroy(pfc_ipcevattr_t *attr);
extern int pfc_ipcevent_attr_gethostset(const pfc_ipcevattr_t
                                        *PFC_RESTRICT attr,
                                        const char **PFC_RESTRICT namep);
extern int pfc_ipcevent_attr_sethostset(pfc_ipcevattr_t *PFC_RESTRICT attr,
                                        const char *PFC_RESTRICT name);
extern int pfc_ipcevent_attr_gettarget(const pfc_ipcevattr_t *PFC_RESTRICT
                                       attr,
                                       const char *PFC_RESTRICT service,
                                       pfc_ipcevmask_t *PFC_RESTRICT mask);
extern int pfc_ipcevent_attr_addtarget(pfc_ipcevattr_t *PFC_RESTRICT attr,
                                       const char *PFC_RESTRICT service,
                                       const pfc_ipcevmask_t *PFC_RESTRICT
                                       mask);
extern int pfc_ipcevent_attr_resettarget(pfc_ipcevattr_t *attr);
extern int pfc_ipcevent_attr_getpriority(const pfc_ipcevattr_t
                                         *PFC_RESTRICT attr,
                                         uint32_t *PFC_RESTRICT prip);
extern int pfc_ipcevent_attr_setpriority(pfc_ipcevattr_t *attr,
                                         uint32_t pri);
extern int pfc_ipcevent_attr_getarg(const pfc_ipcevattr_t *PFC_RESTRICT
                                    attr,
                                    pfc_ptr_t *PFC_RESTRICT argp);
extern int pfc_ipcevent_attr_setarg(pfc_ipcevattr_t *PFC_RESTRICT attr,
                                    pfc_ptr_t arg);
extern int pfc_ipcevent_attr_getargdtor(const pfc_ipcevattr_t
                                        *PFC_RESTRICT attr,
                                        pfc_ipcevdtor_t *PFC_RESTRICT
                                        dtorp);
extern int pfc_ipcevent_attr_setargdtor(pfc_ipcevattr_t *PFC_RESTRICT attr,
                                        pfc_ipcevdtor_t dtor);
extern int pfc_ipcevent_attr_getlog(const pfc_ipcevattr_t *PFC_RESTRICT
                                    attr,
                                    pfc_bool_t *PFC_RESTRICT logp);
extern int pfc_ipcevent_attr_setlog(pfc_ipcevattr_t *PFC_RESTRICT attr,
                                    pfc_bool_t log);

extern int pfc_ipcevent_isconnected(const char *PFC_RESTRICT channel,
                                    const pfc_hostaddr_t *PFC_RESTRICT
                                    addr);
extern int pfc_ipcevent_notifystate(pfc_ipcevhdlr_t id);

extern pfc_ipcevid_t  pfc_ipcevent_getserial(pfc_ipcevent_t *event);
extern pfc_ipcevtype_t  pfc_ipcevent_gettype(pfc_ipcevent_t *event);
extern void    pfc_ipcevent_gettime(pfc_ipcevent_t *PFC_RESTRICT event,
                                    pfc_timespec_t *PFC_RESTRICT tsp);
extern const char  *pfc_ipcevent_getchannelname(pfc_ipcevent_t *event);
extern const pfc_hostaddr_t  *pfc_ipcevent_gethostaddr(pfc_ipcevent_t
                                                       *event);
extern const char  *pfc_ipcevent_getservicename(pfc_ipcevent_t *event);
extern pfc_ipcsess_t  *pfc_ipcevent_getsess(pfc_ipcevent_t *event);
extern pfc_bool_t  pfc_ipcevent_isstatechange(pfc_ipcevent_t *event);

typedef struct {
  uint32_t  evopt_idle_timeout;
  uint32_t  evopt_maxthreads;
  uint32_t  evopt_conn_interval;
  uint32_t  evopt_keep_interval;
  uint32_t  evopt_keep_timeout;
  uint32_t  evopt_timeout;
  int  (*evopt_thread_create)(void *(*func)(void *), void *arg);
} pfc_ipcevopts_t;

extern int pfc_ipcclnt_event_init(const pfc_ipcevopts_t *opts);
extern int pfc_ipcclnt_event_shutdown(void);
extern int pfc_ipcclnt_event_fini(void);

extern int pfc_ipcevent_add_handler(pfc_ipcevhdlr_t *PFC_RESTRICT idp,
                                    const char *PFC_RESTRICT channel,
                                    pfc_ipcevfunc_t func,
                                    const pfc_ipcevattr_t *PFC_RESTRICT
                                    attr,
                                    const char *PFC_RESTRICT name);
extern int pfc_ipcevent_remove_handler(pfc_ipcevhdlr_t id);

extern void  pfc_ipcclnt_cancel(pfc_bool_t permanent);
extern void  pfc_ipcclnt_cpool_reap(pfc_bool_t forced);
extern void  pfc_ipcclnt_enable_log(pfc_bool_t enable);

extern int  __pfc_ipcclnt_output_struct(pfc_ipcsess_t *PFC_RESTRICT sess,
                                        const uint8_t *PFC_RESTRICT data,
                                        uint32_t length,
                                        const char *PFC_RESTRICT stname,
                                        const char *PFC_RESTRICT sig);
extern int  __pfc_ipcclnt_getres_struct(pfc_ipcsess_t *PFC_RESTRICT sess,
                                        uint32_t index,
                                        uint8_t *PFC_RESTRICT datap,
                                        uint32_t length,
                                        const char *PFC_RESTRICT stname,
                                        const char *PFC_RESTRICT sig);



static inline int
pfc_ipcclnt_altopen(const char *PFC_RESTRICT name,
                    pfc_ipcconn_t *PFC_RESTRICT connp) {
  *connp = 1;
  return result_;
}

static inline void
set_pfc_ipcclnt_altopen(int result) {
  result_ = result;
}

static inline int
pfc_ipcclnt_altclose(pfc_ipcconn_t conn) {
  return 1;
}

PFC_C_END_DECL

#endif  // PFC_IPC_CLIENT_H_

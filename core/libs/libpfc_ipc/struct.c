/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * struct.c - IPC struct information management.
 */

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pfc/util.h>
#include "ipc_struct_impl.h"

/*
 * Path to IPC struct information file.
 */
#define	IPC_STRUCT_BIN_PATH	PFC_DATADIR "/ipc/ipc_struct.bin"

/*
 * Read-Write lock which protects IPC struct information.
 */
static pfc_rwlock_t	ipc_struct_lock = PFC_RWLOCK_INITIALIZER;

#define	IPC_STRUCT_RDLOCK()					\
	PFC_ASSERT_INT(pfc_rwlock_rdlock(&ipc_struct_lock), 0)
#define	IPC_STRUCT_WRLOCK()					\
	PFC_ASSERT_INT(pfc_rwlock_wrlock(&ipc_struct_lock), 0)
#define	IPC_STRUCT_UNLOCK()					\
	PFC_ASSERT_INT(pfc_rwlock_unlock(&ipc_struct_lock), 0)

#define	IPC_STRUCT_TRYWRLOCK()	pfc_rwlock_trywrlock(&ipc_struct_lock)

/*
 * Global reference counter of struct information.
 * We use global counter instead of per ipc_strinfo_t counter because
 * the latter costs too much CPU time on nested struct maintenance.
 */
uint32_t	ipc_struct_refcnt PFC_ATTR_HIDDEN;

/*
 * Context to load struct information.
 */
typedef struct {
	ipc_ifhead_t	*sld_head;	/* struct information header */
	pfc_rbtree_t	*sld_tree;	/* target struct tree */
	const char	*sld_path;	/* path to information file */
	uint32_t	sld_size;	/* size of sld_head mapping */
} ipc_strload_t;

#define	IPC_STRLOAD_INIT(ldctx, tree, path)	\
	do {					\
		(ldctx)->sld_head = NULL;	\
		(ldctx)->sld_size = 0;		\
		(ldctx)->sld_tree = (tree);	\
		(ldctx)->sld_path = (path);	\
	} while (0)

/*
 * Context to fetch struct field information.
 */
typedef struct {
	ipc_strinfo_t	*fld_strinfo;		/* target struct information */
	ipc_fldinfo_t	*fld_fldinfo;		/* field information */
	ipc_iffield_t	*fld_field;		/* field section entry */
	uint32_t	fld_size;		/* size of struct */
} ipc_fldload_t;

#define	IPC_FLDLOAD_INIT(loadp, sip, flp, ffp)		\
	do {						\
		(loadp)->fld_strinfo = (sip);		\
		(loadp)->fld_fldinfo = (flp);		\
		(loadp)->fld_field = (ffp);		\
		(loadp)->fld_size = 0;			\
	} while (0)

/*
 * Internal prototypes.
 */
static int	ipc_struct_load(ipc_strload_t *ldctx);
static int	ipc_struct_getmap(ipc_strload_t *ldctx);
static int	ipc_struct_load_entry(ipc_strload_t *PFC_RESTRICT ldctx,
				      ipc_ifstruct_t *PFC_RESTRICT fsp);
static int	ipc_struct_load_field(ipc_strload_t *PFC_RESTRICT ldctx,
				      ipc_fldload_t *PFC_RESTRICT loadp);

static int	ipc_fldmeta_load(ipc_strload_t *ldctx);
static int	ipc_fldmeta_load_struct(ipc_strload_t *PFC_RESTRICT ldctx,
					ipc_ifstruct_t *PFC_RESTRICT fsp,
					uint32_t *loaded);
static int	ipc_fldmeta_load_field(ipc_strload_t *PFC_RESTRICT ldctx,
				       ipc_strinfo_t *PFC_RESTRICT sip,
				       uint32_t index, uint32_t offset,
				       uint32_t size,
				       ipc_iffield_t *PFC_RESTRICT ffp);

static int	ipc_strinfo_lookup(const char *PFC_RESTRICT name,
				   ipc_strinfo_t **PFC_RESTRICT sipp);

static const char	*ipc_struct_string(ipc_ifhead_t *head, uint32_t index);
static pfc_cptr_t	ipc_strinfo_getkey(pfc_rbnode_t *node);
static pfc_cptr_t	ipc_fldmeta_getkey(pfc_rbnode_t *node);

static void	ipc_strinfo_dtor(pfc_rbnode_t *node,
				 pfc_ptr_t arg PFC_ATTR_UNUSED);
static void	ipc_fldmeta_dtor(pfc_rbnode_t *node,
				 pfc_ptr_t arg PFC_ATTR_UNUSED);

/*
 * Red-Black tree which keeps IPC struct information.
 */
static pfc_rbtree_t	ipc_structs =
	PFC_RBTREE_INITIALIZER((pfc_rbcomp_t)strcmp, ipc_strinfo_getkey);

/*
 * Clean up all struct information.
 * This macro must be used with holding the IPC struct lock in writer mode.
 */
#define	IPC_STRUCT_CLEANUP(tree)			\
	pfc_rbtree_clear(tree, ipc_strinfo_dtor, NULL)

/*
 * Clean up all field meta data of the specified IPC struct.
 * This macro must be used with holding the IPC struct lock in writer mode.
 */
#define	IPC_FLDMETA_CLEANUP(sip)					\
	pfc_rbtree_clear(&(sip)->sti_fldmeta, ipc_fldmeta_dtor, NULL)

/*
 * Result of loading IPC struct information file.
 * Zero means it was loaded successfully.
 * Positive value is the error number which indicates the cause of load error.
 */
static volatile int	ipc_struct_bin_error = -1;

/*
 * Result of loading IPC struct field meta data.
 * Zero means it was loaded successfully.
 * Positive value is the error number which indicates the cause of load error.
 */
static volatile int	ipc_fldmeta_error = -1;

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * ipc_strload_destroy(ipc_strload_t *ldctx)
 *	Destroy the loader context.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
ipc_strload_destroy(ipc_strload_t *ldctx)
{
	if (ldctx->sld_head != NULL) {
		(void)munmap(ldctx->sld_head, ldctx->sld_size);
	}

	if (ldctx->sld_tree != &ipc_structs) {
		IPC_STRUCT_CLEANUP(ldctx->sld_tree);
	}
}

/*
 * int
 * pfc_ipc_struct_load(void)
 *	Load struct information from the IPC struct information file.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipc_struct_load(void)
{
	ipc_strload_t	ldctx;
	int		err;

	IPC_STRLOAD_INIT(&ldctx, &ipc_structs, IPC_STRUCT_BIN_PATH);

	IPC_STRUCT_WRLOCK();
	err = ipc_struct_load(&ldctx);
	ipc_strload_destroy(&ldctx);
	IPC_STRUCT_UNLOCK();

	return err;
}

/*
 * int
 * pfc_ipc_struct_load_fields(void)
 *	Load meta data of IPC struct fields from the IPC struct information
 *	file only if it is not yet loaded.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipc_struct_load_fields(void)
{
	ipc_strload_t	ldctx;
	int		err;

	IPC_STRLOAD_INIT(&ldctx, &ipc_structs, IPC_STRUCT_BIN_PATH);

	IPC_STRUCT_WRLOCK();
	err = ipc_fldmeta_load(&ldctx);
	ipc_strload_destroy(&ldctx);
	IPC_STRUCT_UNLOCK();

	return err;
}

/*
 * int
 * pfc_ipc_struct_loadfile(const char *path, pfc_bool_t need_fields)
 *	Load an additional IPC structure information file specified by `path'.
 *
 *	If `need_fields' is PFC_TRUE, information about structure fields are
 *	also loaded.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Currently, this function is provided only for debugging purpose.
 */
int
pfc_ipc_struct_loadfile(const char *path, pfc_bool_t need_fields)
{
	ipc_strload_t	ldctx;
	pfc_rbtree_t	tree;
	int		err;

	IPC_STRLOAD_INIT(&ldctx, &ipc_structs, IPC_STRUCT_BIN_PATH);

	IPC_STRUCT_WRLOCK();

	/* At first, load default IPC structure information. */
	err = (need_fields) ? ipc_fldmeta_load(&ldctx)
		: ipc_struct_load(&ldctx);
	ipc_strload_destroy(&ldctx);

	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	/* Load the specified file. */
	pfc_rbtree_init(&tree, (pfc_rbcomp_t)strcmp, ipc_strinfo_getkey);
	IPC_STRLOAD_INIT(&ldctx, &tree, path);

	err = (need_fields) ? ipc_fldmeta_load(&ldctx)
		: ipc_struct_load(&ldctx);

	if (PFC_EXPECT_TRUE(err == 0)) {
		pfc_rbnode_t	*node;

		/* Merge loaded entries with ipc_structs. */
		while ((node = pfc_rbtree_next(&tree, NULL)) != NULL) {
			pfc_rbtree_remove_node(&tree, node);

			PFC_ASSERT_INT(pfc_rbtree_put(&ipc_structs, node), 0);
		}
	}
	ipc_strload_destroy(&ldctx);

out:
	IPC_STRUCT_UNLOCK();

	return err;
}

/*
 * int
 * pfc_ipc_struct_loaddefault(const char *path, pfc_bool_t need_fields)
 *	Load default struct information from the given file.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	Currently, this function is provided only for debugging purpose.
 */
int
pfc_ipc_struct_loaddefault(const char *path, pfc_bool_t need_fields)
{
	ipc_strload_t	ldctx;
	int		err;

	IPC_STRLOAD_INIT(&ldctx, &ipc_structs, path);

	IPC_STRUCT_WRLOCK();
	err = (need_fields) ? ipc_fldmeta_load(&ldctx)
		: ipc_struct_load(&ldctx);
	ipc_strload_destroy(&ldctx);
	IPC_STRUCT_UNLOCK();

	return err;
}

/*
 * int
 * pfc_ipc_strinfo_get(const char *PFC_RESTRICT name,
 *		       ipc_cstrinfo_t **PFC_RESTRICT sipp,
 *		       pfc_bool_t need_fields)
 *	Get the IPC structure information associated by the IPC structure
 *	name specified by `name'.
 *
 *	This function loads IPC structure information file if not yet loaded.
 *	If `need_fields' is PFC_TRUE, information about structure fields are
 *	also loaded.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to IPC structure information
 *	is set to the buffer pointed by `sipp', and zero is returned.
 *	Note that the caller must release the IPC structure information by
 *	IPC_STRINFO_RELEASE().
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
int
pfc_ipc_strinfo_get(const char *PFC_RESTRICT name,
		    ipc_cstrinfo_t **PFC_RESTRICT sipp, pfc_bool_t need_fields)
{
	ipc_strload_t	ldctx;
	int		err;

	if (PFC_EXPECT_FALSE(name == NULL || sipp == NULL)) {
		return EINVAL;
	}

	IPC_STRLOAD_INIT(&ldctx, &ipc_structs, IPC_STRUCT_BIN_PATH);

	IPC_STRUCT_RDLOCK();

	/* Load IPC structure information. */
	err = (need_fields) ? ipc_fldmeta_load(&ldctx)
		: ipc_struct_load(&ldctx);
	if (PFC_EXPECT_TRUE(err == 0)) {
		/*
		 * Search for the information about the specified IPC structure
		 * name.
		 */
		err = ipc_strinfo_lookup(name, (ipc_strinfo_t **)sipp);
	}

	ipc_strload_destroy(&ldctx);

	IPC_STRUCT_UNLOCK();

	return err;
}

/*
 * ipc_fldmeta_t *
 * pfc_ipc_strinfo_getfield(ipc_strinfo_t *PFC_RESTRICT sip,
 *			    const char *PFC_RESTRICT name)
 *	Return the meta data of the IPC structure field specified by the
 *	field name.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to ipc_fldmeta_t is
 *	returned.
 *	NULL is returned if not found.
 *
 * Remarks:
 *	This function always return NULL if field information is not yet
 *	loaded by pfc_ipc_struct_load_fields().
 */
ipc_fldmeta_t *
pfc_ipc_strinfo_getfield(ipc_strinfo_t *PFC_RESTRICT sip,
			 const char *PFC_RESTRICT name)
{
	pfc_rbnode_t	*node;
	ipc_fldmeta_t	*fmp;

	PFC_ASSERT(name != NULL);

	IPC_STRUCT_RDLOCK();
	PFC_ASSERT(ipc_struct_refcnt != 0);

	node = pfc_rbtree_get(&sip->sti_fldmeta, name);
	if (PFC_EXPECT_TRUE(node != NULL)) {
		fmp = IPC_FLDMETA_NODE2PTR(node);
	}
	else {
		fmp = NULL;
	}

	IPC_STRUCT_UNLOCK();

	return fmp;
}

/*
 * void
 * pfc_ipc_strinfo_hold(ipc_cstrinfo_t *csip)
 *	Hold reference to the specified IPC structure information.
 */
void
pfc_ipc_strinfo_hold(ipc_cstrinfo_t *csip)
{
	PFC_ASSERT(ipc_struct_refcnt > 0);
	pfc_atomic_inc_uint32(&ipc_struct_refcnt);
}

/*
 * void
 * pfc_ipc_strinfo_release(ipc_cstrinfo_t *csip)
 *	Release reference to the specified IPC structure information.
 */
void
pfc_ipc_strinfo_release(ipc_cstrinfo_t *csip)
{
#ifdef	PFC_VERBOSE_DEBUG
	uint32_t	ref = pfc_atomic_dec_uint32_old(&ipc_struct_refcnt);

	PFC_ASSERT(ref > 0);
#else	/* !PFC_VERBOSE_DEBUG */
	pfc_atomic_dec_uint32(&ipc_struct_refcnt);
#endif	/* PFC_VERBOSE_DEBUG */
}

/*
 * int PFC_ATTR_HIDDEN
 * pfc_ipc_struct_get(const char *PFC_RESTRICT name,
 *		      const char *PFC_RESTRICT sig,
 *		      pfc_cptr_t PFC_RESTRICT addr, uint32_t size,
 *		      ipc_cstrinfo_t **PFC_RESTRICT sipp)
 *	Obtain IPC struct information associated with the name specified by
 *	`name'.
 *
 *	`addr' must be an address of the specified struct data.
 *	It is used only check address alignment.
 *
 *	`sig' must be the layout signature required by the struct.
 *	`size' must be expected struct size.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to ipc_strinfo_t
 *	associated with the given name is set to `*sipp', and zero is returned.
 *
 *	EFAULT is returned if `data' is not aligned on required boundary.
 *	Otherwise ENODEV is returned on error.
 *
 * Remarks:
 *	The caller must release ipc_cstrinfo_t set in `*sipp' by
 *	IPC_STRINFO_RELEASE().
 */
int PFC_ATTR_HIDDEN
pfc_ipc_struct_get(const char *PFC_RESTRICT name,
		   const char *PFC_RESTRICT sig,
		   pfc_cptr_t PFC_RESTRICT addr, uint32_t size,
		   ipc_cstrinfo_t **PFC_RESTRICT sipp)
{
	ipc_strinfo_t	*sip = NULL;	/* Suppress unreasonable warning. */
	ipc_cpduops_t	*ops;
	int		diff, err;

	PFC_ASSERT(name != NULL && sig != NULL && sipp != NULL);
	PFC_ASSERT(strlen(sig) == sizeof(sip->sti_sig));

	/* Search for a struct information associated with the given name. */
	IPC_STRUCT_RDLOCK();
	err = ipc_strinfo_lookup(name, &sip);
	IPC_STRUCT_UNLOCK();
	if (PFC_EXPECT_FALSE(err != 0)) {
		return err;
	}

	/* Verify the struct size. */
	ops = &sip->sti_pduops;
	if (PFC_EXPECT_FALSE(ops->ipops_size != size)) {
		IPC_LOG_ERROR("Bad size for struct %s: %u: required=%u",
			      name, size, ops->ipops_size);
		err = ENODEV;
		goto error;
	}

	/* Verify address alignment. */
	if (PFC_EXPECT_FALSE(!PFC_IS_POW2_ALIGNED(addr, ops->ipops_align))) {
		IPC_LOG_ERROR("Invalid alignment for struct %s: addr=%p, "
			      "align=%u", name, addr, ops->ipops_align);
		err = EFAULT;
		goto error;
	}

	/* Verify the layout signature. */
	diff = memcmp(sig, sip->sti_sig, sizeof(sip->sti_sig));
	if (PFC_EXPECT_FALSE(diff != 0)) {
		IPC_LOG_ERROR("Bad layout signature for struct %s.", name);
		err = ENODEV;
		goto error;
	}

	*sipp = sip;

	return 0;

error:
	IPC_STRINFO_RELEASE(sip);

	return err;
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipc_struct_fork_prepare(void)
 *	fork(2) handler which will be called just before fork(2) on parent
 *	process.
 */
void PFC_ATTR_HIDDEN
pfc_ipc_struct_fork_prepare(void)
{
	IPC_STRUCT_WRLOCK();
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipc_struct_fork_parent(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on parent process.
 */
void PFC_ATTR_HIDDEN
pfc_ipc_struct_fork_parent(void)
{
	IPC_STRUCT_UNLOCK();
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipc_struct_fork_child(void)
 *	fork(2) handler which will be called just before returning from fork(2)
 *	on child process.
 */
void PFC_ATTR_HIDDEN
pfc_ipc_struct_fork_child(void)
{
	PFC_ASSERT_INT(pfc_rwlock_init(&ipc_struct_lock), 0);
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipc_struct_fini(void)
 *	Free up loaded struct information.
 *	This function will be called via library destructor.
 */
void PFC_ATTR_HIDDEN
pfc_ipc_struct_fini(void)
{
	int	lock_err = IPC_STRUCT_TRYWRLOCK();

	if (lock_err == 0 && ipc_struct_refcnt == 0) {
		IPC_STRUCT_CLEANUP(&ipc_structs);
		IPC_STRUCT_UNLOCK();
	}
}

/*
 * void PFC_ATTR_HIDDEN
 * pfc_ipc_struct_bswap(ipc_cpduops_t *PFC_RESTRICT ops,
 *			ipc_bswap_t *PFC_RESTRICT bsp)
 *	Copy STRUCT PDU data with byte swapping.
 *
 *	The caller must initialize ipc_bswap_t fields properly, and must
 *	pass a pointer to ipc_bswap_t buffer to `bsp'.
 */
void PFC_ATTR_HIDDEN
pfc_ipc_struct_bswap(ipc_cpduops_t *PFC_RESTRICT ops,
		     ipc_bswap_t *PFC_RESTRICT bsp)
{
	/* Adjust buffer addresses to the boundary required by this struct. */
	pfc_ipc_bswap_align(bsp, ops->ipops_align);

	/* Decode serialized struct data with byte swapping. */
	ops->ipops_bswap(ops, bsp, 1);
}

/*
 * static int
 * ipc_struct_load(ipc_strload_t *ldctx)
 *	Load IPC struct information file.
 *
 *	The loader context specified to `ldctx' must be initialized by
 *	IPC_STRLOAD_INIT() in advance.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	- This function must be called with holding the IPC struct lock in
 *	  writer mode.
 *
 *	- The caller must destroy the loader context specified to `ldctx' by
 *	  calling ipc_strload_destroy().
 *
 *	- On error return, this function behaves as follows:
 *	  + On temporary error, such as ENOMEM, cleans up loaded struct
 *	    information, and returns without changing ipc_struct_bin_error.
 *	    So pfc_ipc_struct_get() will try to load struct information again
 *	    on next call.
 *	  + Otherwise, leave loaded struct information and set error number to
 *	    ipc_struct_bin_error. So pfc_ipc_struct_get() will always return
 *	    error number in ipc_struct_bin_error.
 */
static int
ipc_struct_load(ipc_strload_t *ldctx)
{
	ipc_ifhead_t	*head;
	ipc_ifstruct_t	*fsp, *first;
	uint32_t	nstructs, loaded;
	int		err;

	if (ldctx->sld_tree == &ipc_structs) {
		err = ipc_struct_bin_error;
		if (PFC_EXPECT_FALSE(err >= 0)) {
			/* Already loaded. */
			return err;
		}
	}
	else {
		/* Initialize error number with zero. */
		err = 0;
	}

	if ((head = ldctx->sld_head) == NULL) {
		/* Open and map IPC struct information file. */
		err = ipc_struct_getmap(ldctx);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto out;
		}

		head = ldctx->sld_head;
	}

	nstructs = head->ifh_nstructs;

	/* Load struct entries. */
	first = IPC_IFSTRUCT_FIRST(head);
	loaded = 0;
	for (fsp = first; fsp < first + nstructs; fsp++) {
		err = ipc_struct_load_entry(ldctx, fsp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			break;
		}
		loaded++;
	}

	IPC_LOG_INFO("%u IPC struct data %s been loaded.",
		     loaded, (loaded > 1) ? "have" : "has");

out:
	if (ldctx->sld_tree == &ipc_structs) {
		if (PFC_EXPECT_TRUE(err == 0 ||
				    (err != ENOMEM && err != EAGAIN))) {
			/*
			 * ipc_struct_bin_error must be updated on successful
			 * completion or fatal error.
			 */
			ipc_struct_bin_error = err;
		}
		else {
			/*
			 * This is a temporary error.
			 * Clean up loaded struct information for the next
			 * try.
			 */
			IPC_STRUCT_CLEANUP(ldctx->sld_tree);
		}
	}

	return err;
}

/*
 * static int
 * ipc_struct_getmap(ipc_strload_t *ldctx)
 *	Open IPC struct information file, and map it into the virtual memory.
 *
 * Calling/Exit State:
 *	Upon successful completion, a pointer to IPC struct information header
 *	is set into the buffer pointed by `ldctx', and then zero is returned.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipc_struct_getmap(ipc_strload_t *ldctx)
{
	ipc_ifhead_t	*head;
	uint8_t		order;
	uint32_t	off;
	uint64_t	fsize;
	struct stat	sbuf;
	const char	*path = ldctx->sld_path;
	char		*dpath;
	int		err, ret, fd;

	/* Ensure that IPC struct information file is safe. */
	dpath = strdup(path);
	if (PFC_EXPECT_FALSE(dpath == NULL)) {
		IPC_LOG_ERROR("No memory for path to struct info file.");

		return ENOMEM;
	}

	err = pfc_is_safepath(dpath);
	free(dpath);
	if (PFC_EXPECT_FALSE(err != 0)) {
		IPC_LOG_ERROR("%s: struct info file is unsafe: %s",
			      path, strerror(err));

		return err;
	}

	/* Open IPC struct information file. */
	fd = pfc_open_cloexec(path, O_RDONLY);
	if (PFC_EXPECT_FALSE(fd == -1)) {
		err = errno;
		IPC_LOG_ERROR("%s: open() failed: %s", path, strerror(err));
		goto error;
	}

	/* Determine the file size. */
	ret = fstat(fd, &sbuf);
	if (PFC_EXPECT_FALSE(ret != 0)) {
		err = errno;
		IPC_LOG_ERROR("%s: stat() failed: %s", path, strerror(err));
		goto error;
	}

	fsize = (uint64_t)sbuf.st_size;
	if (PFC_EXPECT_FALSE(fsize < sizeof(*head))) {
		IPC_LOG_ERROR("%s: Too small: %" PFC_PFMT_u64, path, fsize);
		err = ENODATA;
		goto error;
	}

	if (PFC_EXPECT_FALSE(fsize >= IPC_STRINFO_MAXSIZE)) {
		IPC_LOG_ERROR("%s: Too large: %" PFC_PFMT_u64, path, fsize);
		err = E2BIG;
		goto error;
	}

	/* Map whole information file to the virtual memory. */
	head = (ipc_ifhead_t *)mmap(NULL, sbuf.st_size, PROT_READ | PROT_WRITE,
				    MAP_PRIVATE, fd, 0);
	if (PFC_EXPECT_FALSE(head == (ipc_ifhead_t *)MAP_FAILED)) {
		err = errno;
		IPC_LOG_ERROR("%s: mmap() failed: %s", path, strerror(err));
		goto error;
	}
	PFC_IPC_CLOSE(fd);
	fd = -1;

	/* Verify contents of the header. */
	if (PFC_EXPECT_FALSE(head->ifh_magic != IPC_STRINFO_MAGIC)) {
		IPC_LOG_ERROR("Bad magic in struct info: 0x%02x",
			      head->ifh_magic);
		goto error_unmap;
	}

	if (PFC_EXPECT_FALSE(head->ifh_version != IPC_STRINFO_VERSION)) {
		IPC_LOG_ERROR("Unexpected struct info version: %u",
			      head->ifh_version);
		goto error_unmap;
	}

	order = head->ifh_order;
	if (PFC_EXPECT_FALSE(!IPC_ORDER_IS_VALID(order))) {
		IPC_LOG_ERROR("Invalid byte order of struct info: %u",
			      order);
		goto error_unmap;
	}

	if (IPC_IFHEAD_NEED_BSWAP(head)) {
		/* Convert byte order to host byte order. */
		IPC_BSWAP(head->ifh_nstructs);
		IPC_BSWAP(head->ifh_namespace);
		IPC_BSWAP(head->ifh_fldoff);
		IPC_BSWAP(head->ifh_fldsize);
		IPC_BSWAP(head->ifh_stroff);
		IPC_BSWAP(head->ifh_strsize);
	}

	/* Verify section offsets. */
	off = sizeof(*head) + (sizeof(ipc_ifstruct_t) * head->ifh_nstructs);
	if (PFC_EXPECT_FALSE(off != head->ifh_fldoff)) {
		IPC_LOG_ERROR("Field section offset must be %u, but %u.",
			      off, head->ifh_fldoff);
		goto error_unmap;
	}

	if (PFC_EXPECT_FALSE((head->ifh_fldsize % sizeof(ipc_iffield_t))
			     != 0)) {
		IPC_LOG_ERROR("Invalid field section size: %u",
			      head->ifh_fldsize);
		goto error_unmap;
	}

	off += head->ifh_fldsize;
	if (PFC_EXPECT_FALSE(off != head->ifh_stroff)) {
		IPC_LOG_ERROR("String table section offset must be %u, "
			      "but %u.", off, head->ifh_stroff);
		goto error_unmap;
	}

	if (PFC_EXPECT_FALSE(head->ifh_strsize == 0)) {
		IPC_LOG_ERROR("Invalid string table section size: %u",
			      head->ifh_strsize);
		goto error_unmap;
	}

	off += head->ifh_strsize;
	if (PFC_EXPECT_FALSE(off != fsize)) {
		IPC_LOG_ERROR("Info file size must be %u, but %u",
			      off, (uint32_t)sbuf.st_size);
		goto error_unmap;
	}

	ldctx->sld_head = head;
	ldctx->sld_size = (uint32_t)sbuf.st_size;

	if (IPC_IFHEAD_NEED_BSWAP(head)) {
		ipc_ifstruct_t	*fsp, *fslimit;
		ipc_iffield_t	*ffp, *fflimit;
		uint32_t	nfields;

		/* Convert byte order of all sections. */
		fsp = IPC_IFSTRUCT_FIRST(head);
		fslimit = fsp + head->ifh_nstructs;
		for (; fsp < fslimit; fsp++) {
			IPC_BSWAP(fsp->ifs_name);
			IPC_BSWAP(fsp->ifs_nfields);
			IPC_BSWAP(fsp->ifs_size);
			IPC_BSWAP(fsp->ifs_align);
			IPC_BSWAP(fsp->ifs_field);
		}

		nfields = head->ifh_fldsize / sizeof(ipc_iffield_t);
		ffp = IPC_IFFIELD_PTR(head, 0);
		fflimit = ffp + nfields;
		for (; ffp < fflimit; ffp++) {
			IPC_BSWAP(ffp->iff_name);
			IPC_BSWAP(ffp->iff_array);
			IPC_BSWAP(ffp->iff_type);
		}
	}

	return 0;

error_unmap:
	(void)munmap(head, sbuf.st_size);

error:
	if (fd != -1) {
		PFC_IPC_CLOSE(fd);
	}

	if (PFC_EXPECT_FALSE(err == 0)) {
		err = EPROTO;
	}

	return err;
}

/*
 * static int
 * ipc_struct_load_entry(ipc_strload_t *PFC_RESTRICT ldctx,
 *			 ipc_ifstruct_t *PFC_RESTRICT fsp)
 *	Load IPC struct information specified by `fsp'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipc_struct_load_entry(ipc_strload_t *PFC_RESTRICT ldctx,
		      ipc_ifstruct_t *PFC_RESTRICT fsp)
{
	const char	*name;
	uint32_t	fstart, fend, nfields, size;
	pfc_rbtree_t	*tree = ldctx->sld_tree;
	ipc_pduops_t	*ops;
	ipc_ifhead_t	*head = ldctx->sld_head;
	ipc_strinfo_t	*sip;
	ipc_fldinfo_t	*fldinfo, *flp;
	ipc_fldload_t	load;
	int		err = 0;

	/* Verify struct information. */
	name = ipc_struct_string(head, fsp->ifs_name);
	if (PFC_EXPECT_FALSE(name == NULL)) {
		IPC_LOG_ERROR("Invalid struct name index: %u", fsp->ifs_name);

		return EPROTO;
	}

	fstart = fsp->ifs_field;
	nfields = fsp->ifs_nfields;
	if (PFC_EXPECT_FALSE(nfields == 0)) {
		IPC_LOG_ERROR("%s: No field is defined.", name);

		return EPROTO;
	}

	fend = fstart + nfields;
	if (PFC_EXPECT_FALSE(fend > (head->ifh_fldsize / sizeof(*flp)))) {
		IPC_LOG_ERROR("%s: Field index is out of range: "
			      "start=%u, end=%u, limit=%u",
			      name, fstart, fend,
			      (uint32_t)(head->ifh_fldsize / sizeof(*flp)));

		return EPROTO;
	}

	if (PFC_EXPECT_FALSE(!IPC_STRUCT_SIZE_IS_VALID(fsp->ifs_size))) {
		IPC_LOG_ERROR("%s: Invalid struct size: %u",
			      name, fsp->ifs_size);

		return EPROTO;
	}

	if (PFC_EXPECT_FALSE(!IPC_STRUCT_ALIGN_IS_VALID(fsp->ifs_align))) {
		IPC_LOG_ERROR("%s: Invalid alignment: %u",
			      name, fsp->ifs_align);

		return EPROTO;
	}

	/* Allocate buffer for struct information. */
	sip = (ipc_strinfo_t *)malloc(sizeof(*sip));
	if (PFC_EXPECT_FALSE(sip == NULL)) {
		IPC_LOG_ERROR("%s: No memory for struct information.", name);

		return ENOMEM;
	}

	sip->sti_name = pfc_refptr_string_create(name);
	if (PFC_EXPECT_FALSE(sip->sti_name == NULL)) {
		IPC_LOG_ERROR("%s: No memory for struct name.", name);
		err = ENOMEM;
		goto error;
	}

	if (PFC_EXPECT_FALSE(IPC_STRINFO_NAMELEN(sip) >=
			     IPC_STRTYPE_MAX_NAMELEN)) {
		IPC_LOG_ERROR("%s: Too long struct name.", name);
		goto error_name;
	}

	fldinfo = (ipc_fldinfo_t *)malloc(sizeof(*fldinfo) * nfields);
	if (PFC_EXPECT_FALSE(fldinfo == NULL)) {
		IPC_LOG_ERROR("%s: No memory for field information.", name);
		err = ENOMEM;
		goto error_name;
	}

	sip->sti_nfields = nfields;
	sip->sti_field = fldinfo;
	pfc_rbtree_init(&sip->sti_fldmeta, (pfc_rbcomp_t)strcmp,
			ipc_fldmeta_getkey);
	memcpy(sip->sti_sig, fsp->ifs_sig, sizeof(sip->sti_sig));

	ops = &sip->sti_pduops;
	pfc_ipc_pdu_setstruct(ops);
	ops->ipops_size = fsp->ifs_size;
	ops->ipops_align = fsp->ifs_align;

	/* Fetch field information. */
	IPC_FLDLOAD_INIT(&load, sip, fldinfo, IPC_IFFIELD_PTR(head, fstart));
	for (; nfields > 0; nfields--) {
		err = ipc_struct_load_field(ldctx, &load);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto error_fldinfo;
		}
	}

	/* Verify struct size. */
	size = PFC_POW2_ROUNDUP(load.fld_size, ops->ipops_align);
	if (PFC_EXPECT_FALSE(size != ops->ipops_size)) {
		IPC_LOG_ERROR("%s: Unexpected struct size: %u: required=%u",
			      name, ops->ipops_size, size);
		goto error_fldinfo;
	}

	/* Register struct information. */
	err = pfc_rbtree_put(tree, &sip->sti_node);
	if (tree != &ipc_structs && err == 0) {
		pfc_rbnode_t	*node = pfc_rbtree_get(&ipc_structs, name);

		if (PFC_EXPECT_FALSE(node != NULL)) {
			err = EEXIST;
		}
	}
	if (PFC_EXPECT_FALSE(err != 0)) {
		PFC_ASSERT(err == EEXIST);
		IPC_LOG_ERROR("%s: Struct name is duplicated.", name);
		goto error_fldinfo;
	}

	IPC_LOG_VERBOSE("IPC struct \"%s\" is loaded: size=%u, align=%u",
			name, ops->ipops_size, ops->ipops_align);

	return 0;

error_fldinfo:
	free(fldinfo);

error_name:
	pfc_refptr_put(sip->sti_name);

error:
	free(sip);

	if (PFC_EXPECT_FALSE(err == 0)) {
		err = EPROTO;
	}

	return err;
}

/*
 * static int
 * ipc_struct_load_field(ipc_strload_t *PFC_RESTRICT ldctx,
 *			 ipc_fldload_t *PFC_RESTRICT loadp)
 *	Load IPC struct field information.
 *
 *	The caller must initialize ipc_fldload_t by IPC_FLDLOAD_INIT()
 *	before the first call.
 *
 * Calling/Exit State:
 *	Upon successful completion, field information specified by
 *	`loadp->fld_field' into `*(loadp->fld_fldinfo)', and update load
 *	context for the next call, and then zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 */
static int
ipc_struct_load_field(ipc_strload_t *PFC_RESTRICT ldctx,
		      ipc_fldload_t *PFC_RESTRICT loadp)
{
	ipc_iffield_t	*ffp = loadp->fld_field;
	ipc_fldinfo_t	*flp = loadp->fld_fldinfo;
	ipc_cpduops_t	*pops;
	uint32_t	type, size, fldsize, align, array;

	size = loadp->fld_size;
	type = ffp->iff_type;
	if (type & IPC_IFFIELD_STRUCT) {
		const char	*stname;
		ipc_strinfo_t	*fsip;
		pfc_rbnode_t	*node;

		/*
		 * Type of this field is another struct.
		 * iff_type must keep index of struct name.
		 */
		type &= ~IPC_IFFIELD_STRUCT;
		stname = ipc_struct_string(ldctx->sld_head, type);
		if (PFC_EXPECT_FALSE(stname == NULL)) {
			IPC_LOG_ERROR("%s: Invalid struct name index in field"
				      ": %u",
				      IPC_STRINFO_NAME(loadp->fld_strinfo),
				      type);

			return EPROTO;
		}

		/* This struct information must be already registered. */
		node = pfc_rbtree_get(ldctx->sld_tree, stname);
		if (PFC_EXPECT_FALSE(node == NULL)) {
			IPC_LOG_ERROR("%s: Unknown struct name in field: "
				      "%s(%u)",
				      IPC_STRINFO_NAME(loadp->fld_strinfo),
				      stname, type);

			return EPROTO;
		}

		fsip = IPC_STRINFO_NODE2PTR(node);
		pops = &fsip->sti_pduops;
	}
	else {
		/* This is primitive IPC PDU type. */
		pops = pfc_ipc_pdu_getops((pfc_ipctype_t)type);
		if (PFC_EXPECT_FALSE(pops == NULL || pops->ipops_align == 0)) {
			IPC_LOG_ERROR("%s: Invalid field type: %u",
				      IPC_STRINFO_NAME(loadp->fld_strinfo),
				      type);

			return EPROTO;
		}

		PFC_ASSERT(pops->ipops_size != 0);
	}

	array = ffp->iff_array;
	align = pops->ipops_align;
	fldsize = pops->ipops_size;
	PFC_ASSERT(PFC_IS_POW2(align));

	if (PFC_EXPECT_FALSE(array > IPC_STRUCT_MAXSIZE)) {
		IPC_LOG_ERROR("%s: Invalid array size: %u",
			      IPC_STRINFO_NAME(loadp->fld_strinfo), array);

		return EPROTO;
	}

	if (array != 0) {
		fldsize *= array;
		if (PFC_EXPECT_FALSE(fldsize > IPC_STRUCT_MAXSIZE)) {
			IPC_LOG_ERROR("%s: Too large field size: %u",
				      IPC_STRINFO_NAME(loadp->fld_strinfo),
				      array);

			return EPROTO;
		}
	}

	/* Update struct size. */
	size = PFC_POW2_ROUNDUP(size, align) + fldsize;
	if (PFC_EXPECT_FALSE(size > IPC_STRUCT_MAXSIZE)) {
		IPC_LOG_ERROR("%s: Too large struct size: %u",
			      IPC_STRINFO_NAME(loadp->fld_strinfo), size);

		return EPROTO;
	}

	loadp->fld_size = size;
	loadp->fld_field = ffp + 1;
	loadp->fld_fldinfo = flp + 1;

	flp->fli_pduops = pops;
	flp->fli_array = array;

	return 0;
}

/*
 * static int
 * ipc_fldmeta_load(ipc_strload_t *ldctx)
 *	Load meta data of IPC struct fields from the IPC struct information
 *	file only if it is not yet loaded.
 *
 *	The loader context specified to `ldctx' must be initialized by
 *	IPC_STRLOAD_INIT() in advance.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the IPC struct lock in
 *	writer mode.
 */
static int
ipc_fldmeta_load(ipc_strload_t *ldctx)
{
	ipc_ifhead_t	*head;
	ipc_ifstruct_t	*fsp, *first;
	uint32_t	nstructs, loaded;
	int		err;

	if (ldctx->sld_tree == &ipc_structs) {
		err = ipc_fldmeta_error;
		if (PFC_EXPECT_FALSE(err >= 0)) {
			/* Already loaded. */
			return err;
		}
	}
	else {
		/* Initialize error number with zero. */
		err = 0;
	}

	if ((head = ldctx->sld_head) == NULL) {
		/* Open and map IPC struct information file. */
		err = ipc_struct_getmap(ldctx);
		if (PFC_EXPECT_FALSE(err != 0)) {
			goto out;
		}

		head = ldctx->sld_head;
	}

	/* At first, load all struct information. */
	err = ipc_struct_load(ldctx);
	if (PFC_EXPECT_FALSE(err != 0)) {
		goto out;
	}

	head = ldctx->sld_head;
	nstructs = head->ifh_nstructs;

	/* Iterate struct entries. */
	first = IPC_IFSTRUCT_FIRST(head);
	loaded = 0;
	for (fsp = first; fsp < first + nstructs; fsp++) {
		err = ipc_fldmeta_load_struct(ldctx, fsp, &loaded);
		if (PFC_EXPECT_FALSE(err != 0)) {
			break;
		}
	}

	IPC_LOG_INFO("%u IPC struct field data %s been loaded.",
		     loaded, (loaded > 1) ? "have" : "has");

out:
	if (ldctx->sld_tree == &ipc_structs) {
		if (PFC_EXPECT_TRUE(err == 0 ||
				    (err != ENOMEM && err != EAGAIN))) {
			/*
			 * ipc_fldmeta_error must be updated on successful
			 * completion* or fatal error.
			 */
			ipc_fldmeta_error = err;
		}
		else {
			pfc_rbtree_t	*tree = ldctx->sld_tree;
			pfc_rbnode_t	*node = NULL;

			/*
			 * This is a temporary error.
			 * Clean up loaded meta data for the next try.
			 */
			while ((node = pfc_rbtree_next(tree, node)) != NULL) {
				ipc_strinfo_t	*sip =
					IPC_STRINFO_NODE2PTR(node);

				IPC_FLDMETA_CLEANUP(sip);
			}
		}
	}

	return err;
}

/*
 * static int
 * ipc_fldmeta_load_struct(ipc_strload_t *PFC_RESTRICT ldctx,
 *			   ipc_ifstruct_t *PFC_RESTRICT fsp, uint32_t *loaded)
 *	Load field meta data of the IPC struct specified by `fsp'.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	The number of loaded fields are added to value in the buffer
 *	pointed by `loaded'.
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the IPC struct lock in
 *	writer mode.
 */
static int
ipc_fldmeta_load_struct(ipc_strload_t *PFC_RESTRICT ldctx,
			ipc_ifstruct_t *PFC_RESTRICT fsp, uint32_t *loaded)
{
	const char	*name;
	pfc_rbnode_t	*node;
	ipc_ifhead_t	*head = ldctx->sld_head;
	ipc_strinfo_t	*sip;
	ipc_cfldinfo_t	*flp;
	ipc_iffield_t	*ffp;
	uint32_t	fstart, nfields, offset, index;
	int		err;

	/*
	 * Remarks:
	 *	No need to verify contents of IPC struct information file
	 *	because it is already done by ipc_struct_load().
	 */

	/* Determine the struct name. */
	name = ipc_struct_string(head, fsp->ifs_name);
	PFC_ASSERT(name != NULL);

	/* Obtain struct information. */
	node = pfc_rbtree_get(ldctx->sld_tree, name);
	PFC_ASSERT(node != NULL);
	sip = IPC_STRINFO_NODE2PTR(node);

	fstart = fsp->ifs_field;
	nfields = fsp->ifs_nfields;
	PFC_ASSERT(nfields == sip->sti_nfields);
	PFC_ASSERT(fstart + nfields <= (head->ifh_fldsize / sizeof(*flp)));

	ffp = IPC_IFFIELD_PTR(head, fstart);
	offset = 0;
	for (flp = sip->sti_field, index = 0; index < nfields;
	     index++, flp++, ffp++) {
		ipc_cpduops_t	*ops = flp->fli_pduops;
		uint32_t	size = ops->ipops_size;

		PFC_ASSERT(flp->fli_array == ffp->iff_array);

		/* Adjust offset to data type alignment. */
		offset = PFC_POW2_ROUNDUP(offset, ops->ipops_align);
		PFC_ASSERT(offset + size <= fsp->ifs_size);

		/* Load field meta data. */
		err = ipc_fldmeta_load_field(ldctx, sip, index, offset, size,
					     ffp);
		if (PFC_EXPECT_FALSE(err != 0)) {
			IPC_FLDMETA_CLEANUP(sip);

			return err;
		}

		if (flp->fli_array != 0) {
			size *= flp->fli_array;
		}
		offset += size;
	}

	PFC_ASSERT(PFC_POW2_ROUNDUP(offset, fsp->ifs_align) == fsp->ifs_size);

	*loaded += fsp->ifs_nfields;

	return 0;
}

/*
 * static int
 * ipc_fldmeta_load_field(ipc_strload_t *PFC_RESTRICT ldctx,
 *			  ipc_strinfo_t *PFC_RESTRICT sip, uint32_t index,
 *			  uint32_t offset, uint32_t size,
 *			  ipc_iffield_t *PFC_RESTRICT ffp)
 *	Load the meta data of the specified IPC struct field.
 *
 * Calling/Exit State:
 *	Upon successful completion, zero is returned.
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the IPC struct lock in
 *	writer mode.
 */
static int
ipc_fldmeta_load_field(ipc_strload_t *PFC_RESTRICT ldctx,
		       ipc_strinfo_t *PFC_RESTRICT sip, uint32_t index,
		       uint32_t offset, uint32_t size,
		       ipc_iffield_t *PFC_RESTRICT ffp)
{
	const char	*name;
	ipc_fldmeta_t	*fmp;
	ipc_ifhead_t	*head = ldctx->sld_head;
	uint32_t	type;
	int		err;

	/* Determine the name of this field. */
	name = ipc_struct_string(head, ffp->iff_name);
	if (PFC_EXPECT_FALSE(name == NULL)) {
		IPC_LOG_ERROR("Invalid field name index in \"%s\": %u",
			      IPC_STRINFO_NAME(sip), ffp->iff_name);

		return EPROTO;
	}

	/* Allocate a new meta data. */
	fmp = (ipc_fldmeta_t *)malloc(sizeof(*fmp));
	if (PFC_EXPECT_FALSE(fmp == NULL)) {
		IPC_LOG_ERROR("No memory for field meta data: %s.%s",
			      IPC_STRINFO_NAME(sip), name);

		return ENOMEM;
	}

	fmp->flm_name = pfc_refptr_string_create(name);
	if (PFC_EXPECT_FALSE(fmp->flm_name == NULL)) {
		IPC_LOG_ERROR("No memory for field name: %s.%s",
			      IPC_STRINFO_NAME(sip), name);
		err = ENOMEM;
		goto error;
	}

	fmp->flm_index = index;
	fmp->flm_size = size;
	fmp->flm_offset = offset;
	fmp->flm_flags = ffp->iff_array;

	type = ffp->iff_type;
	if (type & IPC_IFFIELD_STRUCT) {
		const char	*stname;
		pfc_rbnode_t	*node;

		/* Resolve struct information of this field type. */
		type &= ~IPC_IFFIELD_STRUCT;
		stname = ipc_struct_string(head, type);
		PFC_ASSERT(stname != NULL);

		node = pfc_rbtree_get(ldctx->sld_tree, stname);
		PFC_ASSERT(node != NULL);
		IPC_FLDMETA_STRINFO(fmp) = IPC_STRINFO_NODE2PTR(node);
		fmp->flm_flags |= IPC_FLMF_STRUCT;

#ifdef	PFC_VERBOSE_DEBUG
		if (ffp->iff_array == 0) {
			IPC_LOG_VERBOSE("Field: %s.%s: type=%s, off=%u",
					IPC_STRINFO_NAME(sip), name,
					stname, offset);
		}
		else {
			IPC_LOG_VERBOSE("Field: %s.%s: type=%s[%u], off=%u",
					IPC_STRINFO_NAME(sip), name,
					stname, ffp->iff_array, offset);
		}
#endif	/* PFC_VERBOSE_DEBUG */
	}
	else {
		IPC_FLDMETA_TYPE(fmp) = (pfc_ipctype_t)type;

#ifdef	PFC_VERBOSE_DEBUG
		if (ffp->iff_array == 0) {
			IPC_LOG_VERBOSE("Field: %s.%s: type=%u, off=%u",
					IPC_STRINFO_NAME(sip), name, type,
					offset);
		}
		else {
			IPC_LOG_VERBOSE("Field: %s.%s: type=%u[%u], off=%u",
					IPC_STRINFO_NAME(sip), name, type,
					ffp->iff_array, offset);
		}
#endif	/* PFC_VERBOSE_DEBUG */
	}

	/* Register field meta data. */
	err = pfc_rbtree_put(&sip->sti_fldmeta, &fmp->flm_node);
	if (PFC_EXPECT_FALSE(err != 0)) {
		PFC_ASSERT(err == EEXIST);
		IPC_LOG_ERROR("%s.%s: Field name is duplicated.",
			      IPC_STRINFO_NAME(sip), name);
		goto error_name;
	}

	return 0;

error_name:
	pfc_refptr_put(fmp->flm_name);

error:
	free(fmp);

	return err;
}

/*
 * static int
 * ipc_strinfo_lookup(const char *PFC_RESTRICT name,
 *		      ipc_strinfo_t **PFC_RESTRICT sipp)
 *	Search for the IPC struct information specified by the struct name
 *	`name'.
 *
 * Calling/Exit State:
 *	Upon successful completion, A non-NULL pointer to IPC struct
 *	information is set to the buffer pointed by `sipp', and zero is
 *	returned. Note that `*sipp' must be released by IPC_STRINFO_RELEASE().
 *
 *	Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *	This function must be called with holding the IPC struct lock in
 *	reader or writer mode. Node that this function may upgrade the lock
 *	to writer mode.
 */
static int
ipc_strinfo_lookup(const char *PFC_RESTRICT name,
		   ipc_strinfo_t **PFC_RESTRICT sipp)
{
	pfc_rbnode_t	*node;
	ipc_strinfo_t	*sip;

	if (PFC_EXPECT_FALSE(ipc_struct_bin_error < 0)) {
		ipc_strload_t	ldctx;
		int		err;

		/* Load IPC struct information file. */
		IPC_STRUCT_UNLOCK();
		IPC_STRLOAD_INIT(&ldctx, &ipc_structs, IPC_STRUCT_BIN_PATH);
		IPC_STRUCT_WRLOCK();
		err = ipc_struct_load(&ldctx);
		if (PFC_EXPECT_FALSE(err != 0)) {
			IPC_STRUCT_UNLOCK();

			return err;
		}
		ipc_strload_destroy(&ldctx);
	}

	/* Search for a struct information associated with the given name. */
	node = pfc_rbtree_get(&ipc_structs, name);
	if (PFC_EXPECT_FALSE(node == NULL)) {
		IPC_LOG_ERROR("Unknown struct name: %s", name);

		return ENODEV;
	}

	sip = IPC_STRINFO_NODE2PTR(node);
	IPC_STRINFO_HOLD(sip);
	*sipp = sip;

	return 0;
}

/*
 * static const char *
 * ipc_struct_string(ipc_ifhead_t *head, uint32_t index)
 *	Return a pointer to a string in the string table section at the
 *	given index.
 *
 *	NULL is returned if the given index is invalid.
 */
static const char *
ipc_struct_string(ipc_ifhead_t *head, uint32_t index)
{
	if (PFC_EXPECT_FALSE(index == 0 || index >= head->ifh_strsize)) {
		return NULL;
	}

	return IPC_IFHEAD_STRTABLE(head) + index;
}

/*
 * static pfc_cptr_t
 * ipc_strinfo_getkey(pfc_rbnode_t *node)
 *	Return the type name of IPC struct information node as node key.
 *	`node' must be a pointer to sti_node in ipc_strinfo_t.
 */
static pfc_cptr_t
ipc_strinfo_getkey(pfc_rbnode_t *node)
{
	ipc_strinfo_t	*sip = IPC_STRINFO_NODE2PTR(node);

	return IPC_STRINFO_NAME(sip);
}

/*
 * static pfc_cptr_t
 * ipc_fldmeta_getkey(pfc_rbnode_t *node)
 *	Return the key of the specified struct field meta data.
 *	`node' must be a pointer to flm_node in ipc_fldmeta_t.
 */
static pfc_cptr_t
ipc_fldmeta_getkey(pfc_rbnode_t *node)
{
	ipc_fldmeta_t	*fmp = IPC_FLDMETA_NODE2PTR(node);

	return IPC_FLDMETA_NAME(fmp);
}

/*
 * static void
 * ipc_strinfo_dtor(pfc_rbnode_t *node, pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	Destructor of struct information.
 *
 *	This function is called by library destructor via pfc_rbtree_clear().
 */
static void
ipc_strinfo_dtor(pfc_rbnode_t *node, pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	ipc_strinfo_t	*sip = IPC_STRINFO_NODE2PTR(node);

	pfc_refptr_put(sip->sti_name);
	IPC_FLDMETA_CLEANUP(sip);
	free((void *)sip->sti_field);
	free(sip);
}

/*
 * static void
 * ipc_fldmeta_dtor(pfc_rbnode_t *node, pfc_ptr_t arg PFC_ATTR_UNUSED)
 *	Destructor of struct field meta data.
 *
 *	This function is called by library destructor via pfc_rbtree_clear().
 */
static void
ipc_fldmeta_dtor(pfc_rbnode_t *node, pfc_ptr_t arg PFC_ATTR_UNUSED)
{
	ipc_fldmeta_t	*fmp = IPC_FLDMETA_NODE2PTR(node);

	pfc_refptr_put(fmp->flm_name);
	free(fmp);
}

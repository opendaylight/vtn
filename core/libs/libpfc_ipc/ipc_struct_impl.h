/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFC_IPC_IPC_STRUCT_IMPL_H
#define	_PFC_LIBPFC_IPC_IPC_STRUCT_IMPL_H

/*
 * Definitions for IPC struct information.
 */

#include <pfc/base.h>
#include <pfc/refptr.h>
#include <pfc/rbtree.h>
#include <pfc/atomic.h>
#include "ipc_impl.h"

/*
 * Format of IPC struct information file:
 *
 *    +----------------------+ 0
 *    |     ipc_ifhead_t     |
 *    +----------------------+ 32
 *    |                      |
 *    |    Struct Section    |
 *    |                      |
 *    +----------------------+ ifh_fldoff
 *    |                      |
 *    |     Field Section    |
 *    |                      |
 *    +----------------------+ ifh_stroff
 *    |                      |
 *    | String Table Section |
 *    |                      |
 *    +----------------------+ ifh_stroff + ifh_strsize
 *
 * - Struct Section
 *
 *   Struct Section is an array of ipc_ifstruct_t.
 *   Each IPC data struct is associated with an ipc_ifstruct_t record.
 *
 * - Field Section
 *
 *   Field Section is an array of ipc_iffield_t.
 *   One ipc_iffield_t record represents a field in IPC data struct.
 *   A sequence of ipc_iffield_t is assigned for each ipc_ifstruct_t record
 *   in Struct Section.
 *
 * - String Table Section
 *
 *   String Table Section keeps all strings used by IPC struct information.
 *   Its format is similar to ELF string table.
 *
 * Remarks:
 *   If you want to change this header file, you may need to modify the
 *   IPC struct template compiler.
 */

/*
 * Magic number in IPC struct information file header.
 */
#define	IPC_STRINFO_MAGIC		PFC_CONST_U(0xac)

/*
 * Current version of IPC struct information file format.
 */
#define	IPC_STRINFO_VERSION		PFC_CONST_U(1)

/*
 * Maximum size of IPC struct information file. (exclusive)
 */
#define	IPC_STRINFO_MAXSIZE		PFC_CONST_U(0x80000000)	/* 2G */

/*
 * Header of IPC struct information file.
 *
 * Remarks:
 *	Size of ipc_ifhead_t must be aligned to 8 bytes.
 */
typedef struct {
	uint8_t		ifh_magic;	/* magic number */
	uint8_t		ifh_version;	/* format version */
	uint8_t		ifh_order;	/* byte order (IPC_ORDER_XXX) */
	uint8_t		ifh_resv1;	/* reserved for future expansion */
	uint32_t	ifh_resv2;	/* reserved for future expansion */
	uint32_t	ifh_nstructs;	/* number of struct */
	uint32_t	ifh_namespace;	/* string table offset to namespace */
	uint32_t	ifh_fldoff;	/* offset to field section */
	uint32_t	ifh_fldsize;	/* size of field section */
	uint32_t	ifh_stroff;	/* offset to string table section */
	uint32_t	ifh_strsize;	/* size of string table section */
} ipc_ifhead_t;

PFC_TYPE_SIZE_ASSERT(ipc_ifhead_t, 32);

/*
 * Macro to determine whether byte swapping is needed or not.
 */
#define	IPC_IFHEAD_NEED_BSWAP(head)				\
	PFC_EXPECT_FALSE((head)->ifh_order != IPC_ORDER_NATIVE)

/*
 * Return the base address of the string table section.
 */
#define	IPC_IFHEAD_STRTABLE(head)			\
	((const char *)(head) + (head)->ifh_stroff)

/*
 * Length of struct signature string.
 */
#define	IPC_STRUCT_SIG_SIZE		PFC_CONST_U(64)		/* SHA-256 */

/*
 * Struct section record.
 *
 * Remarks:
 *	Size of ipc_ifstruct_t must be aligned to 8 bytes.
 */
typedef struct {
	uint32_t	ifs_name;	/* string table offset to type name */
	uint32_t	ifs_nfields;	/* number of fields */
	uint32_t	ifs_size;	/* size of struct */
	uint32_t	ifs_align;	/* alignment required by this struct */
	uint32_t	ifs_field;	/* start index of field section */
	uint32_t	ifs_resv;	/* reserved for future expansion */

	/* Signature of struct layout */
	const char	ifs_sig[IPC_STRUCT_SIG_SIZE];
} ipc_ifstruct_t;

PFC_TYPE_SIZE_ASSERT(ipc_ifstruct_t, 88);

/*
 * Maximum size of IPC struct.
 */
#define	IPC_STRUCT_MAXSIZE		PFC_CONST_U(0x100000)	/* 1M */

/*
 * Determine whether the given struct size is valid or not.
 */
#define	IPC_STRUCT_SIZE_IS_VALID(size)			\
	((size) != 0 && size <= IPC_STRUCT_MAXSIZE)

/*
 * Maximum value of alignment.
 */
#define	IPC_STRUCT_MAXALIGN		PFC_CONST_U(8)

/*
 * Determine whether the given address alignment is valid or not.
 */
#define	IPC_STRUCT_ALIGN_IS_VALID(align)			\
	((align) != 0 && (align) <= IPC_STRUCT_MAXALIGN &&	\
	 PFC_IS_POW2(align))

/*
 * Return a pointer to the first ipc_ifstruct_t entry.
 * `head' must be a pointer to ipc_ifhead_t.
 */
#define	IPC_IFSTRUCT_FIRST(head)	((ipc_ifstruct_t *)((head) + 1))

/*
 * Field section record.
 *
 * Remarks:
 *	Size of ipc_iffield_t must be aligned to 8 bytes.
 */
typedef struct {
	uint32_t	iff_name;	/* string table offset to field name */
	uint32_t	iff_array;	/* number of array elements */
	uint32_t	iff_type;	/* data type (PFC_IPCTYPE_XXX) */
	uint32_t	iff_resv;	/* reserved for future expansion */
} ipc_iffield_t;

PFC_TYPE_SIZE_ASSERT(ipc_iffield_t, 16);

/*
 * Return a pointer to field section entry at the given index.
 */
#define	IPC_IFFIELD_PTR(head, index)					\
	((ipc_iffield_t *)((uint8_t *)(head) + (head)->ifh_fldoff) + (index))

/*
 * IPC_IFFIELD_STRUCT means that the type of this field is other struct.
 * If IPC_IFFIELD_STRUCT is set in iff_type, iff_type value without this bit
 * represents the offset to string table which points the name of struct name.
 */
#define	IPC_IFFIELD_STRUCT		PFC_CONST_U(0x80000000)

/*
 * Maximum name length of structure data type. (including NULL byte)
 */
#define	IPC_STRTYPE_MAX_NAMELEN		PFC_CONST_U(64)

struct ipc_fldinfo;
typedef struct ipc_fldinfo	ipc_fldinfo_t;
typedef const ipc_fldinfo_t	ipc_cfldinfo_t;

/*
 * Struct information to be loaded into memory.
 */
struct ipc_strinfo {
	pfc_refptr_t	*sti_name;	/* type name */
	ipc_pduops_t	sti_pduops;	/* PDU operations for this struct */
	ipc_cfldinfo_t	*sti_field;	/* struct field information */
	pfc_rbtree_t	sti_fldmeta;	/* meta data of struct fields */
	pfc_rbnode_t	sti_node;	/* Red-Black tree node */
	uint32_t	sti_nfields;	/* number of fields */

	/* Signature of struct layout */
	char		sti_sig[IPC_STRUCT_SIG_SIZE];
};

#ifndef	__PFC_IPC_STRINFO_DEFINED
#define	__PFC_IPC_STRINFO_DEFINED
typedef struct ipc_strinfo	ipc_strinfo_t;
typedef const ipc_strinfo_t	ipc_cstrinfo_t;
#endif	/* !__PFC_IPC_STRINFO_DEFINED */

#define	IPC_STRINFO_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), ipc_strinfo_t, sti_node)
#define	IPC_STRINFO_PDUOPS2PTR(ops)				\
	PFC_CAST_CONTAINER((ops), ipc_strinfo_t, sti_pduops)

#define	IPC_STRINFO_NAME(sip)	pfc_refptr_string_value((sip)->sti_name)
#define	IPC_STRINFO_NAMELEN(sip)			\
	pfc_refptr_string_length((sip)->sti_name)
#define	IPC_STRINFO_SIZE(sip)	((sip)->sti_pduops.ipops_size)
#define	IPC_STRINFO_ALIGN(sip)	((sip)->sti_pduops.ipops_align)

/*
 * Struct field information to be loaded into memory.
 */
struct ipc_fldinfo {
	ipc_cpduops_t	*fli_pduops;	/* PDU operations */
	uint32_t	fli_array;	/* number of array elements */
};

/*
 * Meta data of struct field indexed by its name.
 */
typedef struct {
	pfc_refptr_t	*flm_name;		/* field name */
	union {
		pfc_ipctype_t	type;		/* type of this field */
		ipc_strinfo_t	*strinfo;	/* struct information */
	} flm_type;
	uint32_t	flm_flags;		/* flags */
	uint32_t	flm_index;		/* field index */
	uint32_t	flm_size;		/* size of this field */
	uint32_t	flm_offset;		/* offset of this field */
	pfc_rbnode_t	flm_node;		/* Red-Black tree node */
} ipc_fldmeta_t;

#define	IPC_FLDMETA_NODE2PTR(node)				\
	PFC_CAST_CONTAINER((node), ipc_fldmeta_t, flm_node)
#define	IPC_FLDMETA_NAME(fmp)	pfc_refptr_string_value((fmp)->flm_name)

/*
 * Flags for flm_flags.
 */
#define	IPC_FLMF_ARRAY_MASK	PFC_CONST_U(0x001fffff)	/* array length */
#define	IPC_FLMF_STRUCT		PFC_CONST_U(0x80000000)	/* struct field */

#define	IPC_FLDMETA_ARRAY_LEN(fmp)			\
	((fmp)->flm_flags & IPC_FLMF_ARRAY_MASK)
#define	IPC_FLDMETA_IS_STRUCT(fmp)	((fmp)->flm_flags & IPC_FLMF_STRUCT)
#define	IPC_FLDMETA_IS_ARRAY(fmp)	(IPC_FLDMETA_ARRAY_LEN(fmp) != 0)

#define	IPC_FLDMETA_STRINFO(fmp)	((fmp)->flm_type.strinfo)
#define	IPC_FLDMETA_TYPE(fmp)		((fmp)->flm_type.type)
#define	IPC_FLDMETA_GETTYPE(fmp)			\
	((IPC_FLDMETA_IS_STRUCT(fmp))			\
	 ? PFC_IPCTYPE_STRUCT : IPC_FLDMETA_TYPE(fmp))

/*
 * Prototypes.
 */
extern int	pfc_ipc_struct_load_fields(void);
extern int	pfc_ipc_struct_loadfile(const char *path,
					pfc_bool_t need_field);
extern int	pfc_ipc_struct_loaddefault(const char *path,
					   pfc_bool_t need_field);
extern int	pfc_ipc_strinfo_get(const char *PFC_RESTRICT name,
				    ipc_cstrinfo_t **PFC_RESTRICT sipp,
				    pfc_bool_t need_fields);
extern void	pfc_ipc_strinfo_hold(ipc_cstrinfo_t *csip);
extern void	pfc_ipc_strinfo_release(ipc_cstrinfo_t *csip);

extern ipc_fldmeta_t	*pfc_ipc_strinfo_getfield(ipc_strinfo_t *PFC_RESTRICT
						  sip,
						  const char *PFC_RESTRICT
						  name);

#ifdef	_PFC_LIBPFC_IPC_BUILD

/*
 * Hold or release reference to IPC structure information.
 */
extern uint32_t		ipc_struct_refcnt;

#define	IPC_STRINFO_HOLD(csip)				\
	pfc_atomic_inc_uint32(&ipc_struct_refcnt)
#define	IPC_STRINFO_RELEASE(csip)			\
	pfc_atomic_dec_uint32(&ipc_struct_refcnt)

/*
 * Internal prototypes.
 */
extern int	pfc_ipc_struct_get(const char *PFC_RESTRICT name,
				   const char *PFC_RESTRICT sig,
				   pfc_cptr_t PFC_RESTRICT addr, uint32_t size,
				   ipc_cstrinfo_t **PFC_RESTRICT sipp);
extern void	pfc_ipc_struct_fork_prepare(void);
extern void	pfc_ipc_struct_fork_parent(void);
extern void	pfc_ipc_struct_fork_child(void);
extern void	pfc_ipc_struct_fini(void);
extern void	pfc_ipc_struct_bswap(ipc_cpduops_t *PFC_RESTRICT ops,
				     ipc_bswap_t *PFC_RESTRICT bsp);

#else	/* !_PFC_LIBPFC_IPC_BUILD */

/*
 * Hold or release reference to IPC structure information.
 */
#define	IPC_STRINFO_HOLD(csip)		pfc_ipc_strinfo_hold(csip)
#define	IPC_STRINFO_RELEASE(csip)	pfc_ipc_strinfo_release(csip)

#endif	/* _PFC_LIBPFC_IPC_BUILD */

#endif	/* !_PFC_LIBPFC_IPC_IPC_STRUCT_IMPL_H */

/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_JNI_H
#define	_PFC_JNI_H

/*
 * Definitions for JNI utilities.
 *
 * Remarks:
 *	- This header is provided only for JNI library build.
 *	  Other components must not include this header file.
 *
 *	- This header file requires libpfc_util to be linked.
 */

#ifndef	PFC_JNI_BUILD
#error	Do NOT include pfc/jni.h.
#endif	/* !PFC_JNI_BUILD */

#include <stdarg.h>
#include <jni.h>
#include <pfc/base.h>
#include <pfc/debug.h>

/*
 * Minimum JNI version supported by PFC-Core.
 */
#define	PJNI_MIN_VERSION	JNI_VERSION_1_6

/*
 * Fully qualified Java class names.
 */
#define	PJNI_CLASS_IllegalArgumentException	\
	"java/lang/IllegalArgumentException"
#define	PJNI_CLASS_IllegalStateException	\
	"java/lang/IllegalStateException"
#define	PJNI_CLASS_NullPointerException		\
	"java/lang/NullPointerException"
#define	PJNI_CLASS_NumberFormatException	\
	"java/lang/NumberFormatException"
#define	PJNI_CLASS_ArithmeticException		\
	"java/lang/ArithmeticException"
#define	PJNI_CLASS_ArrayIndexOutOfBoundsException	\
	"java/lang/ArrayIndexOutOfBoundsException"
#define	PJNI_CLASS_OutOfMemoryError		"java/lang/OutOfMemoryError"

#define	PJNI_CLASS_Object			"java/lang/Object"
#define	PJNI_CLASS_String			"java/lang/String"
#define	PJNI_CLASS_Thread			"java/lang/Thread"

#define	PJNI_CLASS(name)	PJNI_CLASS_##name

/*
 * JNI signature definitions.
 */
#define	PJNI_SIG_boolean		"Z"
#define	PJNI_SIG_byte			"B"
#define	PJNI_SIG_char			"C"
#define	PJNI_SIG_short			"S"
#define	PJNI_SIG_int			"I"
#define	PJNI_SIG_long			"J"
#define	PJNI_SIG_float			"F"
#define	PJNI_SIG_double			"D"
#define	PJNI_SIG_void			"V"
#define	PJNI_SIG_CLASS(clname)		"L" PJNI_CLASS_##clname ";"
#define	PJNI_SIG_ARRAY(primitive)	"[" PJNI_SIG_##primitive
#define	PJNI_SIG_CLARRAY(clname)	"[" PJNI_SIG_CLASS(clname)
#define	PJNI_SIG_jbyteArray		PJNI_SIG_ARRAY(byte)
#define	PJNI_SIG_jshortArray		PJNI_SIG_ARRAY(short)
#define	PJNI_SIG_jintArray		PJNI_SIG_ARRAY(int)
#define	PJNI_SIG_jlongArray		PJNI_SIG_ARRAY(long)
#define	PJNI_SIG_jfloatArray		PJNI_SIG_ARRAY(float)
#define	PJNI_SIG_jdoubleArray		PJNI_SIG_ARRAY(double)
#define	PJNI_SIG_String			PJNI_SIG_CLASS(String)

#define	PJNI_SIG_METHOD0(ret)		"()" PJNI_SIG_##ret
#define	PJNI_SIG_METHOD1(ret, arg)	"(" PJNI_SIG_##arg ")" PJNI_SIG_##ret
#define	PJNI_SIG_METHOD2(ret, arg1, arg2)			\
	"(" PJNI_SIG_##arg1 PJNI_SIG_##arg2 ")" PJNI_SIG_##ret
#define	PJNI_SIG_METHOD3(ret, arg1, arg2, arg3)				\
	"(" PJNI_SIG_##arg1 PJNI_SIG_##arg2 PJNI_SIG_##arg3 ")" PJNI_SIG_##ret
#define	PJNI_SIG_METHOD4(ret, arg1, arg2, arg3, arg4)		\
	"(" PJNI_SIG_##arg1 PJNI_SIG_##arg2 PJNI_SIG_##arg3	\
	PJNI_SIG_##arg4 ")" PJNI_SIG_##ret
#define	PJNI_SIG_METHOD5(ret, arg1, arg2, arg3, arg4, arg5)		\
	"(" PJNI_SIG_##arg1 PJNI_SIG_##arg2 PJNI_SIG_##arg3		\
	PJNI_SIG_##arg4 PJNI_SIG_##arg5 ")" PJNI_SIG_##ret

/*
 * Pseudo method name which indicates constructor.
 */
#define	PJNI_METHOD_CTOR		"<init>"

/*
 * Determine whether an exception is pending or not.
 */
#define	PJNI_EXCEPTION_CHECK(env)			\
	PFC_EXPECT_FALSE((*(env))->ExceptionCheck(env))

/*
 * Prototypes.
 */
extern const char	*pjni_string_get(JNIEnv *env, jstring string);
extern jbyteArray	pjni_bytearray_new(JNIEnv *env, const jbyte *array,
					   jsize nelems);
extern jshortArray	pjni_shortarray_new(JNIEnv *env, const jshort *array,
					    jsize nelems);
extern jintArray	pjni_intarray_new(JNIEnv *env, const jint *array,
					  jsize nelems);
extern jlongArray	pjni_longarray_new(JNIEnv *env, const jlong *array,
					   jsize nelems);
extern jfloatArray	pjni_floatarray_new(JNIEnv *env, const jfloat *array,
					    jsize nelems);
extern jdoubleArray	pjni_doublearray_new(JNIEnv *env, const jdouble *array,
					     jsize nelems);
extern jbyte		*pjni_bytearray_get(JNIEnv *env, jbyteArray array);
extern void		pjni_call_void(JNIEnv *env, jobject obj,
				       const char *name, const char *sig, ...);
extern jobject		pjni_stcall_object(JNIEnv *env, const char *clname,
					   const char *name, const char *sig,
					   ...);
extern jobject		pjni_newobject(JNIEnv *env, const char *clname,
				       const char *sig, ...);
extern jclass		pjni_loadclass(JNIEnv *env, const char *clname);
extern void		pjni_throw(JNIEnv *env, const char *clname,
				   const char *fmt, ...)
	PFC_FATTR_PRINTFLIKE(3, 4);
extern void		pjni_vthrow(JNIEnv *env, const char *clname,
				    const char *fmt, va_list ap)
	PFC_FATTR_PRINTFLIKE(3, 0);
extern JNIEnv		*pjni_attach(JavaVM *jvm, pfc_bool_t *attached);

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pjni_string_release(JNIEnv *env, jstring string, const char *ustr)
 *	Release UTF-8 string obtained by pjni_string_get().
 *
 * Remarks:
 *	This function does nothing if `ustr' is NULL.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pjni_string_release(JNIEnv *env, jstring string, const char *ustr)
{
	if (PFC_EXPECT_TRUE(ustr != NULL)) {
		(*env)->ReleaseStringUTFChars(env, string, ustr);
	}
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pjni_bytearray_release(JNIEnv *env, jbyteArray array, jbyte *ptr,
 *			  pfc_bool_t do_abort)
 *	Release byte array elements obtained by pjni_bytearray_get().
 *
 *	If PFC_TRUE is specified to `do_abort', any change to the array
 *	elements pointed by `ptr' is discarded. If PFC_FALSE, array elements
 *	are copied back to byte array.
 *
 * Remarks:
 *	This function does nothing if `ustr' is NULL.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pjni_bytearray_release(JNIEnv *env, jbyteArray array, jbyte *ptr,
		       pfc_bool_t do_abort)
{
	if (PFC_EXPECT_TRUE(ptr != NULL)) {
		jint	mode = (do_abort) ? JNI_ABORT : 0;

		(*env)->ReleaseByteArrayElements(env, array, ptr, mode);
	}
}

/*
 * static inline void PFC_FATTR_ALWAYS_INLINE
 * pjni_detach(JavaVM *jvm)
 *	Detach calling thread from the specified Java VM.
 */
static inline void PFC_FATTR_ALWAYS_INLINE
pjni_detach(JavaVM *jvm)
{
	PFC_ASSERT_INT((*jvm)->DetachCurrentThread(jvm), JNI_OK);
}

/*
 * static inline JavaVM PFC_FATTR_ALWAYS_INLINE *
 * pjni_getvm(JNIEnv *env)
 *	Get Java VM handle associated with the specified JNI environment.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to Java VM handle is
 *	returned.
 *
 *	On error, an IllegalStateException is thrown, and NULL is returned.
 */
static inline JavaVM PFC_FATTR_ALWAYS_INLINE *
pjni_getvm(JNIEnv *env)
{
	JavaVM	*jvm;
	jint	ret = (*env)->GetJavaVM(env, &jvm);

	if (PFC_EXPECT_FALSE(ret != 0)) {
		/* This should never happen. */
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unable to get Java VM handle.");
		jvm = NULL;
	}

	return jvm;
}

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * pjni_ensure_local_capacity(JNIEnv *env, jint capacity)
 *	Ensure that at least the specified number of local references can
 *	be created on the calling thread.
 *
 * Calling/Exit State:
 *	Upon successful completion, PFC_TRUE is returned.
 *	Otherwise an OutOfMemoryError is thrown and PFC_FALSE is returned.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
pjni_ensure_local_capacity(JNIEnv *env, jint capacity)
{
	jint	ret;

	ret = (*env)->EnsureLocalCapacity(env, capacity);
	if (PFC_EXPECT_FALSE(ret != 0)) {
		pjni_throw(env, PJNI_CLASS(OutOfMemoryError),
			   "Unable to ensure local capacity: capacity=%d",
			   capacity);

		return PFC_FALSE;
	}

	return PFC_TRUE;
}

/*
 * static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
 * pjni_push_local_frame(JNIEnv *env, jint capacity)
 *	Push a new local reference frame.
 *	`capacity' is the number of required local references.
 *
 * Calling/Exit State:
 *	Upon successful completion, PFC_TRUE is returned.
 *	Otherwise an OutOfMemoryError is thrown and PFC_FALSE is returned.
 */
static inline pfc_bool_t PFC_FATTR_ALWAYS_INLINE
pjni_push_local_frame(JNIEnv *env, jint capacity)
{
	jint	ret;

	ret = (*env)->PushLocalFrame(env, capacity);
	if (PFC_EXPECT_FALSE(ret != 0)) {
		pjni_throw(env, PJNI_CLASS(OutOfMemoryError),
			   "Unable to create local frame: capacity=%d",
			   capacity);

		return PFC_FALSE;
	}

	return PFC_TRUE;
}

/*
 * static inline jobject PFC_FATTR_ALWAYS_INLINE
 * pjni_pop_local_frame(JNIEnv *env, jobject result)
 *	Pop a current local reference frame.
 *	`result' is a local reference to be passed to previous frame.
 *
 * Calling/Exit State:
 *	A local reference associated with `result' is returned.
 */
static inline jobject PFC_FATTR_ALWAYS_INLINE
pjni_pop_local_frame(JNIEnv *env, jobject result)
{
	return (*env)->PopLocalFrame(env, result);
}

/*
 * static inline jfieldID PFC_FATTR_ALWAYS_INLINE
 * pjni_getfield(JNIEnv *env, jclass cls, const char *name, const char *sig)
 *	Get instance field ID.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL value to the field ID is
 *	returned.
 *	Otherwise NULL is returned with throwing an IllegalStateException.
 */
static inline jfieldID PFC_FATTR_ALWAYS_INLINE
pjni_getfield(JNIEnv *env, jclass cls, const char *name, const char *sig)
{
	jfieldID	fid = (*env)->GetFieldID(env, cls, name, sig);

	if (PFC_EXPECT_FALSE(fid == NULL)) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unable to determine field ID for \"%s\".", name);
		/* FALLTHROUGH */
	}

	return fid;
}

/*
 * static inline jclass PFC_FATTR_ALWAYS_INLINE
 * pjni_getclass(JNIEnv *env, jobject obj)
 *	Derive Java class from the specified Java object.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL value to Java class is returned.
 *	Otherwise NULL is returned with throwing IllegalStateException.
 */
static inline jclass PFC_FATTR_ALWAYS_INLINE
pjni_getclass(JNIEnv *env, jobject obj)
{
	jclass	cls = (*env)->GetObjectClass(env, obj);

	if (PFC_EXPECT_FALSE(cls == NULL)) {
		/* This should never happen. */
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unable to determine object class.");
		/* FALLTHROUGH */
	}

	return cls;
}

/*
 * static inline jclass PFC_FATTR_ALWAYS_INLINE
 * pjni_findclass(JNIEnv *env, const char *clname)
 *	Find a Java class associated with the specified class name.
 *
 *	`clname' must be a fully qualified Java class name.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL value to Java class is returned.
 *	Otherwise NULL is returned with throwing IllegalStateException.
 */
static inline jclass PFC_FATTR_ALWAYS_INLINE
pjni_findclass(JNIEnv *env, const char *clname)
{
	jclass	cls = (*env)->FindClass(env, clname);

	if (PFC_EXPECT_FALSE(cls == NULL)) {
		/* This should never happen. */
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unable to find Java class: %s", clname);
		/* FALLTHROUGH */
	}

	return cls;
}

/*
 * static inline jstring PFC_FATTR_ALWAYS_INLINE
 * pjni_newstring(JNIEnv *env, const char *string)
 *	Create a new String object which contains the UTF-8 string specified
 *	by `string'.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to String object is
 *	returned.
 *	Otherwise NULL is returned with throwing IllegalStateException.
 */
static inline jstring PFC_FATTR_ALWAYS_INLINE
pjni_newstring(JNIEnv *env, const char *string)
{
	jstring	jstr = (*env)->NewStringUTF(env, string);

	if (PFC_EXPECT_FALSE(jstr == NULL)) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unable to create a String.");
		/* FALLTHROUGH */
	}

	return jstr;
}

/*
 * Declare temporary JNI environment block.
 * PJNI_ATTACH() attaches the calling thread to the specified VM if not yet
 * attached, and PJNI_DETACH() detaches it.
 * PJNI_ATTACH() and PJNI_DETACH() are implemented as macro, and they must be
 * used in matching pairs at the same block.
 */
#define	PJNI_ATTACH(jvm, env)				\
	do {						\
		pfc_bool_t	__attached;		\
							\
		(env) = pjni_attach(jvm, &__attached);	\
		if (PFC_EXPECT_TRUE((env) != NULL)) {	\

#define	PJNI_DETACH(jvm)				\
		}					\
		if (__attached) {			\
			pjni_detach(jvm);		\
		}					\
	} while (0)

#endif	/* !_PFC_JNI_H */

/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * jni.c - JNI utilities.
 */

#include <stdio.h>
#include <pfc/jni.h>
#include <pfc/debug.h>
#include "jni_impl.h"

/*
 * Internal buffer size for exception message.
 */
#define	JEX_BUFSIZE		PFC_CONST_U(128)

/*
 * const char *
 * pjni_string_get(JNIEnv *env, jstring string)
 *	Get UTF-8 string in the string.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to UTF-8 string is
 *	returned.
 *	Otherwise NULL is returned.
 *
 * Remarks:
 *	- Returned UTF-8 string must be released by pjni_string_release().
 *
 *	- If NULL is returned, an IllegalStateException is thrown.
 *
 *	- The caller must guarantee that no exception is pending.
 */
const char *
pjni_string_get(JNIEnv *env, jstring string)
{
	const char	*ustr;

	ustr = (*env)->GetStringUTFChars(env, string, NULL);
	if (PFC_EXPECT_TRUE(ustr != NULL)) {
		return ustr;
	}

	pjni_throw(env, PJNI_CLASS(IllegalStateException),
		   "Unable to get UTF-8 string.");

	return NULL;
}

/*
 * jbyte *
 * pjni_bytearray_get(JNIEnv *env, jbyteArray array)
 *	Get byte array elements.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to jbyte array is
 *	returned.
 *	Otherwise an exception is thrown and NULL is returned.
 *
 * Remarks:
 *	- Returned jbyte array must be released by pjni_bytearray_release().
 *
 *	- The caller must guarantee that no exception is pending.
 */
jbyte *
pjni_bytearray_get(JNIEnv *env, jbyteArray array)
{
	jbyte	*ptr;

	ptr = (*env)->GetByteArrayElements(env, array, NULL);
	if (PFC_EXPECT_TRUE(ptr != NULL)) {
		return ptr;
	}

	pjni_throw(env, PJNI_CLASS(IllegalStateException),
		   "Unable to get byte array elements.");

	return NULL;
}

/*
 * Declare functions to construct a new Java array.
 */
#define	PJNI_NEWARRAY_DECL(prefix, jtype, ftype)			\
	jtype##Array							\
	pjni_##prefix##array_new(JNIEnv *env, const jtype *array,	\
				 jsize nelems)				\
	{								\
		jtype##Array	jarray;					\
									\
		jarray = (*env)->New##ftype##Array(env, nelems);	\
		if (PFC_EXPECT_FALSE(jarray == NULL)) {			\
			pjni_throw(env, PJNI_CLASS(IllegalStateException), \
				   "Unable to create %s array: nelems=%u", \
				   #prefix, nelems);			\
									\
			return NULL;					\
		}							\
									\
		if (PFC_EXPECT_TRUE(nelems != 0)) {			\
			/* Copy data into Java array. */		\
			PFC_ASSERT(array != NULL);			\
			(*env)->Set##ftype##ArrayRegion(env, jarray, 0,	\
							nelems, array); \
									\
			if (PJNI_EXCEPTION_CHECK(env)) {		\
				(*env)->DeleteLocalRef(env, jarray);	\
				jarray = NULL;				\
				/* FALLTHROUGH */			\
			}						\
		}							\
									\
		return jarray;						\
	}

/*
 * jbyteArray
 * pjni_bytearray_new(JNIEnv *env, const jbyte *array, jsize nelems)
 *
 * jshortArray
 * pjni_shortarray_new(JNIEnv *env, const jshort *array, jsize nelems)
 *
 * jintArray
 * pjni_intarray_new(JNIEnv *env, const jint *array, jsize nelems)
 *
 * jlongArray
 * pjni_longarray_new(JNIEnv *env, const jlong *array, jsize nelems)
 *
 * jfloatArray
 * pjni_floatarray_new(JNIEnv *env, const jfloat *array, jsize nelems)
 *
 * jdoubleArray
 * pjni_doublearray_new(JNIEnv *env, const jdouble *array, jsize nelems)
 *
 *	Create a new array of Java primitive type, and initialize it with
 *	the specified data.
 *
 *	This function creates a new array, and copy data specified by
 *	`array' into it. `nelems' must be the number of array elements
 *	specified by `array'.
 *
 * Calling/Exit State:
 *	Upon successful completion, a valid local reference to a newly
 *	created array is returned.
 *	Otherwise NULL is returned with throwing an exception.
 *
 * Remarks:
 *	The caller must guarantee that no exception is pending.
 */
PJNI_NEWARRAY_DECL(byte, jbyte, Byte);
PJNI_NEWARRAY_DECL(short, jshort, Short);
PJNI_NEWARRAY_DECL(int, jint, Int);
PJNI_NEWARRAY_DECL(long, jlong, Long);
PJNI_NEWARRAY_DECL(float, jfloat, Float);
PJNI_NEWARRAY_DECL(double, jdouble, Double);

/*
 * void
 * pjni_call_void(JNIEnv *env, jobject obj, const char *name, const char *sig,
 *		  ...)
 *	Call instance method of the specified Java object.
 *
 *	`obj' must be a Java instance.
 *	`name' must be the name of the method to be called, and `sig' must be
 *	the JNI signature of the method. The return type of the method must
 *	be void.
 *
 *	Rest of arguments are passed to instance method.
 *
 * Calling/Exit State:
 *	This function throws an exception on error.
 *	So PJNI_EXCEPTION_CHECK() returns true after error return.
 *
 * Remarks:
 *	- The caller must guarantee that no exception is pending.
 *
 *	- The caller must ensure that one more local reference slot is
 *	  available.
 */
void
pjni_call_void(JNIEnv *env, jobject obj, const char *name, const char *sig,
	       ...)
{
	va_list		ap;
	jclass		cls;
	jmethodID	mid;

	/* Determine object class. */
	cls = pjni_getclass(env, obj);
	if (PFC_EXPECT_FALSE(cls == NULL)) {
		return;
	}

	/* Determine method ID. */
	mid = (*env)->GetMethodID(env, cls, name, sig);
	(*env)->DeleteLocalRef(env, cls);
	if (PFC_EXPECT_FALSE(mid == NULL)) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unable to determine method ID: %s, %s", name, sig);
	}
	else {
		/* Call instance method. */
		va_start(ap, sig);
		(*env)->CallVoidMethodV(env, obj, mid, ap);
		va_end(ap);
	}
}

/*
 * jobject
 * pjni_stcall_object(JNIEnv *env, const char *clname, const char *name,
 *		      const char *sig, ...)
 *	Call static method of the specified class.
 *
 *	`clname' must be the fully qualified name of the class.
 *	`name' must be the name of the method to be called, and `sig' must be
 *	the JNI signature of the method. The return type of the method must
 *	be Java object.
 *
 *	Rest of arguments are passed to instance method.
 *
 * Calling/Exit State:
 *	Upon successful completion, the return value of the specified method
 *	is returned.
 *
 *	Otherwise NULL is returned with throwing an exception.
 *	So PJNI_EXCEPTION_CHECK() returns true after error return.
 *
 * Remarks:
 *	- The caller must guarantee that no exception is pending.
 *
 *	- The caller must ensure that two more local reference slots are
 *	  available.
 */
jobject
pjni_stcall_object(JNIEnv *env, const char *clname, const char *name,
		   const char *sig, ...)
{
	va_list		ap;
	jclass		cls;
	jobject		obj;
	jmethodID	mid;

	/* Determine the class. */
	cls = pjni_findclass(env, clname);
	if (PFC_EXPECT_FALSE(cls == NULL)) {
		return NULL;
	}

	/* Determine static method ID. */
	mid = (*env)->GetStaticMethodID(env, cls, name, sig);
	if (PFC_EXPECT_FALSE(mid == NULL)) {
		(*env)->DeleteLocalRef(env, cls);
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unable to determine static method: %s.%s, %s",
			   clname, name, sig);
		obj = NULL;
	}
	else {
		/* Call the specified static method. */
		va_start(ap, sig);
		obj = (*env)->CallStaticObjectMethodV(env, cls, mid, ap);
		va_end(ap);
		(*env)->DeleteLocalRef(env, cls);
	}

	return obj;
}

/*
 * jobject
 * pjni_newobject(JNIEnv *env, const char *clname, const char *sig, ...)
 *	Create a new Java object.
 *
 *	`clname' must be a fully qualified Java class name.
 *	`sig' must be the JNI signature of the constructor.
 *
 *	Rest of arguments are passed to the constructor.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to created Java object
 *	is returned.
 *
 *	Otherwise an exception is thrown, and then NULL is returned.
 *
 * Remarks:
 *	- The caller must guarantee that no exception is pending.
 *
 *	- The caller must ensure that two more local reference slots are
 *	  available.
 */
jobject
pjni_newobject(JNIEnv *env, const char *clname, const char *sig, ...)
{
	va_list		ap;
	jclass		cls;
	jobject		obj;
	jmethodID	mid;

	/* Determine the class. */
	cls = pjni_findclass(env, clname);
	if (PFC_EXPECT_FALSE(cls == NULL)) {
		return NULL;
	}

	/* Determine method ID of the constructor. */
	mid = (*env)->GetMethodID(env, cls, PJNI_METHOD_CTOR, sig);
	if (PFC_EXPECT_FALSE(mid == NULL)) {
		(*env)->DeleteLocalRef(env, cls);
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unable to determine constructor: %s, %s",
			   clname, sig);

		return NULL;
	}

	/* Create a new object. */
	va_start(ap, sig);
	obj = (*env)->NewObjectV(env, cls, mid, ap);
	va_end(ap);
	(*env)->DeleteLocalRef(env, cls);
	if (PFC_EXPECT_FALSE(obj == NULL)) {
		pjni_throw(env, PJNI_CLASS(IllegalStateException),
			   "Unable to construct a new object: %s", clname);
		/* FALLTHROUGH */
	}

	return obj;
}

/*
 * jclass
 * pjni_loadclass(JNIEnv *env, const char *clname)
 *	Load a Java class specified by a fully qualified class name.
 *
 *	Unlike pjni_findclass(), this function returns a global reference
 *	to a class object.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL global reference to Java class
 *	is returned.
 *	Otherwise NULL is returned with throwing an exception.
 */
jclass
pjni_loadclass(JNIEnv *env, const char *clname)
{
	jclass	cls;

	cls = pjni_findclass(env, clname);
	if (PFC_EXPECT_TRUE(cls != NULL)) {
		jclass	gcls = (*env)->NewGlobalRef(env, cls);

		(*env)->DeleteLocalRef(env, cls);
		cls = gcls;
		if (PFC_EXPECT_FALSE(gcls == NULL)) {
			pjni_throw(env, PJNI_CLASS(OutOfMemoryError),
				   "Unable to create a global reference: %s",
				   clname);
		}
	}

	return cls;
}

/*
 * void
 * pjni_throw(JNIEnv *env, const char *clname, const char *fmt, ...)
 *	Throw a Java exception.
 *
 *	`clname' must be a fully-qualified exception class name.
 *	The detailed message is specified by `fmt' and the rest of arguments
 *	in printf(3) format.
 *
 * Remarks:
 *	This function does nothing if an exception is pending.
 */
void
pjni_throw(JNIEnv *env, const char *clname, const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	pjni_vthrow(env, clname, fmt, ap);
	va_end(ap);
}

/*
 * void
 * pjni_vthrow(JNIEnv *env, const char *clname, const char *fmt, va_list ap)
 *	Throw a Java exception.
 *
 *	`clname' must be a fully-qualified exception class name.
 *	The detailed message is specified by `fmt' and `ap' in vprintf(3)
 *	format.
 *
 * Remarks:
 *	- This function does nothing if an exception is pending.
 *
 *	- The caller must ensure that one more local reference slot is
 *	  available.
 */
void
pjni_vthrow(JNIEnv *env, const char *clname, const char *fmt, va_list ap)
{
	jclass	cls;
	char	msg[JEX_BUFSIZE];

	if (PJNI_EXCEPTION_CHECK(env)) {
		/* Another exception is already thrown. */
		return;
	}

	vsnprintf(msg, sizeof(msg), fmt, ap);

	/*
	 * Resolve exception class name.
	 * Never use pjni_findclass() here because it may call pjni_throw().
	 */
	cls = (*env)->FindClass(env, clname);
	if (!PJNI_EXCEPTION_CHECK(env)) {
		PFC_ASSERT(cls != NULL);

		(*env)->ThrowNew(env, cls, msg);
		(*env)->DeleteLocalRef(env, cls);
	}
}

/*
 * JNIEnv *
 * pjni_attach(JavaVM *jvm, pfc_bool_t *attached)
 *	Attach JNI environment to the calling thread.
 *
 * Calling/Exit State:
 *	Upon successful completion, a non-NULL pointer to JNI environment is
 *	returned. PFC_TRUE is set to the buffer pointed by `attached' if a
 *	new environment was attached. PFC_FALSE is set if the calling thread
 *	already has a JNI environment.
 *
 *	NULL is returned on failure. PFC_FALSE is always set to `attached'
 *	on error.
 */
JNIEnv *
pjni_attach(JavaVM *jvm, pfc_bool_t *attached)
{
	JNIEnv	*env;
	jint	jerr;

	*attached = PFC_FALSE;

	/* Try to obtain JNI environment already attached. */
	jerr = (*jvm)->GetEnv(jvm, (void **)&env, PJNI_MIN_VERSION);
	if (PFC_EXPECT_TRUE(jerr == JNI_OK)) {
		return env;
	}

	if (PFC_EXPECT_FALSE(jerr != JNI_EDETACHED)) {
		pjni_log_error("Unable to get JNI environment: jerr=%d", jerr);

		return NULL;
	}

	/* Attach the calling thread to the Java VM. */
	jerr = (*jvm)->AttachCurrentThread(jvm, (void **)&env, NULL);
	if (PFC_EXPECT_FALSE(jerr != JNI_OK)) {
		pjni_log_error("Unable to attach current thread to Java VM: "
			       "jerr=%d", jerr);

		return NULL;
	}

	*attached = PFC_TRUE;

	return env;
}

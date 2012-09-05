/*
 * Variable argument (varargs) parsing macros compliant with the C standard.
 *
 * Copyright (c) 1991, 1993 The Regents of the University of California.
 * See section "BSD License" in the file LICENSES for licensing terms.
 *
 * This code is derived from JOS and from BSD. 
 */

/*
 * Adapted from PIOS for use in libdeterm by Chris Cotter.
 * Aug 2012.
 */

#ifndef _INC_STDARG_H
#define	_INC_STDARG_H

#ifdef __i386__
typedef char *va_list;

#define	__va_size(type) \
	(((sizeof(type) + sizeof(long) - 1) / sizeof(long)) * sizeof(long))

#define	va_start(ap, last) \
	((ap) = (va_list)&(last) + __va_size(last))

#define	va_arg(ap, type) \
	(*(type *)((ap) += __va_size(type), (ap) - __va_size(type)))

#define	va_end(ap)	((void)0)

#else

typedef __builtin_va_list va_list;
#define va_start(ap, last) __builtin_va_start(ap,last)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_end(ap) __builtin_va_end(ap)


#endif

#endif	/* !_INC_STDARG_H */

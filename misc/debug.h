/*
 * debug.h
 *
 *  Created on: 28.08.2014
 *      Author: markus
 */

#ifndef MISC_DEBUG_H_
#define MISC_DEBUG_H_

#define D1(x)
#define D2(x)

#define D_INFO fprintf(stderr, "%s, %d (%s): ", __FILE__, __LINE__, __func__);

#if VERBOSITY > 0
#undef D1
#define D1(x) D_INFO x
#endif

#if VERBOSITY > 1
#undef D2
#define D2(x) D_INFO x
#endif

#endif /* MISC_DEBUG_H_ */

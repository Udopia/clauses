/*
 * debug.h
 *
 *  Created on: 28.08.2014
 *      Author: markus
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#define D1(x)
#define D2(x)

#if VERBOSITY > 0
#undef D1
#define D1(x) x
#endif

#if VERBOSITY > 1
#undef D2
#define D2(x) x
#endif

#endif /* DEBUG_H_ */

/* 
 * File:   system.h
 * Author: Jake
 *
 * Created on October 30, 2012, 8:49 AM
 */

#ifndef SYSTEM_H
#define	SYSTEM_H

#define bitSet(a, b) ((a) |= (b))
#define bitClear(a, b) ((a) &= ~(b))

#define tmpGet(b) ((tmp) = (b))
#define tmpSet(b) ((tmp) |= (b))
#define tmpClear(b) ((tmp) &= ~(b))
#define tmpPut(b) ((b) = (tmp))

#endif	/* SYSTEM_H */


/******************************************************************************
 *                                                                            *
 *                 (C) Copyright 2000, Sony Corporation                       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *                                                                            *
 *    <IDENTIFICATION>                                                        *
 *       file name    : $Workfile: SonyErrorBase.h $
 *                                                                            *
 *    <PROFILE>                                                               *
 *       The defines of error base in sony CLIE system                        *
 *                                                                            *
 *    <HISTORY>                                                               *
 *       Started on   : 00/10/30                                              *
 *       Last Modified: $Date: 2002/02/11 08:28:28 $
 *                                                                            *
 ******************************************************************************/
/* this file is best viewed by setting TAB-stop as 3 */

#ifndef __SONYERRORBASE_H__
#define __SONYERRORBASE_H__

#include <ErrorBase.h>

// error code
#define sonyErrorClass				(oemErrorClass)
#define sonyVFSErrorClass			(sonyErrorClass | 0x100)
#define sonyHRErrorClass			(sonyErrorClass | 0x200)
#define sonyMsaErrorClass			(sonyErrorClass | 0x300)
#define sonyRmcErrorClass			(sonyErrorClass | 0x400)

#endif // __SONYERRORBASE_H__


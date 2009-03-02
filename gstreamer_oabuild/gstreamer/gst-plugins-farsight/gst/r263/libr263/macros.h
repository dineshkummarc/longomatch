/************************************************************************/
/* libr263: fast H.263 encoder library                                  */
/*                                                                      */
/* Copyright (C) 1996, 1997 by Roalt Aalmoes, Twente University         */
/*                     SPA multimedia group 'huygens'                   */
/*                                                                      */
/* Based on Telenor TMN 1.6 encoder (Copyright (C) 1995, Telenor R&D)   */
/* created by Karl Lillevold                                            */
/*                                                                      */
/* Author    : Roalt Aalmoes, <aalmoes@huygens.nl>                      */
/*                                                                      */
/* Version   : 0.1                                                      */
/* DISCLAIMER: See DISCLAIMER file also distributed with this package   */
/************************************************************************/

/*****************************************************************
 * tmn (TMN encoder) 
 * Copyright (C) 1995 Telenor R&D
 *                    Karl Olav Lillevold <kol@nta.no>                    
 *
 *
 *****************************************************************/


#define sign(a)  	((a) < 0 ? -1 : 1)
#define Int(a)          ((a) < 0 ? (int)(a-0.5) : (int)(a))
#define mnint(a)	((a) < 0 ? (int)(a - 0.5) : (int)(a + 0.5))
#define mfloor(a)       ((a) < 0 ? (int)(a - 0.5) : (int)(a))
#define mmax(a, b)  	((a) > (b) ? (a) : (b))
#define mmin(a, b)  	((a) < (b) ? (a) : (b))
#define limit(x) \
{ \
    if (x > 255) x = 255; \
    if (x <   0)   x = 0; \
}




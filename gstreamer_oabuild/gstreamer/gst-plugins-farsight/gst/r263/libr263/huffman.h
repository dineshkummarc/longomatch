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
 * This is written by Andy C. Hung
 *
 *****************************************************************/

/*************************************************************
Copyright (C) 1990, 1991, 1993 Andy C. Hung, all rights reserved.
PUBLIC DOMAIN LICENSE: Stanford University Portable Video Research
Group. If you use this software, you agree to the following: This
program package is purely experimental, and is licensed "as is".
Permission is granted to use, modify, and distribute this program
without charge for any purpose, provided this license/ disclaimer
notice appears in the copies.  No warranty or maintenance is given,
either expressed or implied.  In no event shall the author(s) be
liable to you or a third party for any special, incidental,
consequential, or other damages, arising out of the use or inability
to use the program for any purpose (or the loss of data), even if we
have been advised of such possibilities.  Any public reference or
advertisement of this source code should refer to it as the Portable
Video Research Group (PVRG) code, and not by any author(s) (or
Stanford University) name.
*************************************************************/
/*
************************************************************
huffman.h

Huffman stuff...
************************************************************
*/

#define EHUFF struct Modified_Encoder_Huffman

EHUFF
{
  int n;
  int *Hlen;
  int *Hcode;
};

/* From huffman.c */
void InitHuff();
void FreeHuff();
void PrintEhuff();

EHUFF *MakeEhuff();
void FreeEhuff(EHUFF *eh);
void LoadETable();
int Encode(int val,EHUFF *huff);
void mputv(int n,int b);

/* From stream.c */
void mwopen();
void mwclose();
int zeroflush();
void mputv();
long mwtell();
void mwseek();




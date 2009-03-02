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
 
/* This is the prototypes and definitions file associated with
   rlib.c
*/
#ifndef RLIB_H
#define RLIB_H



#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>


/* Definitions for AVA */
#define XTILESIZE 8
#define YTILESIZE 8

#define QCIF_YWIDTH 176
#define QCIF_YHEIGHT 144
#define QCIF_UWIDTH 88
#define QCIF_UHEIGHT 72
#define QCIF_VWIDTH 88
#define QCIF_VHEIGHT 72
#define QCIFXTILES 22
#define QCIFYTILES 18

#define SQCIF_YWIDTH 128
#define SQCIF_YHEIGHT 96

#define SQCIFXTILES 16
#define SQCIFYTILES 12

#define CIF4_YWIDTH 704
#define CIF4_YHEIGHT 576

#define CIF16_YWIDTH 1408
#define CIF16_YHEIGHT 1152


#define CIF_YWIDTH 352
#define CIF_YHEIGHT 288
#define CIF_UWIDTH 176
#define CIF_UHEIGHT 144
#define CIF_VWIDTH 176
#define CIF_VHEIGHT 144
#define CIFXTILES 44
#define CIFYTILES 36

/* As you can see, there are 2 different formats:
   - avaqcif format, U and V signed (not used in this distribution)
   - qcif format, U and V unsigned

   Also note that the data is received and stored as 8-bit characters,but
   is processed as 32-bit integers. This is done because the data can fit
   into 8-bit, but on most systems 8-bit operations are a LOT slower than
   32-bit operations.
*/

/* Format as expected by encoder/decoder */

/* Note that from AVA, the U and V values must be converted from signed to
   unsigned, like     U.new = (unsigned char) U.old + 128 
   (Not applicable for this distribution)
*/

struct qcif {
  unsigned int Y[QCIF_YHEIGHT][QCIF_YWIDTH];
  unsigned int U[QCIF_UHEIGHT][QCIF_UWIDTH];
  unsigned int V[QCIF_VHEIGHT][QCIF_VWIDTH];
};

struct cif {
  unsigned int Y[CIF_YHEIGHT][CIF_YWIDTH];
  unsigned int U[CIF_UHEIGHT][CIF_UWIDTH];
  unsigned int V[CIF_VHEIGHT][CIF_VWIDTH];
};


struct qcif8bit {
  unsigned char Y[QCIF_YHEIGHT][QCIF_YWIDTH];
  unsigned char U[QCIF_UHEIGHT][QCIF_UWIDTH];
  unsigned char V[QCIF_VHEIGHT][QCIF_VWIDTH];
};

struct cif8bit {
  unsigned char Y[CIF_YHEIGHT][CIF_YWIDTH];
  unsigned char U[CIF_UHEIGHT][CIF_UWIDTH];
  unsigned char V[CIF_VHEIGHT][CIF_VWIDTH];
};


/* Prototypes */
int ReadQCIF(FILE *f, struct qcif *aq);
int WriteQCIF(FILE *f, struct qcif *qc);
int ReadCIF(FILE *f, struct cif *aq);
int WriteCIF(FILE *f, struct cif *qc);
void my_usleep(unsigned int microseconds);
void ReadQCIFBuffer(const unsigned char* data, struct qcif *qc);
void ReadBuffer(const unsigned char * src, unsigned int *dst, int width, int height);

#endif

















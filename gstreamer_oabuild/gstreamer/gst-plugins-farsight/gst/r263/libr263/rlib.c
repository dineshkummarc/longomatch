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

#include "rlib.h"
#include "owntypes.h"

/* this function is added by Wang, Zhanglei */
void ReadBuffer(const unsigned char * src, unsigned int *dst, int width, int height)
{
  unsigned int *ud;
  unsigned char *us;
  int i;

  us = src;
  ud = dst;

  for(i = 0; i < width*height; i++)
    *ud++ = (unsigned int) *us++;

  us = src + width*height;
  ud = dst + width*height;

  for(; i < width*height*3/2; i++)
    *ud++ = (signed int) *us++;

  return;
}

void ReadQCIFBuffer(const unsigned char * data, struct qcif *qc)
{
  struct qcif8bit * source = (struct qcif8bit *)data;
  unsigned int *ud;
  unsigned char *us;
  int i;

  us = &source->Y[0][0];
  ud = &qc->Y[0][0];

  for(i = 0; i < QCIF_YWIDTH*QCIF_YHEIGHT; i++)
    *ud++ = (unsigned int) *us++;

  us = &source->U[0][0];
  ud = &qc->U[0][0];

  for(; i < QCIF_YWIDTH*QCIF_YHEIGHT + QCIF_UWIDTH*QCIF_UHEIGHT +
	QCIF_VWIDTH*QCIF_VHEIGHT; i++)
    *ud++ = (signed int) *us++;

  return;
}
	
int ReadQCIF(FILE *f, struct qcif *qc)
{
  struct qcif8bit source;
  int i;
  int file_err;
  unsigned int *ud;
  unsigned char *us;

  file_err = fread(&source, sizeof(source), 1, f);

  if(file_err != 1)
    return FALSE;

  us = &source.Y[0][0];
  ud = &qc->Y[0][0];
  
  for(i = 0; i < QCIF_YWIDTH*QCIF_YHEIGHT; i++)
    *ud++ = (unsigned int) *us++;
  
  us = &source.U[0][0];
  ud = &qc->U[0][0];
  
  for(; i < QCIF_YWIDTH*QCIF_YHEIGHT + QCIF_UWIDTH*QCIF_UHEIGHT + 
	QCIF_VWIDTH*QCIF_VHEIGHT; i++)
    *ud++ = (signed int) *us++;

  return TRUE;
}

int ReadCIF(FILE *f, struct cif *qc)
{
  struct cif8bit source;
  int i;
  int file_err;
  unsigned int *ud;
  unsigned char *us;

  file_err = fread(&source, sizeof(source), 1, f);

  if(file_err != 1)
    return FALSE;

  us = &source.Y[0][0];
  ud = &qc->Y[0][0];
  
  for(i = 0; i < CIF_YWIDTH*CIF_YHEIGHT; i++)
    *ud++ = (unsigned int) *us++;
  
  us = &source.U[0][0];
  ud = &qc->U[0][0];
  
  for(; i < CIF_YWIDTH*CIF_YHEIGHT + CIF_UWIDTH*CIF_UHEIGHT + 
	CIF_VWIDTH*CIF_VHEIGHT; i++)
    *ud++ = (signed int) *us++;

  return TRUE;
}

int WriteQCIF(FILE *f, struct qcif *qc)
{
  struct qcif8bit dest;
  int i;
  int file_err;
  unsigned char *ud;
  unsigned int *us;

  us = &qc->Y[0][0];
  ud = &dest.Y[0][0];
  
  for(i = 0; i < QCIF_YWIDTH*QCIF_YHEIGHT; i++)
    *ud++ = (unsigned char) *us++;
  
  us = &qc->U[0][0];
  ud = &dest.U[0][0];

  for(; i < QCIF_YWIDTH*QCIF_YHEIGHT + QCIF_UWIDTH*QCIF_UHEIGHT + 
	QCIF_VWIDTH*QCIF_VHEIGHT; i++)
    *ud++ = (unsigned char) *us++;

 file_err = fwrite(&dest, sizeof(dest), 1, f);

 return (file_err == 1);

}

int WriteCIF(FILE *f, struct cif *qc)
{
  struct cif8bit dest;
  int i;
  int file_err;
  unsigned char *ud;
  unsigned int *us;

  us = &qc->Y[0][0];
  ud = &dest.Y[0][0];
  
  for(i = 0; i < CIF_YWIDTH*CIF_YHEIGHT; i++)
    *ud++ = (unsigned char) *us++;
  
  us = &qc->U[0][0];
  ud = &dest.U[0][0];

  for(; i < CIF_YWIDTH*CIF_YHEIGHT + CIF_UWIDTH*CIF_UHEIGHT + 
	CIF_VWIDTH*CIF_VHEIGHT; i++)
    *ud++ = (unsigned char) *us++;

 file_err = fwrite(&dest, sizeof(dest), 1, f);

 return (file_err == 1);

}

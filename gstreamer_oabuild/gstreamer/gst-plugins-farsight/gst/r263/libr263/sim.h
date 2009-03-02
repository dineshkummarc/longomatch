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

/**********************************************************************
 *
 * Headerfile for TMN4 coder
 * Type definitions and declaration of functions 
 * Date last modified: every now and then
 *
 **********************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include "macros.h"

#include "libr263.h"
/* config parameters */

#define YES 1
#define NO 0
#define DEF_UMV_MODE   NO
#define DEF_SAC_MODE   NO
#define DEF_ADV_MODE   NO
#define DEF_PBF_MODE   NO
#define DEF_ORIG_SKIP      0
#define DEF_FRAMESKIP      2   
/* headerlength on concatenated 4:1:1 YUV input file */
#define DEF_HEADERLENGTH   0

/* insert sync after each DEF_INSERT_SYNC for increased error robustness
 * 0 means do not insert extra syncs */
#define DEF_INSERT_SYNC   0




/* Parameters from TMN */
#define PREF_NULL_VEC 100
#define PREF_16_VEC 200
#define PREF_PBDELTA_NULL_VEC 50

int headerlength; /* Global variables */
int pels;
int cpels;
int uskip;
int vskip;
size_t sizeof_frame;
int lines;
int trace;
int advanced;
int pb_frames;
int mv_outside_frame;
int long_vectors;
int mbr;
int mbc;

float target_framerate;
FILE *tf;

#ifdef PRINT_DEBUG
FILE *debugf;
#endif


int search_p_frames;		/* internal variable for exh/log search */
FILE *mv_file;


/****************************/

#define PSC				1
#define PSC_LENGTH			17

#define ESCAPE                          7167

#define PCT_INTER                       1
#define PCT_INTRA                       0
#define ON                              1
#define OFF                             0

#define SF_SQCIF                        1  /* 001 */
#define SF_QCIF                         2  /* 010 */
#define SF_CIF                          3  /* 011 */
#define SF_4CIF                         4  /* 100 */
#define SF_16CIF                        5  /* 101 */

#define MODE_INTER                      0
#define MODE_INTER_Q                    1
#define MODE_INTER4V                    2
#define MODE_INTRA                      3
#define MODE_INTRA_Q                    4
#define MODE_SKIP			5 /* Created by Roalt */

#define PBMODE_NORMAL                   0
#define PBMODE_MVDB                     1
#define PBMODE_CBPB_MVDB                2

#define NO_VEC                          999

/* added by Roalt */
#define DEF_SPIRAL_SEARCH		0
#define DEF_LOGARITHMIC_SEARCH		1
#define DEF_WRITE_TRACE   NO

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (1)
#endif


/* Global variable */
#ifndef EXTERN_BUF
WriteByte WriteByteFunction;
#else
char * stream_buffer;
int stream_ptr;
#endif

/* Motionvector structure */

typedef struct motionvector {
  int x;			/* Horizontal comp. of mv 	 */
  int y;			/* Vertical comp. of mv 	 */
  int x_half;			/* Horizontal half-pel acc.	 */
  int y_half;			/* Vertical half-pel acc.	 */
  int min_error;		/* Min error for this vector	 */
  int Mode;                     /* Necessary for adv. pred. mode */
} MotionVector;

/* Point structure */

typedef struct point {
  int x;
  int y;
} Point;

/* Structure with image data */

typedef struct pict_image {
  unsigned int *lum;		/* Luminance plane		*/
  unsigned int *Cr;		/* Cr plane			*/
  unsigned int *Cb;		/* Cb plane			*/
} PictImage;

/* Group of pictures structure. */

/* Picture structure */

typedef struct pict {
  int prev; 
  int curr;
  int TR;             /* Time reference */
  int source_format;
  int picture_coding_type;
  int spare;
  int unrestricted_mv_mode;
  int PB;
  int QUANT;
  int DQUANT;
  int MB;
  int seek_dist;        /* Motion vector search window */
  int use_gobsync;      /* flag for gob_sync */
  int MODB;             /* B-frame mode */
  int BQUANT;           /* which quantizer to use for B-MBs in PB-frame */
  int TRB;              /* Time reference for B-picture */
  int frame_inc;        /* buffer control frame_inc */
  float QP_mean;        /* mean quantizer */
} Pict;


Pict *pic;

/* Slice structure */

typedef struct slice {
  unsigned int vert_pos;	/* Vertical position of slice 	*/
  unsigned int quant_scale;	/* Quantization scale		*/
} Slice;

/* Macroblock structure */

typedef struct macroblock {
  int mb_address;		/* Macroblock address 		*/
  int macroblock_type;		/* Macroblock type 		*/
  int skipped;			/* 1 if skipped			*/
  MotionVector motion;	        /* Motion Vector 		*/
} Macroblock;

/* Structure for macroblock data */

typedef struct mb_structure {
  int lum[16][16];
  int Cr[8][8];
  int Cb[8][8];
} MB_Structure;

/* Structure for average results and virtal buffer data */

typedef struct results {
  float SNR_l;			/* SNR for luminance */
  float SNR_Cr;			/* SNR for chrominance */
  float SNR_Cb;
  float QP_mean;                /* Mean quantizer */
} Results;

/* Internal prototypes */

/* stream.c prototypes */
void mwinit(void);
void mwcloseinit(void);

/* mot_est.c prototypes */
void FindMB(int x, int y, unsigned int *image, unsigned int MB[16][16]);
void FullMotionEstimation(unsigned int *curr, unsigned int *prev_ipol, 
		     int seek_dist, MotionVector *current_MV, int x_curr, 
		     int y_curr);

int SAD_HalfPixelMacroblock(unsigned int *ii,
			    unsigned int *curr,
			    int pixels_on_line, int Min_SAD);

int SAD_HalfPixelMacroblock2(unsigned int *ii,
			    unsigned int *curr,
			    int pixels_on_line, int Min_SAD);
unsigned int *LoadArea(unsigned int *im, int x, int y, 
			int x_size, int y_size, int lx);

/* pred.c prototypes */
MB_Structure *Predict_P(unsigned int *curr_image, unsigned int *prev_image,
			unsigned int *prev_ipol, int x, int y, 
			MotionVector *MV_ptr);

void DoPredChrom_P(int x_curr, int y_curr, int dx, int dy,
		   unsigned int *curr, unsigned int *prev, 
		   MB_Structure *pred_error);
void FindPred(int x, int y, MotionVector *fr, unsigned int *prev, 
	      int *pred); 

MB_Structure *MB_Recon_P(unsigned int *prev_image, unsigned int *prev_ipol,
			 MB_Structure *diff, int x_curr, int y_curr, 
			 MotionVector *MV_ptr);

void ReconLumBlock_P(int x, int y, MotionVector *fr,
		     unsigned int *prev, int *data);
void ReconChromBlock_P(int x_curr, int y_curr, int dx, int dy,
		       unsigned int *prev, MB_Structure *data);
void FindChromBlock_P(int x_curr, int y_curr, int dx, int dy,
		       unsigned int *prev, MB_Structure *data);
int ChooseMode(unsigned int *curr, int x_pos, int y_pos, int min_SAD);


/* countbit.c prototypes */
void ZeroBits(Bits *bits);
void ZeroRes(Results *res);
int FindCBP(int *qcoeff, int Mode, int ncoeffs);
void CountBitsVectors(MotionVector *MV_ptr, Bits *bits, 
		      int x, int y, int Mode, int newgob, Pict *pic);
void FindPMV(MotionVector *MV_ptr, int x, int y, 
	     int *p0, int *p1, int block, int newgob, int half_pel);
void CountBitsCoeff(int *qcoeff, int I, int CBP, Bits *bits, int ncoeffs);
int CodeCoeff(int Mode, int *qcoeff, int block, int ncoeffs);
int CountBitsPicture(Pict *pic);
void AddBitsPicture(Bits *bits);
void CountBitsMB(int Mode, int COD, int CBP, int CBPB, Pict *pic, Bits *bits);
int CountBitsSlice(int slice, int quant);
void ZeroVec(MotionVector *MV);
void MarkVec(MotionVector *MV);
void CopyVec(MotionVector *MV1, MotionVector *MV2);
int EqualVec(MotionVector *MV2, MotionVector *MV1);

/* coder.c prototypes */
void ZeroMBlock(MB_Structure *data);
void CodeIntraH263(CParam *params, Bits *bits);
void CodeInterH263(CParam *params, Bits *bits);
__inline__ void Clip(MB_Structure *data);
int *MB_EncodeAndFindCBP(MB_Structure *mb_orig, int QP, int I, int *CBP);
int MB_Decode(int *qcoeff, MB_Structure *mb_recon, int QP, int I);
void FullMotionEstimatePicture(unsigned int *curr, unsigned int *prev, 
			       unsigned int *prev_ipol, int seek_dist, 
			       MotionVector *MV_ptr,
			       int advanced_method,
			       int *EncodeThisBlock);
void ReconCopyImage(int i, int j, unsigned int *recon, unsigned int *prev_recon);
void ReconImage (int i, int j, MB_Structure *data, unsigned int *recon);
void InterpolateImage(unsigned int *image,
			       unsigned int *ipol_image, 
			       int w, int h);
void FillLumBlock( int x, int y, unsigned int *image, MB_Structure *data);
void FillChromBlock(int x_curr, int y_curr, unsigned int *image,
		    MB_Structure *data);


/* quant.c prototypes */
void Dequant(int *qcoeff, int *rcoeff, int QP, int I);
int QuantAndFindCBP(int *coeff, int *qcoeff, int QP, int I, int CBP_Mask);

/* dct.c prototypes */
int Dct( int *block, int *coeff);
int idct(int *coeff,int *block);

#ifndef FASTIDCT
/* global declarations for idctref */
void init_idctref (void);
void idctref (int *coeff, int *block);
#endif

#ifdef VERYFASTIDCT
void init_idct(void);
#endif

/* Fix broken header-files on suns to avoid compiler warnings */
/* #define BROKEN_SUN_HEADERS here or in Makefile */
#ifdef BROKEN_SUN_HEADERS
extern int printf();
extern int fprintf();
extern int time();
extern int fclose();
extern int rewind();
extern int fseek();
extern int fread();
extern int fwrite();
extern int fflush();
extern int fscanf();
extern int _flsbuf();
extern int _filbuf();
#endif


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

#ifndef LIBR263_H
#define LIBR263_H
/* This should not be changed */
#define MB_SIZE 16


/* Order of usage of lib263:
   1. Assign size of frame type to "format" field and call InitCompress()
   2. WriteByteFunction = OwnWriteFunction  (1 and 2 in arbitrary order)
   3. Set cparams and do CompressQCIFToH263(cparams) with INTRA encoding
   4. Set cparams and do CompressQCIFToH263(cparams) with either INTRA 
      or INTER encoding
   5. redo 4. or to stop do 6
   6. CloseCompress()
 */

/* Compression parameter structure */

#define CPARAM_INTER TRUE
#define CPARAM_INTRA FALSE
#define CPARAM_EXHAUSTIVE TRUE
#define CPARAM_LOGARITHMIC FALSE
#define CPARAM_ADVANCED TRUE
#define CPARAM_NOADVANCED FALSE

#define CPARAM_QCIF 0
#define CPARAM_CIF 1
#define CPARAM_4CIF 2
#define CPARAM_16CIF 3
#define CPARAM_SQCIF 4
#define CPARAM_OTHER 99

#define CPARAM_DEFAULT_INTER_Q 8
#define CPARAM_DEFAULT_INTRA_Q 8
#define CPARAM_DEFAULT_SEARCHWINDOW 3

#define CPARAM_DEFAULT_INTER CPARAM_INTRA
#define CPARAM_DEFAULT_SEARCH_METHOD CPARAM_LOGARITHMIC
#define CPARAM_DEFAULT_ADVANCED_METHOD CPARAM_NOADVANCED
#define CPARAM_DEFAULT_FORMAT CPARAM_QCIF

typedef struct compression_parameters {
/* Contains all the parameters that are needed for 
   encoding plus all the status between two encodings */
  int half_pixel_searchwindow; /* size of search window in half pixels
				  if this value is 0, no search is performed
				*/
  int format;			/*  */
  int pels;			/* Only used when format == CPARAM_OTHER */
  int lines;			/* Only used when format == CPARAM_OTHER */
  int inter;			/* TRUE of INTER frame encoded frames,
				   FALSE for INTRA frames */
  int search_method;		/* DEF_EXHAUSTIVE or DEF_LOGARITHMIC */
  int advanced_method;		/* TRUE : Use array to determine 
				          macroblocks in INTER frame
					  mode to be encoded */
  int Q_inter;			/* Quantization factor for INTER frames */
  int Q_intra;			/* Quantization factor for INTRA frames */
  unsigned int *data;		/* source data in qcif format */
  unsigned int *interpolated_lum;	/* intepolated recon luminance part */
  unsigned int *recon;		/* Reconstructed copy of compressed frame */
  int *EncodeThisBlock; 
                                /* Array when advanced_method is used */
  /* added by Wang, Zhanglei */
  int rtp_threshold;		/* if you don't want rtp support, set it to 0 */
} CParam;
/* Structure for counted bits */

typedef struct bits_counted {
  int Y;
  int C;
  int vec;
  int CBPY;
  int CBPCM;
  int MODB;
  int CBPB;
  int COD;
  int header;
  int DQUANT;
  int total;
  int no_inter;
  int no_inter4v;
  int no_intra;
/* NB: Remember to change AddBits(), ZeroBits() and AddBitsPicture() 
   when entries are added here */
   /* added by Wang, Zhanglei */
/* #ifdef RTP_SUPPORT */ /* avoid  #ifdef in struct definition */
   int rtp_offset[16];	/* at most 4 packets per frame, I guess, the first one is dummy, not used */
   int rtp_count;
/* #endif   */
} Bits;

typedef void (*WriteByte) (int);

/* Global variable */
#ifndef EXTERN_BUF
extern WriteByte WriteByteFunction;
#else
extern char * stream_buffer;
extern int stream_ptr;
#endif

/* Prototypes */
int CompressToH263(CParam *params, Bits *bits);
int InitCompress(CParam *params);
void CloseCompress(CParam *params);
void SkipH263Frames(int frames_to_skip);

/* Procedure to detect motion, expects param->EncodeThisBlock is set to
   array. 
   Advised values for threshold: mb_threholds = 2; pixel_threshold = 2 */
int FindMotion(CParam *params, int mb_threshold, int pixel_threshold);

#endif

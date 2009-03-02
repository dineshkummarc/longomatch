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

#include "sim.h"
#include "huffman.h"

/* static libr263.c prototypes */
static void init_motion_detection(void);
__inline__ static int Check8x8(unsigned int *orig, 
			       unsigned int *recon, int pos);
static int HasMoved(int call_time,  void *real, void *recon, int x, int y);


 
int InitCompress(CParam *params)
{


  pic = (Pict *)malloc(sizeof(Pict));
  if(!pic) {
    return -1;
  }

  pic->unrestricted_mv_mode = DEF_UMV_MODE;
  pic->use_gobsync = DEF_INSERT_SYNC;
  pic->PB = 0;
  pic->TR = 0;
  pic->QP_mean = 0.0;

  if(params->format == CPARAM_QCIF) {
    pels = QCIF_YWIDTH;
    lines = QCIF_YHEIGHT;
    cpels = QCIF_YWIDTH/2;
    pic->source_format = SF_QCIF;
  } else if (params->format == CPARAM_CIF) {
    pels = CIF_YWIDTH;
    lines = CIF_YHEIGHT;
    cpels = CIF_YWIDTH/2;
    pic->source_format = SF_CIF;
  } else if (params->format == CPARAM_SQCIF) {
    pels = SQCIF_YWIDTH;
    lines = SQCIF_YHEIGHT;
    cpels = SQCIF_YWIDTH/2;
    pic->source_format = SF_SQCIF;
  } else if (params->format == CPARAM_4CIF) {
    pels = CIF4_YWIDTH;
    lines = CIF4_YHEIGHT;
    cpels = CIF4_YWIDTH/2;
    pic->source_format = SF_4CIF;  
  } else if (params->format == CPARAM_16CIF) {
    pels = CIF16_YWIDTH;
    lines = CIF16_YHEIGHT;
    cpels = CIF16_YWIDTH/2;
    pic->source_format = SF_16CIF;
  } else {
    pels = params->pels;
    lines = params->lines;
    cpels = params->pels / 2;
    pic->source_format = 0;	/* ILLEGAL H.263! Use it only for testing */
  }
  
  mbr = lines / MB_SIZE;
  mbc = pels / MB_SIZE;
  uskip = lines*pels;
  vskip = uskip + lines*pels/4;
  sizeof_frame = (vskip + lines*pels/4)*sizeof(int);


  headerlength = DEF_HEADERLENGTH;
  /* Initalize VLC_tables */
  InitHuff();
  mwinit();

  /* Init motion detection */
  init_motion_detection();


#ifdef VERYFASTIDCT
  init_idct();			/* Do this in case of VERYFASTIDCT */
#elif STANDARDIDCT
  init_idctref();		/* Do this in case of standard IDCT */
#endif
				/* Do nothing for FASTIDCT */

  /* Set internal variables */
  advanced = DEF_ADV_MODE;
  mv_outside_frame = DEF_UMV_MODE || DEF_ADV_MODE;
  long_vectors = DEF_UMV_MODE;
  pb_frames = DEF_PBF_MODE;
  search_p_frames = DEF_SPIRAL_SEARCH;
  trace = DEF_WRITE_TRACE;

  params->half_pixel_searchwindow = CPARAM_DEFAULT_SEARCHWINDOW; 
  params->inter = CPARAM_DEFAULT_INTER;
  params->search_method = CPARAM_DEFAULT_SEARCH_METHOD;
  params->advanced_method = CPARAM_DEFAULT_ADVANCED_METHOD;
  params->Q_inter = CPARAM_DEFAULT_INTER_Q;
  params->Q_intra = CPARAM_DEFAULT_INTRA_Q;

  params->interpolated_lum = malloc(pels*lines*4*sizeof(int));

  if(!params->interpolated_lum)
    return -1;

  params->recon = malloc(sizeof_frame);
  if(!params->recon) {
    free(params->interpolated_lum);
    free(pic);
    return -1;
  }

  return 0;
}

void SkipH263Frames(int frames_to_skip)
{
  pic->TR += frames_to_skip % 256;
}

int CompressToH263(CParam *params, Bits *bits)
{
  if(!params->inter) {
    CodeIntraH263(params, bits);
  } else {
    CodeInterH263(params, bits);
  }
  bits->header += zeroflush();  /* pictures shall be byte aligned */
  pic->TR += 1 % 256; /* one means 30 fps */
  /* added by Wang, Zhanglei */
  bits->rtp_offset[bits->rtp_count++] = stream_ptr;
  return 0;
}

void CloseCompress(CParam *params)
{
  mwcloseinit();
  free(params->interpolated_lum);
  free(params->recon);
  free(pic);
  return;
}

/* Motion Detection part */

static int global_mb_threshold;
static int global_pixel_threshold;

/* This array is computed for QCIF
  movement_detection[] = {0, 354, 528, 177,
                          3, 353, 531, 178,
		          352, 179, 530, 1,
		          355, 176, 2, 529 };
			  */
/* This array determines the order in a pixel is checked per 4x4 block */
/* {x, y} within [0..3] */
static unsigned int movement_coords[16][2] = { {0,0}, {2,2},{0,3},{1,1},
		               		       {3,0},{1,2},{3,3},{2,1},
				               {0,2},{3,1},{2,3},{1,0},
				               {3,2},{0,1},{2,0},{1,3} };


static int movement_detection[16][4];

void init_motion_detection()
{
  unsigned int counter, pos;

  for(counter = 0; counter < 16; counter++) {
    pos = movement_coords[counter][0] + movement_coords[counter][1]*pels;
    movement_detection[counter][0] = pos;
    movement_detection[counter][1] = pos + 4;
    movement_detection[counter][2] = pos + pels*4;
    movement_detection[counter][3] = pos + pels*4 + 4;
  }
  return;
}

__inline__ static int Check8x8(unsigned int *orig, unsigned int *recon, int pos)
{
  int value, index;
  register int thres = global_pixel_threshold;

  value = 0;

  /* Mark pixel changed when lum value differs more than "thres" */
  index = movement_detection[pos][0];
  value += abs(*(orig + index) - *(recon+index)) > thres;

  index = movement_detection[pos][1];
  value += abs(*(orig + index) - *(recon+index)) > thres;

  index = movement_detection[pos][2];				
  value += abs(*(orig + index) - *(recon+index)) > thres;

  index = movement_detection[pos][3];			
  value += abs(*(orig + index) - *(recon+index)) > thres;

  return value;
}

static int HasMoved(int call_time,  void *real,
	     void *recon, int x, int y)
{
  int offset1;
  unsigned int *MB_orig;
  unsigned int *MB_recon;
  int position;
  int value = 0;

  offset1 = (y*pels+x)*MB_SIZE;
  position = call_time;

  /* Integration of 8x8 and 4x4 check might improve performance, 
     but is not done here */
  MB_orig = (unsigned int *) real + offset1;
  MB_recon = (unsigned int *) recon + offset1;
  value += Check8x8(MB_orig, MB_recon, position);

  MB_orig += 8; MB_recon += 8;
  value += Check8x8(MB_orig, MB_recon, position); 

  MB_orig += 8*pels - 8; MB_recon += 8*pels - 8;
  value += Check8x8(MB_orig, MB_recon, position);

  MB_orig += 8; MB_recon += 8;
  value += Check8x8(MB_orig, MB_recon, position);

  return value > global_mb_threshold;	
  /* Mark MB changed if more than "global_mb_threshold" pixels are changed */
}


int FindMotion(CParam *params, int mb_threshold, int pixel_threshold)
{
  static int call_time = 0;

  int j,i;
  int counter = 0;

  global_mb_threshold = mb_threshold;
  global_pixel_threshold = pixel_threshold;

  for(j = 0; j < mbr; j++) {
    for(i = 0; i < mbc; i++) {
      *(params->EncodeThisBlock + j*mbc + i) = 
	HasMoved(call_time, params->data, params->recon, i,j);

      counter += *(params->EncodeThisBlock +j*mbc + i);
    }
  }

  call_time = (call_time + 1) % 16;
 
  return counter;
}




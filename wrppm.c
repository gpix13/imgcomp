/*
 * This file contains routines to write output images in PPM/PGM format.
 * The extended 2-byte-per-sample raw PPM/PGM formats are supported.
 * The PBMPLUS library is NOT required to compile this software
 * (but it is highly useful as a set of PPM image manipulation programs).
 *
 * These routines may need modification for non-Unix environments or
 * specialized applications.  As they stand, they assume output to
 * an ordinary stdio stream.
 */

#include "libjpeg/cdjpeg.h"		/* Common decls for cjpeg/djpeg applications */

#ifdef PPM_SUPPORTED

/*
 * For 12-bit JPEG data, we either downscale the values to 8 bits
 * (to write standard byte-per-sample PPM/PGM files), or output
 * nonstandard word-per-sample PPM/PGM files.  Downscaling is done
 * if PPM_NORAWWORD is defined (this can be done in the Makefile
 * or in jconfig.h).
 * (When the core library supports data precision reduction, a cleaner
 * implementation will be to ask for that instead.)
 */
#if BITS_IN_JSAMPLE == 8
#else
sfhdashfdkas
#endif

#define PUTPPMSAMPLE(ptr,v)  *ptr++ = (char) (v)
#define BYTESPERSAMPLE 1
#define PPM_MAXVAL 255


/* Private version of data destination object */

typedef struct {
  struct djpeg_dest_struct pub;	/* public fields */

  /* Usually these two pointers point to the same place: */
  char *iobuffer;		/* fwrite's I/O buffer */
  JSAMPROW pixrow;		/* decompressor output buffer */
  size_t buffer_width;		/* width of I/O buffer */
  JDIMENSION samples_per_row;	/* JSAMPLEs per output row */
} ppm_dest_struct;

typedef ppm_dest_struct * ppm_dest_ptr;


/*
 * Write some pixel data.
 * In this module rows_supplied will always be 1.
 *
 * put_pixel_rows handles the "normal" 8-bit case where the decompressor
 * output buffer is physically the same as the fwrite buffer.
 */

static void
put_pixel_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
		JDIMENSION rows_supplied)
{
  ppm_dest_ptr dest = (ppm_dest_ptr) dinfo;

  (void) JFWRITE(dest->pub.output_file, dest->iobuffer, dest->buffer_width);
}



//----------------------------------------------------------------------
// Startup: write the file header.
//----------------------------------------------------------------------

static void start_output_ppm (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  ppm_dest_ptr dest = (ppm_dest_ptr) dinfo;

  /* Emit file header */
  switch (cinfo->out_color_space) {
  case JCS_GRAYSCALE:
    /* emit header for raw PGM format */
    fprintf(dest->pub.output_file, "P5\n%ld %ld\n%d\n",
	    (long) cinfo->output_width, (long) cinfo->output_height,
	    PPM_MAXVAL);
    break;
  case JCS_RGB:
    /* emit header for raw PPM format */
    fprintf(dest->pub.output_file, "P6\n%ld %ld\n%d\n",
	    (long) cinfo->output_width, (long) cinfo->output_height,
	    PPM_MAXVAL);
    break;
  default:
    ERREXIT(cinfo, JERR_PPM_COLORSPACE);
  }
}


//----------------------------------------------------------------------
// Finish up at the end of the file.
//----------------------------------------------------------------------

static void finish_output_ppm (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  /* Make sure we wrote the output file OK */
  fflush(dinfo->output_file);
  if (ferror(dinfo->output_file))
    ERREXIT(cinfo, JERR_FILE_WRITE);
}


//----------------------------------------------------------------------
// The module selection routine for PPM format output.
//----------------------------------------------------------------------
djpeg_dest_ptr jinit_write_ppm (j_decompress_ptr cinfo)
{
  ppm_dest_ptr dest;

  /* Create module interface object, fill in method pointers */
  dest = (ppm_dest_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(ppm_dest_struct));
  dest->pub.start_output = start_output_ppm;
  dest->pub.finish_output = finish_output_ppm;

  /* Calculate output image dimensions so we can allocate space */
  jpeg_calc_output_dimensions(cinfo);

  /* Create physical I/O buffer.  Note we make this near on a PC. */
  dest->samples_per_row = cinfo->output_width * cinfo->out_color_components;
  dest->buffer_width = dest->samples_per_row * (BYTESPERSAMPLE * SIZEOF(char));
  dest->iobuffer = (char *) (*cinfo->mem->alloc_small)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, dest->buffer_width);


  /* We will fwrite() directly from decompressor output buffer. */
  /* Synthesize a JSAMPARRAY pointer structure */
  /* Cast here implies near->far pointer conversion on PCs */
  dest->pixrow = (JSAMPROW) dest->iobuffer;
  dest->pub.buffer = & dest->pixrow;
  dest->pub.buffer_height = 1;
  dest->pub.put_pixel_rows = put_pixel_rows;

  return (djpeg_dest_ptr) dest;
}

#endif /* PPM_SUPPORTED */

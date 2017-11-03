#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#include <stdio.h>
#include <jpeglib.h>
#include <stdlib.h>


JSAMPLE *raw_image = NULL;

/* dimensions of the image we want to write */
int width = 690;
int height = 298;
int bytes_per_pixel = 3; 
int color_space = JCS_RGB; 

void print_scanlines(JSAMPLE* ba, int row_stride)
{
  static int height;
  int i;

  for (i=0; i < row_stride; i++)
    printf("%d\n", ba[i]);

  //printf ("width: %3d height: %3d\n", row_stride, height++);
}
int read_jpeg_file( char *filename )
{
/* these are standard libjpeg structures for reading(decompression) */
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	JSAMPROW row_pointer[1];
	int row_stride;

	FILE *infile = fopen(filename, "rb");
	unsigned long location = 0;
	int i = 0;

	if (!infile)
	{
		printf("Error opening jpeg file %s\n!", filename);
		return -1;
	}

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

/* allocate memory to hold the uncompressed image */
	raw_image = (unsigned char*)malloc(cinfo.output_width*cinfo.output_height*cinfo.num_components);
/* now actually read the jpeg into the raw buffer */
	row_pointer[0] = (unsigned char *)malloc(cinfo.output_width*cinfo.num_components);
	row_stride = cinfo.output_width * cinfo.output_components;
/* read one scan line at a time */
	while(cinfo.output_scanline < cinfo.image_height)
	{
		jpeg_read_scanlines(&cinfo, row_pointer, 1);
		for( i=0; i<cinfo.image_width*cinfo.num_components;i++)
			raw_image[location++] = row_pointer[0][i];
		print_scanlines(row_pointer[0], row_stride);
	}


	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	free(row_pointer[0]);
	fclose( infile );

	return 1;
}

GLOBAL(void) write_JPEG_file (char * filename, int quality)
{
  
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
  /* More stuff */
	FILE * outfile;		/* target file */
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride;		/* physical row width in image buffer */

  /* Step 1: allocate and initialize JPEG compression object */


	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	if ((outfile = fopen(filename, "wb")) == NULL) 
	{
		fprintf(stderr, "can't open %s\n", filename);
		exit(1);
	}
	jpeg_stdio_dest(&cinfo, outfile);


	cinfo.image_width = width; 	/* image width and height, in pixels */
	cinfo.image_height = height;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
 
	jpeg_set_defaults(&cinfo);
  
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
	jpeg_set_colorspace(&cinfo, JCS_GRAYSCALE);
  /* Step 4: Start compressor */

	jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */

	row_stride = width * 3;	/* JSAMPLEs per row in image_buffer */

	while (cinfo.next_scanline < cinfo.image_height) 
	{

		row_pointer[0] = & raw_image[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

  /* Step 6: Finish compression */

	jpeg_finish_compress(&cinfo);
  /* After finish_compress, we can close the output file. */
	fclose(outfile);

  /* Step 7: release JPEG compression object */

  /* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_compress(&cinfo);

  /* And we're done! */
}


int main()
{
	char *infilename = "jpeg-home.jpg", *outfilename = "test_oot.jpg";

/* Try opening a jpeg*/
	if(read_jpeg_file( infilename ) > 0)
	{
/* then copy it to another file */
		
		write_JPEG_file( outfilename, 100 );
 		return -1;
	}
	else 
		return -1;
	
	return 0;
}



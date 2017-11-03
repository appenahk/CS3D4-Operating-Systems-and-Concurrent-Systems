#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#pragma GCC diagnostic ignored "-Wint-conversion"
#pragma GCC diagnostic ignored "-Wformat="
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#include <stdio.h>
#include <jpeglib.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>


JSAMPLE *raw_image = NULL;

typedef struct data_struct {
    char *filename;
    int quality;
} data_struct;

/* dimensions of the image we want to write */
int width = 640;
int height = 480;
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

void *greyscale(void *threadarg)

{	char *filename;
	int quality;
	
	struct data_struct *actual_args = (data_struct*) threadarg;
	(*actual_args).filename = *filename;
	(*actual_args).quality = quality;

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
 
	FILE * outfile;		
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
	cinfo.in_color_space = JCS_RGB; 	
 
	jpeg_set_defaults(&cinfo);
  
	jpeg_set_quality(&cinfo, quality, TRUE);
	jpeg_set_colorspace(&cinfo, JCS_GRAYSCALE);
  /* Step 4: Start compressor */

	jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */

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
	jpeg_destroy_compress(&cinfo);
	free(threadarg);
	pthread_exit(NULL);
}

void read_jpeg_file(char *filename)
{
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
	}

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	raw_image = (unsigned char*)malloc(cinfo.output_width*cinfo.output_height*cinfo.num_components);

	row_pointer[0] = (unsigned char *)malloc(cinfo.output_width*cinfo.num_components);
	row_stride = cinfo.output_width * cinfo.output_components;
/* read one scan line at a time */

	while(cinfo.output_scanline < cinfo.image_height)
	{
		jpeg_read_scanlines(&cinfo, row_pointer, 1);
		for( i=0; i<cinfo.image_width*cinfo.num_components;i++)
			raw_image[location++] = row_pointer[0][i];
		//print_scanlines(row_pointer[0], row_stride);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	free(row_pointer[0]);
	fclose( infile );

}

int chartoint(char* threadcnt)
{
	char* num_ptr = threadcnt;
	while(*num_ptr < '0' || *num_ptr > '9')
          ++num_ptr;
     	int num = atoi(num_ptr);
	return num;
}
int main(int argc, char *argv[])
{
	
	char *infilename = argv[1], *outfilename = argv[2];
 	int NUM_THREADS = chartoint(argv[3]);
	int quality = 100;
	pthread_t threads[NUM_THREADS]; 
	//data_struct tdata[NUM_THREADS];
	int rc,t; 
	struct data_struct *args;

	clock_t begin = clock();
	read_jpeg_file( infilename ) ;
	for(t = 0; t<NUM_THREADS; t++)
	{	
		args = malloc(sizeof(data_struct));
		(*args).filename = outfilename;
		(*args).quality = quality;
		
		rc = pthread_create(&threads[t], NULL, greyscale, (void*)args);
		
		if(rc)
		{
			free(args);
			printf("ERROR return code from pthread_create(): %d\n", rc);
			exit(-1);
		}
	}
	
	for(t = 0; t<NUM_THREADS;t++)
	{
	
		pthread_join(threads[t], NULL);
	}
	
	//write_JPEG_file( outfilename, 100 );
	printf("%d x %d, %d pixels, 8 bit RGB\n", width, height, (width*height));
	
	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	
	printf("convert to %s\n", argv[4]);
	printf("%d threads in use\n", NUM_THREADS);	
	printf("%f seconds picture processing time\n", time_spent);
	
	return 0;
}

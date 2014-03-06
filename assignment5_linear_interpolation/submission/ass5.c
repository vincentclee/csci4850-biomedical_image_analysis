#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mvcd.h"
#include "dbh.h"

void ShowHdr(char *, struct dsr *);
void swap_long(unsigned char *);
void swap_short(unsigned char *);

int main(int argc, char *argv[]) {
	struct dsr hdr;
	unsigned char ***src, ***outp, temp[8];
	int i, j, k, x, image_size, z_size, sum, average;
	float Scale_image, Scale_z;
	FILE *fp;
	int out_image_size, out_z_size;
	char *pch;
	
	//argv[3] scale factor (example:0.5)
	Scale_image = (float)atof(argv[3]);
	Scale_z = (float)atof(argv[3]);

	//find image dimensions from argv[1] .hdr header file
	if((fp = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr,"Can't open:<%s>\n", argv[1]);
		exit(1);
	}
	fread(&hdr, 1, sizeof(struct dsr), fp);
	
	if(hdr.dime.dim[0] < 0 || hdr.dime.dim[0] > 15)
		swap_hdr(&hdr);
	
	/* image size in xy plane */
	image_size = (int)hdr.dime.dim[1];
	
	//Scale strut for re-write to new .hdr file.
	hdr.dime.dim[1] = (int) hdr.dime.dim[1] * Scale_image;
	hdr.dime.dim[2] = (int) hdr.dime.dim[2] * Scale_image;
	hdr.dime.dim[3] = (int) hdr.dime.dim[3] * Scale_z;
	
	//set data
	/*
	strncpy(hdr.hist.descrip, "12345678901234567890123456789012345678901234567890123456789012345678901234567890", 80);
	strncpy(hdr.hist.aux_file, "imagefile.exe", 24);
	strncpy(hdr.hist.generated, "1111111111", 10);
	strncpy(hdr.hist.scannum, "2222222222", 10);
	strncpy(hdr.hist.patient_id, "3333333333", 10);
	strncpy(hdr.hist.exp_date, "09-17-2013", 10);
	strncpy(hdr.hist.exp_time, "02:30:33", 10);
	*/
	
	//write new .hdr file
	fp = fopen(argv[2], "w");
	fwrite(&hdr, 1, sizeof(struct dsr), fp);
	fclose(fp);
	
	//Show new header file information
	//ShowHdr(argv[1], &hdr);
	
	/*
	 * Find last occurrence of .
	 * change letters after that to img
	 * example: file.hdr to file.img
	 */
	pch = strrchr(argv[1],'.');
	argv[1][pch-argv[1]+1] = 'i';
	argv[1][pch-argv[1]+2] = 'm';
	argv[1][pch-argv[1]+3] = 'g';
	//puts(argv[1]);

	//Open original .img file
	fp = myopen(argv[1], "r");
	fseek(fp,0,SEEK_END);
	z_size = ftell(fp) / (image_size*image_size);
	rewind(fp);
	
	out_image_size = image_size*Scale_image;
	out_z_size =  z_size*Scale_z;
	
	printf("Input  image size: %d, %d, %d\n", image_size, image_size, z_size);
	printf("Output image size: %d, %d, %d\n", out_image_size, out_image_size, out_z_size);
	
	/* apply for memory */
	src = UCalloc3d(image_size, image_size, z_size);
	outp = UCalloc3d(out_image_size, out_image_size, out_z_size);

	
	/* read data */
	for(k = 0; k < z_size; k++)
		for(i = 0; i < image_size; i++)
			fread(src[k][i], 1, image_size, fp);
	fclose(fp);
	
	/* sample ...  */
	for( k = 0; k < out_z_size; k++)
		for(i = 0; i < out_image_size; i++)
			for(j = 0; j < out_image_size; j++) {				
				/*
				 * find the eight pixel colors which border the coordinates
				 * floor 0
				 * ceil 1
				 * 
				 * 000
				 * 001
				 * 010
				 * 100
				 * 110
				 * 011
				 * 101
				 * 111
				 * 
				 */
				temp[0] = src[(int)floor(k/Scale_z)][(int)floor(i/Scale_image)][(int)floor(j/Scale_image)];
				temp[1] = src[(int)floor(k/Scale_z)][(int)floor(i/Scale_image)][(int)ceil(j/Scale_image)];
				temp[2] = src[(int)floor(k/Scale_z)][(int)ceil(i/Scale_image)][(int)floor(j/Scale_image)];
				temp[3] = src[(int)ceil(k/Scale_z)][(int)floor(i/Scale_image)][(int)floor(j/Scale_image)];
				temp[4] = src[(int)ceil(k/Scale_z)][(int)ceil(i/Scale_image)][(int)floor(j/Scale_image)];
				temp[5] = src[(int)floor(k/Scale_z)][(int)ceil(i/Scale_image)][(int)ceil(j/Scale_image)];
				temp[6] = src[(int)ceil(k/Scale_z)][(int)floor(i/Scale_image)][(int)ceil(j/Scale_image)];
				temp[7] = src[(int)ceil(k/Scale_z)][(int)ceil(i/Scale_image)][(int)ceil(j/Scale_image)];
				
				//sum up the eight pixel colors, and average
				sum = 0;
				for (x = 0; x < 8; x++)
					sum += (int)temp[x];
				
				outp[k][i][j] = (unsigned char)round(sum/8);
			}

	/*
	 * Find last occurrence of .
	 * change letters after that to img
	 * example: file.hdr to file.img
	 */
	pch = strrchr(argv[2],'.');
	argv[2][pch-argv[2]+1] = 'i';
	argv[2][pch-argv[2]+2] = 'm';
	argv[2][pch-argv[2]+3] = 'g';
	//puts(argv[2]);
	
	/* write the result */
	fp = myopen(argv[2], "w");
	for(k = 0; k < out_z_size; k++)
		for(i = 0; i < out_image_size; i++)
			fwrite(outp[k][i], 1, out_image_size, fp);
	fclose(fp);
	
	/* free */
	UCfree3d(src, z_size, image_size) ;
	UCfree3d(outp, out_z_size, out_image_size) ;
	
	return EXIT_SUCCESS;
}

/*
 * Method source:
 * http://eeg.sourceforge.net/ANALYZE75.pdf
 */
void ShowHdr(fileName,hdr)
struct dsr *hdr;
char *fileName;
{
	int i;
	char string[128];
	printf("Analyze Header Dump of: <%s> \n", fileName);
	/* Header Key */
	printf("sizeof_hdr: <%d> \n", hdr->hk.sizeof_hdr);
	printf("data_type:  <%s> \n", hdr->hk.data_type);
	printf("db_name:    <%s> \n", hdr->hk.db_name);
	printf("extents:    <%d> \n", hdr->hk.extents);
	printf("session_error: <%d> \n", hdr->hk.session_error);
	printf("regular:  <%c> \n", hdr->hk.regular);
	printf("hkey_un0: <%c> \n", hdr->hk.hkey_un0);

	/* Image Dimension */
	for(i=0;i<8;i++)
		printf("dim[%d]: <%d> \n", i, hdr->dime.dim[i]);
		   
	//strncpy(string,hdr->dime.vox_units,4);
	//printf("vox_units:  <%s> \n", string);
	
	//strncpy(string,hdr->dime.cal_units,8);
	//printf("cal_units: <%s> \n", string);
	//printf("unused1:   <%d> \n", hdr->dime.unused1);
	printf("datatype:  <%d> \n", hdr->dime.datatype);
	printf("bitpix:    <%d> \n", hdr->dime.bitpix);
       
	for(i=0;i<8;i++)
		printf("pixdim[%d]: <%6.4f> \n",i, hdr->dime.pixdim[i]);
       
	printf("vox_offset: <%6.4> \n",  hdr->dime.vox_offset);
	printf("funused1:   <%6.4f> \n", hdr->dime.funused1);
	printf("funused2:   <%6.4f> \n", hdr->dime.funused2);
	printf("funused3:   <%6.4f> \n", hdr->dime.funused3);
	printf("cal_max:    <%6.4f> \n", hdr->dime.cal_max);
	printf("cal_min:    <%6.4f> \n", hdr->dime.cal_min);
	printf("compressed: <%d> \n", hdr->dime.compressed);
	printf("verified:   <%d> \n", hdr->dime.verified);
	printf("glmax:      <%d> \n", hdr->dime.glmax);
	printf("glmin:      <%d> \n", hdr->dime.glmin);
	
	/* Data History */
	strncpy(string,hdr->hist.descrip,80);
	printf("descrip:  <%s> \n", string);
	strncpy(string,hdr->hist.aux_file,24);
	printf("aux_file: <%s> \n", string);
	printf("orient:   <%d> \n", hdr->hist.orient);
	
	strncpy(string,hdr->hist.originator,10);
	printf("originator: <%s> \n", string);
	
	strncpy(string,hdr->hist.generated,10);
	printf("generated: <%s> \n", string);
	
	
	strncpy(string,hdr->hist.scannum,10);
	printf("scannum: <%s> \n", string);
	
	strncpy(string,hdr->hist.patient_id,10);
	printf("patient_id: <%s> \n", string);
	
	strncpy(string,hdr->hist.exp_date,10);
	printf("exp_date: <%s> \n", string);
	
	strncpy(string,hdr->hist.exp_time,10);
	printf("exp_time: <%s> \n", string);
	
	strncpy(string,hdr->hist.hist_un0,10);
	printf("hist_un0: <%s> \n", string);
	
	printf("views:      <%d> \n", hdr->hist.views);
	printf("vols_added: <%d> \n", hdr->hist.vols_added);
	printf("start_field:<%d> \n", hdr->hist.start_field);
	printf("field_skip: <%d> \n", hdr->hist.field_skip);
	printf("omax: <%d> \n", hdr->hist.omax);
	printf("omin: <%d> \n", hdr->hist.omin);
	printf("smin: <%d> \n", hdr->hist.smax);
	printf("smin: <%d> \n", hdr->hist.smin);
}

/*
 * Method source:
 * http://eeg.sourceforge.net/ANALYZE75.pdf
 */
void swap_hdr(pntr)
struct dsr *pntr;
{
   swap_long(&pntr->hk.sizeof_hdr);
   swap_long(&pntr->hk.extents);
   swap_short(&pntr->hk.session_error);
   swap_short(&pntr->dime.dim[0]);
   swap_short(&pntr->dime.dim[1]);
   swap_short(&pntr->dime.dim[2]);
   swap_short(&pntr->dime.dim[3]);
   swap_short(&pntr->dime.dim[4]);
   swap_short(&pntr->dime.dim[5]);
   swap_short(&pntr->dime.dim[6]);
   swap_short(&pntr->dime.dim[7]);
   //swap_short(&pntr->dime.unused1);
   swap_short(&pntr->dime.datatype);
   swap_short(&pntr->dime.bitpix);
   swap_long(&pntr->dime.pixdim[0]);
   swap_long(&pntr->dime.pixdim[1]);
   swap_long(&pntr->dime.pixdim[2]);
   swap_long(&pntr->dime.pixdim[3]);
   swap_long(&pntr->dime.pixdim[4]);
   swap_long(&pntr->dime.pixdim[5]);
   swap_long(&pntr->dime.pixdim[6]);
   swap_long(&pntr->dime.pixdim[7]);
   swap_long(&pntr->dime.vox_offset);
   swap_long(&pntr->dime.funused1);
   swap_long(&pntr->dime.funused2);
   swap_long(&pntr->dime.cal_max);
   swap_long(&pntr->dime.cal_min);
   swap_long(&pntr->dime.compressed);
   swap_long(&pntr->dime.verified);
   swap_short(&pntr->dime.dim_un0);
   swap_long(&pntr->dime.glmax);
   swap_long(&pntr->dime.glmin);
}
/*
 * Method source:
 * http://eeg.sourceforge.net/ANALYZE75.pdf
 */
void swap_long(pntr)
unsigned char *pntr;
{
	unsigned char b0, b1, b2, b3;

	b0 = *pntr;
	b1 = *(pntr+1);
	b2 = *(pntr+2);
	b3 = *(pntr+3);

	*pntr = b3;
	*(pntr+1) = b2;
	*(pntr+2) = b1;
	*(pntr+3) = b0;
}
/*
 * Method source:
 * http://eeg.sourceforge.net/ANALYZE75.pdf
 */
void swap_short(pntr)
unsigned char *pntr;
{
	unsigned char b0, b1;
	
	b0 = *pntr;
	b1 = *(pntr+1);
	
	*pntr = b1;
	*(pntr+1) = b0;
}

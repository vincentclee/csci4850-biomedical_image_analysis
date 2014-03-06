##Assignment 5:

Use linear interpolation to replace the nearest neighbor interpolation used in the sampling function below. Please use the MRI image in Assignment 3 as testing images. 

```c
void SampleDownImg(int argc, char *argv[])
{
  unsigned char ***src, ***outp;
  int  i,j,k, image_size,z_size, temp ;
  float  ns ;
  FILE *fp;
  long idum, intensity ;
  float Scale_image, Scale_z ;
  int out_image_size, out_z_size ;

  sscanf(optarg, "%f,%f,%d", &Scale_image, &Scale_z, &image_size) ;

  /* image size in xy plane */
  /*  image_size=256;*/


  fp=myopen(argv[1],"r");
  fseek(fp,0,SEEK_END);
  z_size=ftell(fp)/(image_size*image_size);
  rewind(fp);

  out_image_size = image_size*Scale_image ;
  out_z_size = z_size*Scale_z ;

  printf("Input  image size: %d, %d, %d\n", image_size, image_size, z_size) ;
  printf("Output image size: %d, %d, %d\n", out_image_size, out_image_size, out_z_size) ;

  /* apply for memory */ 
  src    =UCalloc3d(image_size,image_size,z_size);
  outp   =UCalloc3d(out_image_size,out_image_size,out_z_size);

  /* read data */ 
  for(k=0;k<z_size;k++)
    for(i=0;i<image_size;i++)
      fread(src[k][i],1,image_size,fp);
  fclose(fp);

  
  /* sample ...  */ 
  for(k=0;k<out_z_size;k++)
    for(i=0;i<out_image_size;i++)
      for(j=0;j<out_image_size;j++)
	outp[k][i][j] = /*replace this line of code */ src[(int)(k/Scale_z)][(int)(i/Scale_image)][(int)(j/Scale_image)];


  /* write the result */
  fp=myopen(argv[2],"w");
  for(k=0;k<out_z_size;k++)
    for(i=0;i<out_image_size;i++)
      fwrite(outp[k][i],1,out_image_size,fp);
  fclose(fp);

  /* free */
  UCfree3d(src,     z_size, image_size) ;
  UCfree3d(outp,    out_z_size, out_image_size) ;
}
```

Due date: Sept 19.

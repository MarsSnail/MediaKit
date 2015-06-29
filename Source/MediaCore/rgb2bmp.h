#include <stdio.h>  
typedef unsigned char  BYTE;  
typedef unsigned short WORD;  

typedef struct  
{    long imageSize;  
    long blank;  
    long startPosition;  
}BmpHead;  

   
typedef struct  
{  
    long    Length;  
    long    width;  
    long    height;  
    WORD    colorPlane;  
    WORD    bitColor;  
    long    zipFormat;  
    long    realSize;  
    long    xPels;  
    long    yPels;  
    long    colorUse;  
    long    colorImportant;  
}InfoHead;  

typedef struct  
{  
         BYTE   rgbBlue;  
         BYTE   rgbGreen;  
         BYTE   rgbRed;  
         BYTE   rgbReserved;  
}RGBMixPlate;  

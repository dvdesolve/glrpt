/*
 * ANSI C code from the article
 * "Contrast Limited Adaptive Histogram Equalization"
 * by Karel Zuiderveld, karel@cv.ruu.nl
 * in "Graphics Gems IV", Academic Press, 1994
 *
 * These functions implement Contrast Limited Adaptive Histogram Equalization.
 * The main routine (CLAHE) expects an input image that is stored contiguously
 * in memory;  the CLAHE output image overwrites the original input image and
 * has the same minimum and maximum values (which must be provided by the user).
 * This implementation assumes that the X- and Y image resolutions are an integer
 * multiple of the X- and Y sizes of the contextual regions. A check on various
 * other error conditions is performed.
 *
 * The maximum number of contextual regions can be redefined
 * by changing MAX_REG_X and/or MAX_REG_Y; the use of more than 256
 * contextual regions is not recommended.
 *
 * The code is ANSI-C and is also C++ compliant.
 *
 * Author: Karel Zuiderveld, Computer Vision Research Group,
 * Utrecht, The Netherlands (karel@cv.ruu.nl)
 */

/*****************************************************************************/

#include "clahe.h"

#include "utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*****************************************************************************/

#define uiNR_OF_GREY    256

/* Max number of contextual regions in x and y directions */
#define MAX_REG_X   16
#define MAX_REG_Y   16

/*****************************************************************************/

static void ClipHistogram(
        unsigned long *pulHistogram,
        uint32_t uiNrGreylevels,
        unsigned long ulClipLimit);
static void MakeHistogram(
        kz_pixel_t *pImage,
        uint32_t uiXRes,
        uint32_t uiSizeX,
        uint32_t uiSizeY,
        unsigned long *pulHistogram,
        uint32_t uiNrGreylevels,
        kz_pixel_t *pLookupTable);
static void MapHistogram(
        unsigned long *pulHistogram,
        kz_pixel_t Min,
        kz_pixel_t Max,
        uint32_t uiNrGreylevels,
        unsigned long ulNrOfPixels);
static void MakeLut(
        kz_pixel_t *pLUT,
        kz_pixel_t Min,
        kz_pixel_t Max,
        uint32_t uiNrBins);
static void Interpolate(
        kz_pixel_t *pImage,
        uint32_t uiXRes,
        unsigned long *pulMapLU,
        unsigned long *pulMapRU,
        unsigned long *pulMapLB,
        unsigned long *pulMapRB,
        uint32_t uiXSize,
        uint32_t uiYSize,
        kz_pixel_t *pLUT);

/*****************************************************************************/

/* ClipHistogram()
 *
 * This function performs clipping of the histogram and redistribution
 * of bins. The histogram is clipped and the number of excess pixels
 * is counted. Afterwards the excess pixels are equally redistributed
 * across the whole histogram ( providing the bin count is smaller than
 * the cliplimit ).
 */
static void ClipHistogram(
        unsigned long *pulHistogram,
        uint32_t uiNrGreylevels,
        unsigned long ulClipLimit) {
  unsigned long *pulBinPointer, *pulEndPointer, *pulHisto;
  unsigned long ulNrExcess, ulUpper, ulBinIncr, ulStepSize, i;
  long lBinExcess;

  ulNrExcess = 0;
  pulBinPointer = pulHistogram;

  for( i = 0; i < uiNrGreylevels; i++ )
  {
    /* calculate total number of excess pixels */
    lBinExcess = (long)pulBinPointer[i] - (long)ulClipLimit;

    /* excess in current bin */
    if( lBinExcess > 0 ) ulNrExcess += (unsigned long)lBinExcess;
  }

  /* Second part: clip histogram and
   * redistribute excess pixels in each bin */

  /* average binincrement */
  ulBinIncr = ulNrExcess / uiNrGreylevels;

  /* Bins larger than ulUpper set to cliplimit */
  ulUpper =  ulClipLimit - ulBinIncr;

  for( i = 0; i < uiNrGreylevels; i++ )
  {
    if( pulHistogram[i] > ulClipLimit )
      pulHistogram[i] = ulClipLimit; /* clip bin */
    else
    {
      if( pulHistogram[i] > ulUpper )
      {
        /* high bin count */
        ulNrExcess -= pulHistogram[i] - ulUpper;
        pulHistogram[i] = ulClipLimit;
      }
      else
      {
        /* low bin count */
        ulNrExcess -= ulBinIncr;
        pulHistogram[i] += ulBinIncr;
      }
    }
  }

  while( ulNrExcess )
  {
    /* Redistribute remaining excess  */
    pulEndPointer = &pulHistogram[uiNrGreylevels];
    pulHisto = pulHistogram;

    while( ulNrExcess && (pulHisto < pulEndPointer) )
    {
      ulStepSize = uiNrGreylevels / ulNrExcess;
      if( ulStepSize < 1 ) ulStepSize = 1;  /* stepsize at least 1 */
      for(
          pulBinPointer = pulHisto;
          (pulBinPointer < pulEndPointer) && ulNrExcess;
          pulBinPointer += ulStepSize )
      {
        if( *pulBinPointer < ulClipLimit )
        {
          ( *pulBinPointer )++;
          ulNrExcess--;   /* reduce excess */
        }
      }

      /* restart redistributing on other bin location */
      pulHisto++;
    }
  }
}

/*****************************************************************************/

/* MakeHistogram()
 *
 * This function classifies the greylevels present in the array image into
 * a greylevel histogram. The pLookupTable specifies the relationship
 * between the greyvalue of the pixel (typically between 0 and 4095) and
 * the corresponding bin in the histogram (usually containing only 128 bins).
 */
static void MakeHistogram(
        kz_pixel_t *pImage,
        uint32_t uiXRes,
        uint32_t uiSizeX,
        uint32_t uiSizeY,
        unsigned long *pulHistogram,
        uint32_t uiNrGreylevels,
        kz_pixel_t *pLookupTable) {
  kz_pixel_t *pImagePointer;
  uint32_t i;

  /* clear histogram */
  for( i = 0; i < uiNrGreylevels; i++ )
    pulHistogram[i] = 0L;

  for( i = 0; i < uiSizeY; i++ )
  {
    pImagePointer = &pImage[uiSizeX];
    while( pImage < pImagePointer )
      pulHistogram[ pLookupTable[ *pImage++ ] ]++;
    pImagePointer += uiXRes;

    /* go to bdeginning of next row */
    pImage = &pImagePointer[ -(int)uiSizeX ];
  }
}

/*****************************************************************************/

/* MapHistogram()
 *
 * This function calculates the equalized lookup table
 * (mapping) by cumulating the input histogram. Note:
 * lookup table is rescaled in range [Min..Max].
 */
static void MapHistogram(
        unsigned long *pulHistogram,
        kz_pixel_t Min,
        kz_pixel_t Max,
        uint32_t uiNrGreylevels,
        unsigned long ulNrOfPixels) {
  uint32_t i;
  unsigned long ulSum = 0;
  const float fScale = ( (float)(Max - Min) ) / ulNrOfPixels;
  const unsigned long ulMin = (unsigned long)Min;

  for( i = 0; i < uiNrGreylevels; i++ )
  {
    ulSum += pulHistogram[i];
    pulHistogram[i] = (unsigned long)( ulMin + ulSum * fScale );
    if( pulHistogram[i] > Max ) pulHistogram[i] = Max;
  }
}

/*****************************************************************************/

/* MakeLut()
 *
 * To speed up histogram clipping, the input image [Min, Max] is
 * scaled down to [0, uiNrBins-1]. This function calculates the LUT.
 */
static void MakeLut(
        kz_pixel_t *pLUT,
        kz_pixel_t Min,
        kz_pixel_t Max,
        uint32_t uiNrBins) {
  int i;
  const kz_pixel_t BinSize = (kz_pixel_t)( 1 + ( Max - Min ) / uiNrBins );

  for( i = Min; i <= Max; i++ )
    pLUT[i] = (kz_pixel_t)( (i - (int)Min) / (int)BinSize );
}

/*****************************************************************************/

/* Interpolate()
 *
 * This function calculates the new greylevel assignments of pixels within a submatrix
 * of the image with size uiXSize and uiYSize. This is done by a bilinear interpolation
 * between four different mappings in order to eliminate boundary artifacts.
 * It uses a division; since division is often an expensive operation, I added code to
 * perform a logical shift instead when feasible.
 *
 * pImage      - pointer to input/output image
 * uiXRes      - resolution of image in x-direction
 * pulMap*     - mappings of greylevels from histograms
 * uiXSize     - uiXSize of image submatrix
 * uiYSize     - uiYSize of image submatrix
 * pLUT        - lookup table containing mapping greyvalues to bins
 */
static void Interpolate(
        kz_pixel_t *pImage,
        uint32_t uiXRes,
        unsigned long *pulMapLU,
        unsigned long *pulMapRU,
        unsigned long *pulMapLB,
        unsigned long *pulMapRB,
        uint32_t uiXSize,
        uint32_t uiYSize,
        kz_pixel_t *pLUT) {
  /* Pointer increment after processing row */
  const uint32_t uiIncr = uiXRes - uiXSize;

  /* Normalization factor */
  kz_pixel_t GreyValue;
  uint32_t uiNum = uiXSize*uiYSize;

  uint32_t uiXCoef, uiYCoef, uiXInvCoef, uiYInvCoef, uiShift = 0;

  /* If uiNum is not a power of two, use division */
  if( uiNum & (uiNum - 1) )
  {
    for(
        uiYCoef = 0, uiYInvCoef = uiYSize;
        uiYCoef < uiYSize;
        uiYCoef++, uiYInvCoef--, pImage += uiIncr )
    {
      for(
          uiXCoef = 0, uiXInvCoef = uiXSize;
          uiXCoef < uiXSize;
          uiXCoef++, uiXInvCoef-- )
      {
        GreyValue = pLUT[ *pImage ];  /* get histogram bin value */
        *pImage++ = (kz_pixel_t)
          ( (uiYInvCoef *
             (uiXInvCoef * pulMapLU[GreyValue] + uiXCoef * pulMapRU[GreyValue]) +
             uiYCoef *
             (uiXInvCoef * pulMapLB[GreyValue] + uiXCoef * pulMapRB[GreyValue])) /
            uiNum );
      }
    }
  }
  else
  {
    /* avoid the division and use a right shift instead */
    while( uiNum >>= 1 ) uiShift++; /* Calculate 2log of uiNum */

    for(
        uiYCoef = 0, uiYInvCoef = uiYSize;
        uiYCoef < uiYSize;
        uiYCoef++, uiYInvCoef--, pImage += uiIncr )
    {
      for(
          uiXCoef = 0, uiXInvCoef = uiXSize;
          uiXCoef < uiXSize;
          uiXCoef++, uiXInvCoef-- )
      {
        GreyValue = pLUT[*pImage]; /* get histogram bin value */
        *pImage++ = (kz_pixel_t)
          ( (uiYInvCoef *
             (uiXInvCoef * pulMapLU[GreyValue] + uiXCoef * pulMapRU[GreyValue]) +
             uiYCoef *
             (uiXInvCoef * pulMapLB[GreyValue] + uiXCoef * pulMapRB[GreyValue])) >>
            uiShift );
      }
    }
  }
}

/*****************************************************************************/

/* CLAHE()
 *
 * The number of "effective" greylevels in the output image is set by uiNrBins;
 * selecting a small value (eg. 128) speeds up processing and still produce an
 * output image of good quality. The output image will have the same minimum
 * and maximum value as the input image. A clip limit smaller than 1 results
 * in standard (non-contrast limited) AHE.
 *   pImage - Pointer to the input/output image
 *   uiXRes - Image resolution in the X direction
 *   uiYRes - Image resolution in the Y direction
 *   Min - Minimum greyvalue of input image (also becomes minimum of output image)
 *   Max - Maximum greyvalue of input image (also becomes maximum of output image)
 *   uiNrX - Number of contextial regions in the X direction (min 2, max MAX_REG_X)
 *   uiNrY - Number of contextial regions in the Y direction (min 2, max MAX_REG_Y)
 *   uiNrBins - Number of greybins for histogram ("dynamic range")
 *   double fCliplimit - Normalized cliplimit (higher values give more contrast)
 */
bool CLAHE(
        kz_pixel_t *pImage,
        uint32_t uiXRes,
        uint32_t uiYRes,
        kz_pixel_t Min,
        kz_pixel_t Max,
        uint32_t uiNrX,
        uint32_t uiNrY,
        uint32_t uiNrBins,
        double fCliplimit) {
  /* counters */
  uint32_t uiX, uiY;

  /* size of context. reg. and subimages */
  uint32_t uiXSize, uiYSize, uiSubX, uiSubY;

  /* auxiliary variables interpolation routine */
  uint32_t uiXL, uiXR, uiYU, uiYB;

  /* clip limit and region pixel count */
  unsigned long ulClipLimit, ulNrPixels;

  /* pointer to image */
  kz_pixel_t* pImPointer;

  /* lookup table used for scaling of input image */
  kz_pixel_t aLUT[uiNR_OF_GREY];

  /* pointer to histogram and mappings*/
  unsigned long *pulHist, *pulMapArray = NULL;

  /* auxiliary pointers interpolation */
  unsigned long* pulLU, *pulLB, *pulRU, *pulRB;

  /* Check for error conditions */
  bool error = 0;
  error |= (uiNrX > MAX_REG_X);         /* # of regions x-direction too large */
  error |= (uiNrY > MAX_REG_Y);         /* # of regions y-direction too large */
  error |= (uiXRes % uiNrX);            /* x-resolution no multiple of uiNrX */
  error |= (uiYRes % uiNrY);            /* y-resolution no multiple of uiNrY */
  error |= (Min >= Max);                /* minimum equal or larger than maximum */
  error |= ((uiNrX < 2) || (uiNrY < 2));/* at least 4 contextual regions required */
  error |= (fCliplimit == 1.0);         /* is OK, immediately returns original image. */
  
  if (error)
      return false;

  if( uiNrBins == 0 ) uiNrBins = 128;   /* default value when not specified */

  mem_alloc( (void **)&pulMapArray,
      sizeof(unsigned long) * uiNrX  *uiNrY * uiNrBins );

  /* Actual size of contextual regions */
  uiXSize = uiXRes / uiNrX;
  uiYSize = uiYRes / uiNrY;
  ulNrPixels = (unsigned long)uiXSize * (unsigned long)uiYSize;

  if( fCliplimit > 0.0 )
  {
    /* Calculate actual cliplimit */
    ulClipLimit = (unsigned long)( fCliplimit * (uiXSize * uiYSize) / uiNrBins );
    ulClipLimit = ( ulClipLimit < 1UL ) ? 1UL : ulClipLimit;
  }
  else ulClipLimit = 1UL << 14; /* Large value, do not clip (AHE) */

  /* Make lookup table for mapping of grey values */
  MakeLut( aLUT, Min, Max, uiNrBins );

  /* Calculate greylevel mappings for each contextual region */
  for( uiY = 0, pImPointer = pImage; uiY < uiNrY; uiY++ )
  {
    for( uiX = 0; uiX < uiNrX; uiX++, pImPointer += uiXSize )
    {
      pulHist = &pulMapArray[uiNrBins * (uiY * uiNrX + uiX)];
      MakeHistogram( pImPointer, uiXRes, uiXSize, uiYSize, pulHist, uiNrBins, aLUT );
      ClipHistogram( pulHist, uiNrBins, ulClipLimit );
      MapHistogram( pulHist, Min, Max, uiNrBins, ulNrPixels );
    }

    /* skip lines, set pointer */
    pImPointer += ( uiYSize - 1 ) * uiXRes;
  }

  /* Interpolate greylevel mappings to get CLAHE image */
  for( pImPointer = pImage, uiY = 0; uiY <= uiNrY; uiY++ )
  {
    if( uiY == 0 )
    {
      /* special case: top row */
      uiSubY = uiYSize >> 1;
      uiYU   = 0;
      uiYB   = 0;
    }
    else
    {
      if( uiY == uiNrY )
      {
        /* special case: bottom row */
        uiSubY = ( uiYSize + 1 ) >> 1;
        uiYU   = uiNrY - 1;
        uiYB   = uiYU;
      }
      else
      {
        /* default values */
        uiSubY = uiYSize;
        uiYU   = uiY  - 1;
        uiYB   = uiYU + 1;
      }
    }

    for( uiX = 0; uiX <= uiNrX; uiX++ )
    {
      if( uiX == 0 )
      {
        /* special case: left column */
        uiSubX = uiXSize >> 1;
        uiXL   = 0;
        uiXR   = 0;
      }
      else
      {
        if( uiX == uiNrX )
        {
          /* special case: right column */
          uiSubX = ( uiXSize + 1 ) >> 1;
          uiXL   = uiNrX - 1;
          uiXR   = uiXL;
        }
        else
        {
          /* default values */
          uiSubX = uiXSize;
          uiXL   = uiX  - 1;
          uiXR   = uiXL + 1;
        }
      }

      pulLU = &pulMapArray[ uiNrBins * (uiYU * uiNrX + uiXL) ];
      pulRU = &pulMapArray[ uiNrBins * (uiYU * uiNrX + uiXR) ];
      pulLB = &pulMapArray[ uiNrBins * (uiYB * uiNrX + uiXL) ];
      pulRB = &pulMapArray[ uiNrBins * (uiYB * uiNrX + uiXR) ];
      Interpolate(
          pImPointer, uiXRes,
          pulLU,  pulRU,
          pulLB,  pulRB,
          uiSubX, uiSubY,
          aLUT );

      /* set pointer on next matrix */
      pImPointer += uiSubX;
    }

    pImPointer += ( uiSubY - 1 ) * uiXRes;
  }

  /* free space for histograms */
  free_ptr( (void **)&pulMapArray );

  /* return status OK */
  return true;
}

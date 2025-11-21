// GIFDraw is called by AnimatedGIF library frame to screen

#undef USE_DMA

#define DISPLAY_WIDTH  tft.width()
#define DISPLAY_HEIGHT tft.height()
// Use inclusive display end coordinates provided by main sketch to avoid off-by-one in CASET/RASET
// The main sketch sets `displayXEnd = tft.width() - 1` and `displayYEnd = tft.height() - 1`.
extern int displayXEnd;
extern int displayYEnd;
#define BUFFER_SIZE 256            // Optimum is >= GIF width or integral division of width

#ifdef USE_DMA
  uint16_t usTemp[2][BUFFER_SIZE]; // Global to support DMA use
#else
  uint16_t usTemp[1][BUFFER_SIZE];    // Global to support DMA use
#endif
bool     dmaBuf = 0;
  
// Draw a line of image directly on the LCD
void GIFDraw(GIFDRAW *pDraw)
{
  // extern the SPI mutex created in the main sketch
  extern SemaphoreHandle_t spiMutex;
  // If the debug popup is active, skip any GIF drawing so the overlay remains intact.
  extern bool debugPopupActive;
  // When true the next GIF frame should draw transparent pixels as background so it fully overwrites overlays
  extern bool forceFullFrame;
  uint8_t *s;
  uint16_t *d, *usPalette;
  int x, y, iWidth, iCount;

  if (debugPopupActive) return;

  // Display bounds check and cropping using inclusive end coordinates
  iWidth = pDraw->iWidth;
  // Compute display width/height from inclusive ends
  int dispW = displayXEnd + 1; // equivalent to tft.width(), but based on inclusive end
  int dispH = displayYEnd + 1;
  // If the draw would exceed the display inclusive end, clamp width so end is inclusive
  if (iWidth + pDraw->iX - 1 > displayXEnd) {
    iWidth = displayXEnd - pDraw->iX + 1;
  }
  if (iWidth < 1) return;
  usPalette = pDraw->pPalette;
  y = pDraw->iY + pDraw->y; // current line
  // If y is beyond inclusive end or X start beyond inclusive end, return
  if (y > displayYEnd || pDraw->iX > displayXEnd || iWidth < 1)
    return;

  // Old image disposal
  s = pDraw->pPixels;
  if (pDraw->ucDisposalMethod == 2) // restore to background color
  {
    for (x = 0; x < iWidth; x++)
    {
      if (s[x] == pDraw->ucTransparent)
        s[x] = pDraw->ucBackground;
    }
    pDraw->ucHasTransparency = 0;
  }

  // Apply the new pixels to the main image
  if (pDraw->ucHasTransparency) // if transparency used
  {
    // If caller requested a full opaque frame, expand transparent pixels into the background
    if (forceFullFrame) {
      // Build full-width RGB565 line replacing transparent pixels with background
      s = pDraw->pPixels;
      usPalette = pDraw->pPalette;
      // we will produce iWidth pixels
      if (spiMutex) xSemaphoreTake(spiMutex, portMAX_DELAY);
      for (int xi = 0; xi < iWidth; xi++) {
        uint8_t c = s[xi];
        uint16_t px;
        if (c == pDraw->ucTransparent) px = usPalette[pDraw->ucBackground];
        else px = usPalette[c];
        usTemp[0][xi] = px;
      }
      // push the full line in one write (overwrite any overlay)
      tft.setAddrWindow(pDraw->iX, y, iWidth, 1);
      tft.pushPixels(&usTemp[0][0], iWidth);
      if (spiMutex) xSemaphoreGive(spiMutex);
      return;
    }
    uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
    pEnd = s + iWidth;
    x = 0;
    iCount = 0; // count non-transparent pixels
    while (x < iWidth)
    {
      c = ucTransparent - 1;
      d = &usTemp[0][0];
      while (c != ucTransparent && s < pEnd && iCount < BUFFER_SIZE )
      {
        c = *s++;
        if (c == ucTransparent) // done, stop
        {
          s--; // back up to treat it like transparent
        }
        else // opaque
        {
          *d++ = usPalette[c];
          iCount++;
        }
      } // while looking for opaque pixels
      if (iCount) // any opaque pixels?
      {
        // Acquire mutex to serialize SPI/TFT access with SD for this write
        if (spiMutex) xSemaphoreTake(spiMutex, portMAX_DELAY);
        // DMA would degrade performance here due to short line segments
        tft.setAddrWindow(pDraw->iX + x, y, iCount, 1);
        tft.pushPixels(usTemp, iCount);
        // Release mutex after write
        if (spiMutex) xSemaphoreGive(spiMutex);
        x += iCount;
        iCount = 0;
      }
      // no, look for a run of transparent pixels
      c = ucTransparent;
      while (c == ucTransparent && s < pEnd)
      {
        c = *s++;
        if (c == ucTransparent)
          x++;
        else
          s--;
      }
    }
  }
  else
  {
    s = pDraw->pPixels;

    // Unroll the first pass to boost DMA performance
    // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
    if (iWidth <= BUFFER_SIZE)
      for (iCount = 0; iCount < iWidth; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];
    else
      for (iCount = 0; iCount < BUFFER_SIZE; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];

#ifdef USE_DMA // 71.6 fps (ST7796 84.5 fps)
    tft.dmaWait();
    // Use iCount (actual number of pixels prepared) for the addr window width so
    // the window exactly matches the pushed pixel count.
    tft.setAddrWindow(pDraw->iX, y, iCount, 1);
    tft.pushPixelsDMA(&usTemp[dmaBuf][0], iCount);
    dmaBuf = !dmaBuf;
#else // 57.0 fps
    // Acquire mutex to serialize SPI/TFT access with SD for this write
    if (spiMutex) xSemaphoreTake(spiMutex, portMAX_DELAY);
    // Match the address window width to the amount of data we're about to push
    tft.setAddrWindow(pDraw->iX, y, iCount, 1);
    tft.pushPixels(&usTemp[0][0], iCount);
    if (spiMutex) xSemaphoreGive(spiMutex);
#endif

    iWidth -= iCount;
    // Loop if pixel buffer smaller than width
    while (iWidth > 0)
    {
      // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
      if (iWidth <= BUFFER_SIZE)
        for (iCount = 0; iCount < iWidth; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];
      else
        for (iCount = 0; iCount < BUFFER_SIZE; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];

#ifdef USE_DMA
      tft.dmaWait();
      tft.pushPixelsDMA(&usTemp[dmaBuf][0], iCount);
      dmaBuf = !dmaBuf;
#else
      // Acquire mutex to serialize SPI/TFT access with SD for this write
      if (spiMutex) xSemaphoreTake(spiMutex, portMAX_DELAY);
      tft.pushPixels(&usTemp[0][0], iCount);
      if (spiMutex) xSemaphoreGive(spiMutex);
#endif
      iWidth -= iCount;
    }
  }
} /* GIFDraw() */

#ifndef _TGA_H_
#define _TGA_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct __attribute__((__packed__)) Header {
    uint8_t id;
    uint8_t colorMapType;
    uint8_t imageType;
    uint16_t firstEntryIndex;
    uint16_t colorMapLength;
    uint8_t colorMapEntrySize;
    uint16_t xOrigin;
    uint16_t yOrigin;
    uint16_t imageWidth;
    uint16_t imageHeight;
    uint8_t pixelDepth;
    uint8_t imageDescriptor;
} Header_t;

typedef struct TGA {
    Header_t* header;
    uint32_t* colorBuffer;
} TGA_t;

// Change to stb_image.h at some point
TGA_t* ReadUncompressedRGB(char* fileName)
{
    FILE *fp = NULL;
    TGA_t* tga = NULL;

    fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening file: %s\n", strerror(errno));
        return tga;
    }
    
    Header_t* header = (Header_t*)malloc(sizeof(Header_t));
    if (header == NULL)
    {
        fprintf(stderr, "Could not allocate memory for the TGA header\n");
        return tga;
    }

    size_t read = fread(header, sizeof(Header_t), 1, fp);
    if (read == 0)
    {
        fprintf(stderr, "Failed to read any header bytes\n");
        return tga;
    }

    if (header->id > 0)
    {
        char imageId[header->id];
        read = fread(imageId, header->id, 1, fp);
        if (read == 0)
        {
            fprintf(stderr, "Failed to read any imageId bytes\n");
            return tga;
        }
    }

    if (header->colorMapType != 0)
    {
        fprintf(stderr, "ColorMap is not handled\n");
        return tga;
    }

    int pixels = header->imageWidth * header->imageHeight;

    //allocate for a 24-bit colorbuffer
    //--WIKI--
    //Image type 2 and 10: The image data is a direct representation of the pixel color. 
    //For a Pixel Depth of 15 and 16 bit, each pixel is stored with 5 bits per color. 
    //If the pixel depth is 16 bits, the topmost bit is reserved for transparency. 
    //For a pixel depth of 24 bits, each pixel is stored with 8 bits per color. 
    //--WIKI--

    uint32_t* buffer = (uint32_t*)malloc(sizeof(uint32_t)*pixels);
    if (buffer == NULL)
    {
        fprintf(stderr, "Could not allocate memory for the TGA header\n");
        return tga;
    }

    uint32_t* color = (uint32_t*)malloc(sizeof(uint32_t));
    if (color == NULL)
    {
        fprintf(stderr, "Could not allocate memory for the TGA header\n");
        return tga;
    }

    for (int i = 0; i < pixels; ++i)
    {
        fread(color, sizeof(uint8_t), 3, fp);
        buffer[i] = *color;
    }

    free(color);

    tga = (TGA_t*)malloc(sizeof(TGA_t));
    if (tga == NULL)
    {
        fprintf(stderr, "Could not allocate memory for the TGA header\n");
        return tga;
    }

    tga->header = header;
    tga->colorBuffer = buffer;

    fclose(fp);

    return tga;
}

void Dispose(TGA_t* tga)
{
    free(tga->header);
    free(tga->colorBuffer);
    free(tga);
}

#endif
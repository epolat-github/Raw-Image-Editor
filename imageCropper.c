#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct pixel
{
    int r, g, b;
};

// writes the given header information to the file given file
void writeHeader(FILE *filePtr, unsigned char *imageType, int bandNum, int height, int width, int bitNum)
{
    fwrite(imageType, 4 * sizeof(char), 1, filePtr);
    fwrite(&bandNum, 1, 1, filePtr);
    fwrite(&height, 4, 1, filePtr);
    fwrite(&width, 4, 1, filePtr);
    fwrite(&bitNum, 1, 1, filePtr);
}

// writes the given pixel to the given file location
void writePixel(struct pixel pixel, FILE *filePtr)
{
    fwrite(&pixel.r, 1, 1, filePtr);
    fwrite(&pixel.g, 1, 1, filePtr);
    fwrite(&pixel.b, 1, 1, filePtr);
}

// reads the given file's header information
void readHeader(FILE *filePtr, unsigned char *imageType, int *bandNum, int *height, int *width, int *bitNum)
{
    fread(imageType, 4 * sizeof(char), 1, filePtr);
    fread(bandNum, 1, 1, filePtr);
    fread(height, 4, 1, filePtr);
    fread(width, 4, 1, filePtr);
    fread(bitNum, 1, 1, filePtr);
}

// read the current pixel and assign them to the pixel struct
void readPixel(struct pixel *pixel, FILE *filePtr)
{
    int red = 0;
    int green = 0;
    int blue = 0;

    fread(&red, 1, 1, filePtr);
    fread(&green, 1, 1, filePtr);
    fread(&blue, 1, 1, filePtr);

    pixel->r = red;
    pixel->g = green;
    pixel->b = blue;
}

// free the allocated memory of the given array
void freeMem(struct pixel **arr, int height, int width)
{
    for (int i = 0; i < height; i++)
    {
        free(arr[i]);
    }

    free(arr);
}

int main(int argc, char const *argv[])
{

    if (argc != 7)
    {
        printf("%s\n", "Wrong number of inputs!");
        return 1;
    }

    const char *inputName = argv[1];
    const char *outputName = argv[2];

    // add 'D' letter to the beginning of the default output name
    char *magnifiedOutputName = malloc(strlen(outputName) + 2);

    if (magnifiedOutputName == NULL)
    {
        printf("Memory error\n");
        return 1;
    }
    magnifiedOutputName[0] = 'D';
    strcpy((magnifiedOutputName + 1), outputName);

    // crop informations
    int cropColumn = atoi(argv[3]);
    int cropRow = atoi(argv[4]);

    int cropWidth = atoi(argv[5]);
    int cropHeight = atoi(argv[6]);

    // header info variables
    unsigned char imageType[4];

    unsigned int bandNum = 0;

    unsigned int height = 0;
    unsigned int width = 0;

    unsigned int bitNum = 0;

    // initial input file
    FILE *inputFile = fopen(inputName, "rb");
    if (inputFile == NULL)
    {
        printf("%s\n", "Wrong input file name.");
        return 1;
    }

    // cropped output file
    FILE *outputFile = fopen(outputName, "wb");
    if (outputFile == NULL)
    {
        printf("%s\n", "Can't create output file name.");
        return 1;
    }

    // magnified output file
    FILE *magnifiedFile = fopen(magnifiedOutputName, "wb");
    if (magnifiedFile == NULL)
    {
        printf("%s\n", "Can't create magnified output file name.");
        return 1;
    }

    // default pixel values
    struct pixel black = {0, 0, 0};
    struct pixel white = {255, 255, 255};

    /* read input file */
    readHeader(inputFile, imageType, &bandNum, &height, &width, &bitNum);

    // input image holder
    struct pixel **inputPixelHolder = malloc(height * sizeof(*inputPixelHolder));
    if (inputPixelHolder == NULL)
    {
        printf("Memory error\n");
        return 1;
    }

    for (int i = 0; i < height; i++)
    {
        inputPixelHolder[i] = malloc(width * sizeof(*inputPixelHolder[i]));
        if (inputPixelHolder[i] == NULL)
        {
            printf("Memory error\n");
            return 1;
        }
    }

    // read and fill the pixel array
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            struct pixel newPixel;

            readPixel(&newPixel, inputFile);

            inputPixelHolder[i][j] = newPixel;
        }
    }

    /* write crop file */
    writeHeader(outputFile, imageType, bandNum, cropHeight, cropWidth, bitNum);

    // crop pixel holder to use in magnifying
    struct pixel **cropPixelHolder = malloc(cropHeight * sizeof(*cropPixelHolder));
    if (cropPixelHolder == NULL)
    {
        printf("Memory error\n");
        return 1;
    }

    for (int i = 0; i < cropHeight; i++)
    {
        cropPixelHolder[i] = malloc(cropWidth * sizeof(*cropPixelHolder[i]));
        if (cropPixelHolder[i] == NULL)
        {
            printf("Memory error\n");
            return 1;
        }
    }

    // print the cropped pixels to the crop output file
    for (int i = 0; i < cropHeight; i++)
    {
        struct pixel newPixel;

        for (int j = 0; j < cropWidth; j++)
        {
            int targetRow = cropRow + i;
            int targetColumn = cropColumn + j;

            // out of border pixels should be black
            if (targetRow >= height || targetColumn >= width)
            {
                newPixel = black;
            }
            else
            {
                newPixel = inputPixelHolder[targetRow][targetColumn];
            }

            cropPixelHolder[i][j] = newPixel;

            writePixel(newPixel, outputFile);
        }
    }

    // pixel array for the magnified output
    struct pixel **doublePixelHolder = malloc(cropHeight * 2 * sizeof(*doublePixelHolder));
    if (doublePixelHolder == NULL)
    {
        printf("Memory error\n");
        return 1;
    }

    for (int i = 0; i < cropHeight * 2; i++)
    {
        doublePixelHolder[i] = malloc(cropWidth * 2 * sizeof(*doublePixelHolder[i]));
        if (doublePixelHolder[i] == NULL)
        {
            printf("Memory error\n");
            return 1;
        }
    }

    // take from crop pixel array and fill the magnified pixel array
    for (int i = 0; i < cropHeight; i++)
    {

        for (int j = 0; j < cropWidth; j++)
        {
            struct pixel pixel = cropPixelHolder[i][j];

            // extend the pixel vertical, horizontal and diagonal
            doublePixelHolder[i * 2][j * 2] = pixel;
            doublePixelHolder[i * 2][j * 2 + 1] = pixel;
            doublePixelHolder[i * 2 + 1][j * 2] = pixel;
            doublePixelHolder[i * 2 + 1][j * 2 + 1] = pixel;
        }
    }

    // write the header of magnified output file
    writeHeader(magnifiedFile, imageType, bandNum, cropHeight * 2, cropWidth * 2, bitNum);

    // print the magnified output data to the file
    for (int i = 0; i < cropHeight * 2; i++)
    {
        for (int j = 0; j < cropWidth * 2; j++)
        {
            struct pixel pixel = doublePixelHolder[i][j];

            writePixel(pixel, magnifiedFile);
        }
    }

    // close the input and output files
    fclose(inputFile);
    fclose(outputFile);
    fclose(magnifiedFile);

    // free the allocated memories
    free(magnifiedOutputName);
    freeMem(inputPixelHolder, height, width);
    freeMem(cropPixelHolder, cropHeight, cropWidth);
    freeMem(doublePixelHolder, cropHeight * 2, cropWidth * 2);

    return 0;
}

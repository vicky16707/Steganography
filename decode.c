#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "common.h"
#include "types.h"

// Function to validate command line arguments for decoding
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo){
    char *ptr;

    // Check if the input file has .bmp extension
    ptr = strstr(argv[2], ".bmp");

    if(ptr != NULL && strcmp(ptr, ".bmp") == 0)
    {
        // Store the stego image filename
        decInfo->stego_image_fname = argv[2];
    }
    else{
        printf("ERROR :Invalid BMP file\n");
        return e_failure;
    }

    // Check if output filename is provided
    if(argv[3] != NULL)
    {
        decInfo->output_fname = argv[3];
    }
    else{
        // Use default output filename
        decInfo->output_fname = "output";
    }

    return e_success;
}

// Function to open the stego image file
Status open_decode_files(DecodeInfo *decInfo)
{
    // Open image in binary read mode
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");

    // Check if file opened successfully
    if(decInfo->fptr_stego_image == NULL){
        perror("fopen");
        return e_failure;
    }

    return e_success;
}

// Function to decode one byte from 8 LSBs
Status decode_byte_from_lsb(char *data, char *image_buffer){

    // Initialize decoded byte
    *data = 0;

    // Extract each LSB and reconstruct the byte
    for (int i = 0; i < 8; i++)
    {
        *data <<= 1;
        *data |= (image_buffer[i] & 1);
    }

    return e_success;
}

// Function to decode a 32-bit integer from LSBs
Status decode_size_from_lsb(int *size, char *image_buffer)

{
    // Initialize decoded size
    *size = 0;

    // Extract 32 bits from image buffer
    for (int i = 0; i < 32; i++)
    {
        *size <<= 1;
        *size |= (image_buffer[i] & 1);
    }

    return e_success;
}

/* Decode magic string */

// Function to decode and verify the magic string
Status decode_magic_string(DecodeInfo *decInfo){

    char image_buffer[8];
    char magic[strlen(MAGIC_STRING) + 1];
    int i;

    // Decode each character of the magic string
    for (i = 0; i < strlen(MAGIC_STRING); i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);
        decode_byte_from_lsb(&magic[i], image_buffer);
    }

    // Add string terminator
    magic[i] = '\0';

    // Compare with original magic string
    if (strcmp(magic, MAGIC_STRING) == 0)
    {
        return e_success;
    }

    printf("ERROR:magic string mismatch\n");
    return e_failure;
}

/* Decode secret file extension size */

// Function to decode extension size
Status decode_secret_file_extn_size(DecodeInfo *decInfo){

    char image_buffer[32];

    // Read 32 bytes containing extension size
    fread(image_buffer, 32, 1, decInfo->fptr_stego_image);

    // Decode extension size
    decode_size_from_lsb(&decInfo->extn_size, image_buffer);

    return e_success;
}

// Function to create output file
Status open_output_file(DecodeInfo *decInfo)
{
    // Use default output filename if user didn't provide one
    if(decInfo->output_fname == NULL)
    {
        strcpy(decInfo->output_file_name, "output");
    }
    else
    {
        // Copy user-given output filename
        strcpy(decInfo->output_file_name, decInfo->output_fname);
    }

    // Append decoded file extension
    strcat(decInfo->output_file_name, decInfo->extn_secret_file);

    // Open output file in binary write mode
    decInfo->fptr_output = fopen(decInfo->output_file_name, "wb");

    // Check if output file opened successfully
    if(decInfo->fptr_output == NULL)
    {
        perror("fopen");
        return e_failure;
    }

    return e_success;
}

/* Decode secret file extension */

// Function to decode secret file extension
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char image_buffer[8];

    // Decode extension character by character
    for (int i = 0; i < decInfo->extn_size; i++)
    {
       fread(image_buffer, 8, 1, decInfo->fptr_stego_image);
       decode_byte_from_lsb(&decInfo->extn_secret_file[i], image_buffer);
    }

    // Null terminate the extension string
    decInfo->extn_secret_file[decInfo->extn_size] = '\0';

    return e_success;
}

/* Decode secret file size */

// Function to decode secret file size
Status decode_secret_file_size(DecodeInfo *decInfo){

    char image_buffer[32];

    // Read encoded file size
    if(fread(image_buffer, 32, 1, decInfo->fptr_stego_image) != 1){
        return e_failure;
    }

    // Decode file size
    decode_size_from_lsb(&decInfo->size_secret_file, image_buffer);

    return e_success;
}

/* Decode secret file data */

// Function to decode and write secret file data
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char ch;

    // Decode each byte of secret file
    for(int i = 0; i < decInfo->size_secret_file; i++)
    {
        // Read 8 encoded bytes
        if(fread(image_buffer, 8, 1, decInfo->fptr_stego_image) != 1)
        {
            printf("ERROR: Unable to read image data\n");
            return e_failure;
        }

        // Decode one character
        decode_byte_from_lsb(&ch, image_buffer);

        // Write decoded character to output file
        fwrite(&ch, 1, 1, decInfo->fptr_output);
    }

    return e_success;
}

// Function to perform complete decoding process
Status do_decoding(DecodeInfo *decInfo){

    // Open the required files
    printf("INFO: Opening required files\n");

    if(open_decode_files(decInfo) == e_failure){
        printf("ERROR : Failed to open the files\n");
        return e_failure;
    }

    printf("INFO: files opened successfully.\n");

    // Skip BMP header
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    // Decode magic string
    printf("INFO :Decoding the magic string\n");

    if (decode_magic_string(decInfo) == e_failure)
    {
        printf("ERROR : Invalid Stego Image\n");
        return e_failure;
    }

    // Decode extension size
    printf("INFO : Decoding Extension Size...\n");

    if (decode_secret_file_extn_size(decInfo) == e_failure)
    {
        return e_failure;
    }

    // Decode extension
    printf("INFO : Decoding Extension...\n");

    if (decode_secret_file_extn(decInfo) == e_failure)
    {
        return e_failure;
    }

    // Create output file
    printf("INFO : Creating Output File...\n");

    if (open_output_file(decInfo) == e_failure)
    {
        return e_failure;
    }

    // Decode secret file size
    printf("INFO : Decoding Secret File Size...\n");

    if (decode_secret_file_size(decInfo) == e_failure)
    {
        return e_failure;
    }

    // Decode secret file contents
    printf("INFO : Decoding Secret File Data...\n");

    if (decode_secret_file_data(decInfo) == e_failure)
    {
        return e_failure;
    }

    // Display success message
    printf("INFO : Decoding Completed Successfully\n");

    // Closing opened files
    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output);

    return e_success;
}
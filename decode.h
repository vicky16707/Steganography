#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

/*
 * Structure to store all decoding information
 */
typedef struct _DecodeInfo
{
    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Output Secret File Info */
    char *output_fname;
    char output_file_name[50];
    FILE *fptr_output;

    /* Secret File Information */
    char extn_secret_file[10];
    int extn_size;
    int size_secret_file;

} DecodeInfo;


/* Read and validate decode arguments */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Open required files */
Status open_decode_files(DecodeInfo *decInfo);

/* Open output secret file */
Status open_output_file(DecodeInfo *decInfo);

/* Decode one character from 8 bytes */
Status decode_byte_from_lsb(char *data, char *image_buffer);

/* Decode a 32-bit integer from 32 bytes */
Status decode_size_from_lsb(int *size, char *image_buffer);

/* Decode magic string */
Status decode_magic_string(DecodeInfo *decInfo);

/* Decode secret file extension size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Decode secret file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo);

#endif
/* 
                    Project Title : LSB Image Steganography

Description :
     A C-based application that implements Least Significant Bit (LSB)
     Image Steganography to securely hide and retrieve secret files
     within 24-bit BMP images using bit manipulation and file handling.

Features:
    1. Encode secret file data into BMP image.
    2. Store secret file extension and size information.
    3. Decode hidden data from the stego image containing secret information.
    4. Maintain image quality after encoding.

Start Date : 17/7/26
End Date   : 26/7/26

Author     : Vignesh.N
Roll No    : 26010_020
*/

#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

// Function prototype to identify the requested operation
OperationType check_operation_type(char *argv);

int main(int argc, char *argv[])
{
     // Check whether sufficient command-line arguments are provided
     if (argc >= 4)
     {
          // Determine whether encoding or decoding is requested
          OperationType op = check_operation_type(argv[1]);

          // Perform encoding operation
          if(op == e_encode)
          {
               printf("INFO: Selected Encoding\n");

               // Structure to hold encoding information
               EncodeInfo encInfo;

               // Validate encoding arguments
               if(read_and_validate_encode_args(argv, &encInfo) == e_success)
               {
                   // Start encoding process
                   if(do_encoding(&encInfo) == e_success)
                   {
                        printf("INFO:Encoding completed successfully\n");
                   }
                   else
                   {
                        // Display error if encoding fails
                        printf("ERROR: Encoding Failed\n");
                        return e_failure;
                   }
               }
               else
               {
                    // Display error for invalid encoding arguments
                    printf("ERROR :Invalid Encoding Arguments\n");
                    return e_failure;
               }
          }

          // Perform decoding operation
          else if(op == e_decode)
          {
               printf("INFO: Selected Decoding\n");

               // Structure to hold decoding information
               DecodeInfo decInfo;

               // Validate decoding arguments
               if(read_and_validate_decode_args(argv, &decInfo) == e_success)
               {
                   // Start decoding process
                   if(do_decoding(&decInfo) == e_success)
                   {
                        printf("INFO:Decoding completed successfully\n");
                   }
                   else
                   {
                        // Display error if decoding fails
                        printf("ERROR: Decoding Failed\n");
                        return e_failure;
                   }
               }
               else
               {
                    // Display error for invalid decoding arguments
                    printf("ERROR :Invalid Decoding Arguments\n");
                    return e_failure;
               }
          }

          // Handle unsupported command-line option
          else
          {
               printf("ERROR:Unsupported Operation\n");
          }
     }
     else
     {
          // Display correct command usage when arguments are insufficient
          printf("Use the arguments like this:\n");
          printf("./a.out -e src.bmp secret.txt stego.bmp\n");
          printf("                 or                       \n");
          printf("./a.out -d stego.bmp output_filename\n");

          return e_failure;
     }

     // Return success after successful execution
     return e_success;
}

// Function to determine the operation type
OperationType check_operation_type(char *argv)
{
     // Check for encoding option
     if(strcmp(argv, "-e") == 0)
     {
          printf("Encode mode\n");
          return e_encode;
     }

     // Check for decoding option
     else if(strcmp(argv, "-d") == 0)
     {
          printf("Decode mode\n");
          return e_decode;
     }

     // Return unsupported for invalid option
     else
     {
          return e_unsupported;
     }
}
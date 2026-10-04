#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "common.h"
#include "types.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    uint size=ftell(fptr);
    rewind(fptr);
    return size;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
*/

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *ptr;
    //validating source image
    ptr=strstr(argv[2],".bmp");

    if(ptr!=NULL&&strcmp(ptr , ".bmp")==0)
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        printf("ERROR: source image should be .bmp\n");
        return e_failure;

    }

    //validating secret file
    ptr=strstr(argv[3],".txt");

    if(ptr !=NULL && strcmp(ptr , ".txt") == 0)
    {
        encInfo->secret_fname = argv[3];
        ptr = strrchr(encInfo->secret_fname, '.');
        strcpy(encInfo->extn_secret_file, ptr);
    }
    
    else
    {
        printf("ERROR: secret image should be .txt\n");
        return e_failure;

    }
    if(argv[4]!=NULL)
    {

        ptr = strstr(argv[4],".bmp");

        if(ptr != NULL && strcmp(ptr , ".bmp") == 0)
        {
            encInfo->dest_image_fname = argv[4];
        }
        else{
            printf("ERROR: destination image should be .bmp\n");
            return e_failure;
        }
    }
    else
    {
        encInfo->dest_image_fname="default.bmp";
    }
    return e_success;


}

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");

    // Do Error handling

    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");

        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file

    encInfo->fptr_dest_image = fopen(encInfo->dest_image_fname, "wb");

    // Do Error handling

    if (encInfo->fptr_dest_image == NULL)
    {
        perror("fopen");

        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->dest_image_fname);

        return e_failure;
    }

    // No failure return e_success

    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity =get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file=  get_file_size(encInfo->fptr_secret);
    if(encInfo->image_capacity > 16 + 32 + 32 + 32 + (encInfo->size_secret_file * 8)){
        return e_success;
    }
    else
        return e_failure;
    
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char header[54];

    rewind (fptr_src_image);

    fread(header,54,1,fptr_src_image);

    fwrite(header,54,1,fptr_dest_image);

    if(ftell(fptr_src_image)==54 && ftell(fptr_dest_image)==54)
    {
        return e_success;
    }
    return e_failure;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char imageBuffer[8];

    // Step 2 : Generate the lopp upto the length of the magic_string

    for(int i=0;i<strlen(magic_string);i++)
    {
        fread(imageBuffer,8,1,encInfo->fptr_src_image);

        encode_byte_to_lsb(magic_string[i] , imageBuffer);

        fwrite(imageBuffer,8,1,encInfo->fptr_dest_image);

    }
    if(ftell(encInfo->fptr_src_image)==ftell(encInfo->fptr_dest_image)){
  
        return e_success;
    }
    return e_failure;
}
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
     char imageBuffer[32];

        fread(imageBuffer, 32, 1,encInfo->fptr_src_image);

        encode_size_to_lsb(size , imageBuffer);
        

        fwrite(imageBuffer, 32, 1, encInfo->fptr_dest_image);

        return e_success;
    
   
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char image_Buffer[8];
    

     for(int i=0;i<strlen(file_extn);i++)
    {
        fread(image_Buffer,8,1,encInfo->fptr_src_image);

        encode_byte_to_lsb(file_extn[i] , image_Buffer);

        fwrite(image_Buffer,8,1,encInfo->fptr_dest_image);

    }
    
    
    return e_success;

}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char imageBuffer[32];

        fread(imageBuffer,32,1,encInfo->fptr_src_image);

        encode_size_to_lsb(file_size , imageBuffer);

        fwrite(imageBuffer,32,1,encInfo->fptr_dest_image);

        return e_success;

    // Step 1 : char imagBuffer[32];
    //  Step 2 : Read 32 bytes of data from src image and store in the image_buffer
    //  Step 3 : Call the enconde_size_to_lsb(file_size, image_buffer)
    //  Step 4 : Write imagebuffer in to the destination image

    // Step 5 : check the offset of both file pointer
    // true : return e_success;
    //  false : return e_failure
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;

    char image_buffer[8];

    rewind(encInfo->fptr_secret);

    while(fread(&ch,1,1,encInfo->fptr_secret)==1){

        fread(image_buffer,8,1,encInfo->fptr_src_image);

        encode_byte_to_lsb(ch , image_buffer);

        fwrite(image_buffer,8,1,encInfo->fptr_dest_image);

        
    }
    return e_success;
    
    // Step 3 : Read the secret data from secret.txt and store inside file_data

    // char image_buffer[54]   ;
    // Step 4 : Generate the loop upto the length of the encinfo->secret_file_size
    // Step 5 : Read 8 bytes of data from src image and store in the image_buffer
    // Step 6 : Call the enconde_byte_to_lsb(file_data[i] , image_buffer)
    // Step 7 : Write imagebuffer in to the destination image
    // Repeat the process untill the loop gets terminates

    // Step 8  check the offset of both file pointer
    // true : return e_success;
    //  false : return e_failure
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i=0;i<8;i++)
    {
        int bit=(data>>(7-i))&1;

        image_buffer[i]&=0xFE;  //Clear the lsb bit of imageBuffer[i]

        image_buffer[i]|=bit;  //Set the get bit in a lsb of imagebuffer[i]
    }
    return e_success;
}

Status encode_size_to_lsb(int size, char *imageBuffer)
{
    for(int i=0;i<32;i++)
    {
        int bit=(size>>(31-i))&1;

        imageBuffer[i]&=0xFE;//clear

        imageBuffer[i]|=bit;//set

    }
    return e_success;
    // Step 1 : Generate the loop for 32 times
    // Step 2 : Get the one bit from size
    // Step 3 : Clear the lsb bit of imageBuffer[i]
    // Step 4 : Set the get bit in a lsb of imagebuffer[i]
    // Repeat the process until the loop get's terminated
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    // Copy the remaining the data from source to destination
    char ch;
    while(fread(&ch,1,1,fptr_src)==1){
        fwrite(&ch,1,1,fptr_dest);
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    printf("INFO: Opening Required Files\n");

    if(open_files(encInfo) == e_failure)
    {
        printf("ERROR: Unable to open files\n");
        return e_failure;
    }
    printf("INFO: Files Opened Successfully\n");

    printf("INFO: Checking Image Capacity...\n");

    if(check_capacity(encInfo) == e_failure)
    {
        printf("ERROR: Image Doesn't Have enough Capacity\n");
        return e_failure;
    }
    printf("INFO: Capacity Check Passed\n");

    printf("INFO: Copying BMP Header\n");

    if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_dest_image) == e_failure)
    {
        printf("ERROR: copy_bmp_header() failed\n");
        return e_failure;
    }

    printf("INFO: Encoding Magic String\n");

    if(encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        printf("ERROR: encode_magic_string() failed\n");
        return e_failure;
    }

    

    printf("INFO: Encoding Extension Size\n");

    if(encode_secret_file_extn_size(strlen(encInfo->extn_secret_file),encInfo) == e_failure)
    {
        printf("ERROR: encode_secret_file_extn_size() failed\n");
        return e_failure;
    }

    printf("INFO: Extension Size Encoded Successfully\n");

    printf("INFO: Encoding Extension\n");

    if(encode_secret_file_extn(encInfo->extn_secret_file,
                               encInfo) == e_failure)
    {
        printf("ERROR: encode_secret_file_extn() failed\n");
        return e_failure;
    }

    printf("INFO: Extension Encoded Successfully\n");

    printf("INFO: Encoding Secret File Size\n");
    
    if(encode_secret_file_size(encInfo->size_secret_file,
                               encInfo) == e_failure)
    {
        printf("ERROR: encode_secret_file_size() failed\n");
        return e_failure;
    }

    printf("INFO: Secret File Size Encoded Successfully\n");

    printf("INFO: Encoding Secret File Data\n");

    if(encode_secret_file_data(encInfo) == e_failure)
    {
        printf("ERROR: encode_secret_file_data() failed\n");
        return e_failure;
    }

    printf("INFO: Secret File Data Encoded Successfully\n");

    printf("INFO: Copying Remaining Image Data\n");

    if(copy_remaining_img_data(encInfo->fptr_src_image,
                               encInfo->fptr_dest_image) == e_failure)
    {
        printf("ERROR: copy_remaining_img_data() failed\n");
        return e_failure;
    }

    printf("INFO: Remaining Data Copied Successfully\n");

    printf("INFO: Encoding Completed Successfully\n");

    return e_success;
}
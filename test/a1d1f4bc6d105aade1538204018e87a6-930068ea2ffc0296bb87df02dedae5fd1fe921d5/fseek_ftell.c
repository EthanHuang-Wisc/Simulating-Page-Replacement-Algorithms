#include<stdio.h>
#include<string.h>
#include<stdlib.h>
/*
fseek used to move file pointer where u want to move
ftell used to find is current location
location is defined by character by character 
like if file contains "Sudhanshu"
minimum position is 0
and max is len("Sudhanshu")
*/
/*  fseek(file pointer, offeset,  origin)
/*
origin will be :
SEEK_SET	Beginning of file
SEEK_CUR	Current position of the file pointer
SEEK_END	End of file *

*/
/* Prototypes */
char *ParseLine(char *arr, char *delimiter);

int main(){
  FILE *fp;
  char line[50];

  fp = fopen("data.txt","r");
  if(fp == NULL){
     printf("  Error in opening File !\n");
     exit(1);
  }
  //fetch a ParseLine
  fscanf(fp,"%s",line); //parse the fist line that has 21 character so it point to 21
  /* Printing position of file pointer */
  //posion is n if fp point to nth char of file
  printf("currrent posion of fp pointer in file : %ld\n", ftell(fp));

  printf("\nmovienf pointer to the end of FILE\n");
  fseek(fp, 0, SEEK_END);
  printf("currrent posion of fp pointer in file : %ld\n", ftell(fp));

  printf("\nmovienf pointer to the -5th postion from End\n");
  fseek(fp, -5, SEEK_END);
  printf("currrent posion of fp pointer in file : %ld\n", ftell(fp));


  printf("\nmovienf pointer to the begining of FILE\n");
  fseek(fp, 0, SEEK_SET);
  printf("currrent posion of fp pointer in file : %ld\n", ftell(fp));

  printf("\nmovienf pointer to the 5th postion from begining\n");
  fseek(fp, 5, SEEK_SET);
  printf("currrent posion of fp pointer in file : %ld\n", ftell(fp));



  fclose(fp);
  return 0;
}

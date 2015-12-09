#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int read_tag(char * buffer, size_t max, FILE * fp)
{
  int index = 0;
  char c;
  while(index < max && (c = fgetc(fp)) != '\0') {
    buffer[index] = c;
    ++index;
  }
  // Get rid of annoying space padding
  if (index == max && c == ' ') {
    int i;
    for(--index; buffer[index] == ' '; --index);
    buffer[index+1] = '\0';
  }
}

int find_start(FILE * fp)
{
  fseek(fp, -3, SEEK_END);
  long int end = ftell(fp);
  int max_read = 200;
  while(end - ftell(fp) < max_read) {
    char c;
    if((c = fgetc(fp)) == 'T') {
      if((c = fgetc(fp)) == 'A') {
        if((c = fgetc(fp)) == 'G') {
          return 0;
        }
        ungetc(c, fp);
      }
      ungetc(c, fp);
    }
    ungetc(c, fp);
    fseek(fp, -1, SEEK_CUR);
  }
  return 1;
}

int get_id3v1_tags(const char * file, char * title, char * artist, char * album)
{
  FILE *fp = fopen(file, "r");
  find_start(fp);
  size_t len = 30;
  char buffer[len];
  read_tag(title, len, fp);
  read_tag(artist, len, fp);
  read_tag(album, len, fp);
  printf("%s, %s, %s\n", title, artist, album);
  fclose(fp);
  return 0;
}

int main(int argc, char ** argv)
{
  char title[30];
  char artist[30];
  char album[30];
  if(argc < 2) {
    fprintf(stderr, "Must provide file path to read.\n");
    exit(1);
  }
  get_id3v1_tags(argv[1], title, artist, album);
  return 0;
}

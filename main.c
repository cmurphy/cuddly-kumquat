#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int read_id3v1_tag(char * buffer, size_t max, FILE * fp)
{
  int index = 0;
  while(index < max) {
    buffer[index] = fgetc(fp);
    ++index;
  }
  // Get rid of annoying space padding
  if (index == max && buffer[index-1] == ' ') {
    for(--index; buffer[index] == ' '; --index);
    buffer[index+1] = '\0';
  }
}

int find_id3v1_start(FILE * fp)
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

void get_id3v1_tags(const char * file, char * title, char * artist, char * album)
{
  FILE *fp = fopen(file, "r");
  find_id3v1_start(fp);
  size_t len = 30;
  read_id3v1_tag(title, len, fp);
  read_id3v1_tag(artist, len, fp);
  read_id3v1_tag(album, len, fp);
  printf("%s, %s, %s\n", title, artist, album);
  fclose(fp);
}

int find_frame_id(FILE *fp, const char * frame_id)
{
  const int max_search = 200;
  const int frame_id_length = 4;
  int current;
  int frame_id_byte_index = 0;
  while((current = ftell(fp)) < max_search && frame_id_byte_index < frame_id_length) {
    frame_id_byte_index = 0;
    char c;
    while((ftell(fp) - current) < frame_id_length && (c = fgetc(fp) == frame_id[frame_id_byte_index])) {
      ++frame_id_byte_index;
    }
  }
  if(current == max_search) {
    printf("Failed to find frame id %s.\n", frame_id);
    return 1;
  }
  return 0;
}

void eat_garbage(FILE *fp)
{
  fgetc(fp); fgetc(fp); // Eat flags
  fgetc(fp); fgetc(fp); fgetc(fp); // Eat encoding descriptor
}

void read_frame_body(FILE *fp, int size, char * buffer)
{
  int buffer_index = 0;
  int source_index = 0;
  while(source_index <= size) {
    char c = fgetc(fp);
    if(c != 0) {
      buffer[buffer_index++] = c;
    }
    ++source_index;
  }
  buffer[buffer_index] = 0;
}

void read_id3v23_tag(char * buffer, const char * tag, FILE *fp)
{
  int read = 0;
  int ind = 0;
  char c;
  int taglen = strlen(tag);
  int maxread = 200;
  int failed = find_frame_id(fp, tag);
  if(failed) {
    exit(1);
  }
  ind = 0;
  while(ind < 3) {
    fgetc(fp);
    ++ind;
    ++read;
  }
  int size = fgetc(fp) - 5;
  eat_garbage(fp);
  read_frame_body(fp, size, buffer);
}

void get_id3v23_tags(const char * file, char * title, char * artist, char * album)
{
  FILE *fp = fopen(file, "r");
  read_id3v23_tag(title, "TIT2", fp);
  read_id3v23_tag(artist, "TPE1", fp);
  read_id3v23_tag(album, "TALB", fp);
  printf("%s, %s, %s\n", title, artist, album);
  fclose(fp);
}

int is_id3v23(FILE *fp)
{
  fseek(fp, 0, SEEK_SET);
  char buffer[4];
  fgets(buffer, 4, fp);
  if (strcmp(buffer, "ID3") == 0) {
    if (fgetc(fp) == 3) {
      return 1;
    }
  }
  return 0;
}

int get_format(const char * file)
{
  int format = -1;
  FILE *fp = fopen(file, "r");
  if (is_id3v23(fp)) {
    format = 2;
  } else {
    format = 1;
  }
  fclose(fp);
  return format;
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
  int format = get_format(argv[1]);
  switch(format) {
    case 1:
      get_id3v1_tags(argv[1], title, artist, album);
      break;
    case 2:
      get_id3v23_tags(argv[1], title, artist, album);
      break;
  }
  return 0;
}

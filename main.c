#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 100

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
  fclose(fp);
}

int find_frame_id(FILE *fp, const char * frame_id, int frame_id_length)
{
  const int max_search = 2000;
  int current;
  int frame_id_byte_index = 0;
  char * chars_read = malloc(sizeof(char) * (frame_id_length + 1));
  while((current = ftell(fp)) < max_search && frame_id_byte_index < frame_id_length) {
    frame_id_byte_index = 0;
    char c;
    while((ftell(fp) - current) < frame_id_length && (c = fgetc(fp)) == frame_id[frame_id_byte_index]) {
      chars_read[frame_id_byte_index] = c;
      ++frame_id_byte_index;
    }
    // There was a partial false positive, so backtrack
    if(frame_id_byte_index > 0 && frame_id_byte_index < frame_id_length) {
      ungetc(c, fp);
      --frame_id_byte_index;
      while(frame_id_byte_index > 0) {
        ungetc(chars_read[frame_id_byte_index], fp);
        --frame_id_byte_index;
      }
    }
  }
  if(current == max_search) {
    //TODO: turn this into debug logging
    //printf("Failed to find frame id %s.\n", frame_id);
    return 1;
  }
  return 0;
}

int eat_garbage(FILE *fp)
{
  fgetc(fp); fgetc(fp); // Eat flags
  int is_unicode = fgetc(fp);
  if(is_unicode == 1) {
    fgetc(fp); fgetc(fp); // Eat encoding descriptor
    return 3;
  }
  return 1;
}

void read_frame_body(FILE *fp, int size, char * buffer)
{
  int buffer_index = 0;
  int source_index = 0;
  while(source_index < size) {
    char c = fgetc(fp);
    if(c != 0) {
      if(buffer_index <= BUFFER_SIZE) {
        buffer[buffer_index++] = c;
      } else {
        break;
      }
    }
    ++source_index;
  }
  buffer[buffer_index] = 0;
}

int read_id3v22_tag(char * buffer, const char * tag, FILE *fp)
{
  int failed = find_frame_id(fp, tag, 3);
  if(failed) {
    return 1;
  }
  int ind = 0;
  while(ind < 2) {
    fgetc(fp);
    ++ind;
  }
  int size = fgetc(fp) - 2;
  fgetc(fp);
  read_frame_body(fp, size, buffer);
  fseek(fp, 0, SEEK_SET);
  return 0;
}

int read_id3v23_tag(char * buffer, const char * tag, FILE *fp)
{
  int failed = find_frame_id(fp, tag, 4);
  if(failed) {
    return 1;
  }
  int ind = 0;
  while(ind < 3) {
    fgetc(fp);
    ++ind;
  }
  int size = fgetc(fp);
  size = size - eat_garbage(fp);
  read_frame_body(fp, size, buffer);
  fseek(fp, 0, SEEK_SET);
  return 0;
}

int read_id3v24_tag(char * buffer, const char * tag, FILE *fp)
{
  int failed = find_frame_id(fp, tag, 4);
  if(failed) {
    return 1;
  }
  int ind = 0;
  while(ind < 3) {
    fgetc(fp);
    ++ind;
  }
  int size = fgetc(fp);
  size = size - eat_garbage(fp);
  read_frame_body(fp, size, buffer);
  fseek(fp, 0, SEEK_SET);
  return 0;
}

int get_id3v22_tags(const char * file, char * title, char * artist, char * album)
{
  FILE *fp = fopen(file, "r");
  int failed = 1;
  failed &= read_id3v22_tag(title, "TT2", fp);
  failed &= read_id3v22_tag(artist, "TP1", fp);
  failed &= read_id3v22_tag(album, "TAL", fp);
  fclose(fp);
  return failed;
}

int get_id3v23_tags(const char * file, char * title, char * artist, char * album)
{
  FILE *fp = fopen(file, "r");
  int failed = 1;
  failed &= read_id3v23_tag(title, "TIT2", fp);
  failed &= read_id3v23_tag(artist, "TPE1", fp);
  failed &= read_id3v23_tag(album, "TALB", fp);
  fclose(fp);
  return failed;
}

int get_id3v24_tags(const char * file, char * title, char * artist, char * album)
{
  FILE *fp = fopen(file, "r");
  int failed = 1;
  failed &= read_id3v24_tag(title, "TIT2", fp);
  failed &= read_id3v24_tag(artist, "TPE1", fp);
  failed &= read_id3v24_tag(album, "TALB", fp);
  fclose(fp);
  return failed;
}

int find_atom(FILE * fp, const char * atom_name, int parent_size)
{
  char buffer[5];
  int start = ftell(fp);
  int size;
  while (1) {
    fread(&size, sizeof(int), 1, fp);
    size = ntohl(size);
    if (size == 0) {
      continue;
    }
    fread(buffer, sizeof(char), 4, fp);
    if (! strncmp(buffer, atom_name, 4)) {
      break;
    } else {
      fseek(fp, size - 8, SEEK_CUR);
    }
    if (ftell(fp) - start > parent_size) {
      return 1;
    }
  }
  return size;
}

int seek_ilst(FILE * fp)
{
  int max_scan = 64;
  int moov_size = find_atom(fp, "moov", max_scan);
  int udta_size = find_atom(fp, "udta", moov_size);
  int meta_size = find_atom(fp, "meta", udta_size);
  int ilst_size = find_atom(fp, "ilst", meta_size);
  return ilst_size;
}

int read_ilst_tag(char * buffer, const char * tag, int ilst_size, FILE * fp)
{
  char atom_name[5] = "\0";
  find_atom(fp, tag, ilst_size);
  // read data
  int size;
  fread(&size, sizeof(int), 1, fp);
  size = ntohl(size);
  fread(atom_name, sizeof(char), 4, fp);
  if (strncmp(atom_name, "data", 4)) {
    printf("No data element!\n");
    exit(1);
  }
  // skip 00 00 00 01 00 00 00 00
  fseek(fp, 8, SEEK_CUR);
  read_frame_body(fp, size - 16, buffer);
}

int get_mp4_tags(const char * file, char * title, char * artist, char * album)
{
  FILE *fp = fopen(file, "r");
  int failed = 1;
  int ilst_size = seek_ilst(fp);
  failed &= read_ilst_tag(title, "\xa9nam", ilst_size, fp);
  failed &= read_ilst_tag(artist, "\xa9""ART", ilst_size, fp);
  failed &= read_ilst_tag(album, "\xa9""alb", ilst_size, fp);
  return failed;
}

int is_id3(FILE *fp)
{
  fseek(fp, 0, SEEK_SET);
  char buffer[4];
  fgets(buffer, 4, fp);
  if (strcmp(buffer, "ID3") == 0) {
    return 1;
  }
  return 0;
}

int is_id3v22(FILE *fp)
{
  return is_id3(fp) && (fgetc(fp) == 2);
}

int is_id3v23(FILE *fp)
{
  return is_id3(fp) && (fgetc(fp) == 3);
}

int is_id3v24(FILE *fp)
{
  return is_id3(fp) && (fgetc(fp) == 4);
}

int compare_extension(const char * file, const char * ext)
{

  int string_length = strlen(file);
  int ext_length = strlen(ext);
  return strncmp(file + string_length - ext_length, ext, ext_length);
}

int is_mp3(const char * file)
{
  return compare_extension(file, ".mp3") == 0;
}

int is_mp4(const char * file)
{
  return compare_extension(file, ".mp4") == 0 || compare_extension(file, ".m4a") == 0;
}

int get_format(const char * file)
{
  int format = -1;
  FILE *fp = fopen(file, "r");
  // todo - check is_id3 first
  // todo - use consts for format ids
  if (is_mp3(file)) {
    if (is_id3v22(fp)) {
      format = 22;
    } else if (is_id3v23(fp)) {
      format = 23;
    } else if (is_id3v24(fp)) {
      format = 24;
    } else {
      format = 1;
    }
  } else if (is_mp4(file)) {
    format = 4;
  } else {
    printf("Could not recognize this file.\n");
    exit(1);
  }
  fclose(fp);
  return format;
}

int main(int argc, char ** argv)
{
  char title[BUFFER_SIZE] = "title not found";
  char artist[BUFFER_SIZE] = "artist not found";
  char album[BUFFER_SIZE] = "album not found";
  if(argc < 2) {
    fprintf(stderr, "Must provide file path to read.\n");
    exit(1);
  }
  int format = get_format(argv[1]);
  int failed = 0;
  switch(format) {
    case 1:
      get_id3v1_tags(argv[1], title, artist, album);
      break;
    case 22:
      failed = get_id3v22_tags(argv[1], title, artist, album);
      if(failed) {
        get_id3v1_tags(argv[1], title, artist, album);
      }
      break;
    case 23:
      failed = get_id3v23_tags(argv[1], title, artist, album);
      if(failed) {
        get_id3v1_tags(argv[1], title, artist, album);
      }
      break;
    case 24:
      failed = get_id3v24_tags(argv[1], title, artist, album);
      if(failed) {
        get_id3v1_tags(argv[1], title, artist, album);
      }
      break;
    case 4:
      failed = get_mp4_tags(argv[1], title, artist, album);
      break;
  }
  printf("%s, %s, %s\n", title, artist, album);
  return 0;
}

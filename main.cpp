#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "file.h"
#include "song.h"

#define BUFFER_SIZE 100

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
  if (! find_atom(fp, tag, ilst_size)) {
    return 1;
  }
  // read data
  int size;
  fread(&size, sizeof(int), 1, fp);
  size = ntohl(size);
  fread(atom_name, sizeof(char), 4, fp);
  if (strncmp(atom_name, "data", 4)) {
    return 1;
  }
  // skip 00 00 00 01 00 00 00 00
  fseek(fp, 8, SEEK_CUR);
  read_frame_body(fp, size - 16, buffer);
  return 0;
}

int get_mp4_tags(const char * file, char * title, char * artist, char * album)
{
  File * song_file = new File(file);
  FILE *fp = song_file->get_file_pointer();
  int failed = 1;
  int ilst_size = seek_ilst(fp);
  failed &= read_ilst_tag(title, "\xa9nam", ilst_size, fp);
  failed &= read_ilst_tag(artist, "\xa9""ART", ilst_size, fp);
  failed &= read_ilst_tag(album, "\xa9""alb", ilst_size, fp);
  delete song_file;
  return failed;
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
  File * media_file = new File(argv[1]);
  MetadataFormat format = media_file->metadata_type();
  Song * song;
  int failed = 0;
  switch(format) {
    case MetadataFormat::ID3V1:
      song = new Id3v1(media_file);
      song->read_frames(title, artist, album);
      break;
    case MetadataFormat::ID3V2_2:
      song = new Id3v2_2(media_file);
      failed = song->read_frames(title, artist, album);
      if(failed) {
        delete song;
        song = new Id3v1(media_file);
        song->read_frames(title, artist, album);
      }
      break;
    case MetadataFormat::ID3V2_3:
      song = new Id3v2_3(media_file);
      failed = song->read_frames(title, artist, album);
      if(failed) {
        song = new Id3v1(media_file);
        song->read_frames(title, artist, album);
      }
      break;
    case MetadataFormat::ID3V2_4:
      song = new Id3v2_4(media_file);
      failed = song->read_frames(title, artist, album);
      if(failed) {
        song = new Id3v1(media_file);
        song->read_frames(title, artist, album);
      }
      break;
    case MetadataFormat::MPEG4:
      failed = get_mp4_tags(argv[1], title, artist, album);
      break;
    case MetadataFormat::ERROR:
      printf("Could not recognize this file.\n");
      exit(1);
  }
  printf("%s, %s, %s\n", title, artist, album);
  delete media_file;
  delete song;
  return 0;
}

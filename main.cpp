#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "file.h"
#include "song.h"

#define BUFFER_SIZE 100

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
      song = new Mp4(media_file);
      failed = song->read_frames(title, artist, album);
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

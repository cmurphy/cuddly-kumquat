#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "file.h"
#include "song.h"

#define BUFFER_SIZE 100

int main(int argc, char ** argv)
{
  if(argc < 2) {
    fprintf(stderr, "Must provide file path to read.\n");
    exit(1);
  }
  std::string file_name = argv[1];
  File * media_file = new File(file_name);
  MetadataFormat format = media_file->metadata_type();
  Song * song;
  switch(format) {
    case MetadataFormat::ID3V1:
      song = new Id3v1(media_file);
      break;
    case MetadataFormat::ID3V2_2:
      song = new Id3v2_2(media_file);
      break;
    case MetadataFormat::ID3V2_3:
      song = new Id3v2_3(media_file);
      break;
    case MetadataFormat::ID3V2_4:
      song = new Id3v2_4(media_file);
      break;
    case MetadataFormat::MPEG4:
      song = new Mp4(media_file);
      break;
    case MetadataFormat::ERROR:
      printf("Could not recognize this file.\n");
      exit(1);
  }
  int failed = 0;
  failed = song->read_frames();
  song->print();
  delete media_file;
  delete song;
  return failed;
}

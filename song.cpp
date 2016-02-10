#include "song.h"
#include <string.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 100

Song::Song(File * media_file)
{
  this->media_file = media_file;
}

Song::~Song()
{

}

int Song::read_frame_body(std::string & body, int size)
{
  char * buffer = new char[size+1];
  FILE * fp = (this->media_file)->get_file_pointer();
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
  // Get rid of annoying space padding
  if (buffer_index == size && buffer[buffer_index-1] == ' ') {
    for(--buffer_index; buffer[buffer_index] == ' '; --buffer_index);
    ++buffer_index;
  }
  buffer[buffer_index] = 0;
  body = buffer;
  delete[] buffer;
  return 0;
}

int Id3v1::read_frames(char * title, char * artist, char * album)
{
  size_t len = 30;
  this->read_frame_body(this->title, len);
  this->read_frame_body(this->artist, len);
  this->read_frame_body(this->album, len);
  strcpy(title, this->title.c_str());
  strcpy(artist, this->artist.c_str());
  strcpy(album, this->album.c_str());
  return 0;
}

int Id3v2::eat_garbage()
{
  FILE * fp = (this->media_file)->get_file_pointer();
  fgetc(fp); fgetc(fp); // Eat flags
  int is_unicode = fgetc(fp);
  if(is_unicode == 1) {
    fgetc(fp); fgetc(fp); // Eat encoding descriptor
    return 3;
  }
  return 1;
}

int Id3v2::eat_frame_header(const char * frame_id, int frame_id_length)
{
  FILE * fp = (this->media_file)->get_file_pointer();
  int start = ftell(fp);
  const int max_search = 500000;
  int current;
  int frame_id_byte_index = 0;
  char * chars_read = (char*)malloc(sizeof(char) * (frame_id_length + 1));
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
    fseek(fp, start, SEEK_SET);
    return 1;
  }
  return 0;
}

int Id3v2_2::read_frame(char * buffer, const char * tag)
{
  FILE * fp = (this->media_file)->get_file_pointer();
  int failed = this->eat_frame_header(tag, 3);
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
  std::string body;
  this->read_frame_body(body, size);
  strcpy(buffer, body.c_str());
  fseek(fp, 0, SEEK_SET);
  return 0;
}

int Id3v2_3::read_frame(char * buffer, const char * tag)
{
  FILE * fp = (this->media_file)->get_file_pointer();
  int failed = this->eat_frame_header(tag, 4);
  if(failed) {
    return 1;
  }
  int ind = 0;
  while(ind < 3) {
    fgetc(fp);
    ++ind;
  }
  int size = fgetc(fp);
  size = size - this->eat_garbage();
  std::string body;
  this->read_frame_body(body, size);
  strcpy(buffer, body.c_str());
  fseek(fp, 0, SEEK_SET);
  return 0;
}

int Id3v2_4::read_frame(char * buffer, const char * tag)
{
  FILE * fp = (this->media_file)->get_file_pointer();
  int failed = this->eat_frame_header(tag, 4);
  if(failed) {
    return 1;
  }
  int ind = 0;
  while(ind < 3) {
    fgetc(fp);
    ++ind;
  }
  int size = fgetc(fp);
  size = size - this->eat_garbage();
  std::string body;
  this->read_frame_body(body, size);
  strcpy(buffer, body.c_str());
  fseek(fp, 0, SEEK_SET);
  return 0;
}

int Id3v2_2::read_frames(char * title, char * artist, char * album)
{
  int failed = 1;
  failed &= this->read_frame(title, "TT2");
  failed &= this->read_frame(artist, "TP1");
  failed &= this->read_frame(album, "TAL");
  return failed;
}

int Id3v2_3::read_frames(char * title, char * artist, char * album)
{
  int failed = 1;
  failed &= this->read_frame(title, "TIT2");
  failed &= this->read_frame(artist, "TPE1");
  failed &= this->read_frame(album, "TALB");
  return failed;
}

int Id3v2_4::read_frames(char * title, char * artist, char * album)
{
  int failed = 1;
  failed &= this->read_frame(title, "TIT2");
  failed &= this->read_frame(artist, "TPE1");
  failed &= this->read_frame(album, "TALB");
  return failed;
}

int Mp4::find_atom(const char * atom_name, int parent_size)
{
  FILE *fp = (this->media_file)->get_file_pointer();
  char buffer[5];
  int start = ftell(fp);
  int size;
  while (1) {
    fread(&size, sizeof(int), 1, fp);
    if (feof(fp)) {
      return 1;
    }
    size = ntohl(size);
    if (size == 0) {
      continue;
    }
    fread(buffer, sizeof(char), 4, fp);
    if (feof(fp)) {
      return 1;
    }
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

int Mp4::seek_ilst()
{
  int max_scan = 64;
  int moov_size = this->find_atom("moov", max_scan);
  int udta_size = this->find_atom("udta", moov_size);
  int meta_size = this->find_atom("meta", udta_size);
  int ilst_size = this->find_atom("ilst", meta_size);
  return ilst_size;
}

int Mp4::read_frame(char * buffer, const char * tag, int ilst_size)
{
  char atom_name[5] = "\0";
  FILE *fp = (this->media_file)->get_file_pointer();
  if (! this->find_atom(tag, ilst_size)) {
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
  std::string body;
  this->read_frame_body(body, size - 16);
  strcpy(buffer, body.c_str());
  return 0;
}

int Mp4::read_frames(char * title, char * artist, char * album)
{
  int failed = 1;
  int ilst_size = this->seek_ilst();
  failed &= read_frame(title, "\xa9nam", ilst_size);
  failed &= read_frame(artist, "\xa9""ART", ilst_size);
  failed &= read_frame(album, "\xa9""alb", ilst_size);
  return failed;
}

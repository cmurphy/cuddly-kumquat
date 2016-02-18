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

int Song::read_frame_body(std::string & body, unsigned int size)
{
  std::string backup = body;
  body = "";
  FILE * fp = (this->media_file)->get_file_pointer();
  unsigned int source_index = 0;
  while(source_index < size) {
    char c = fgetc(fp);
    if(feof(fp)) {
      body = backup;
      return 1;
    }
    if(c != 0) {
      if(body.length() <= BUFFER_SIZE) {
        body += c;
      } else {
        break;
      }
    }
    ++source_index;
  }
  // Get rid of annoying space padding
  if (body.length() == size) {
    while (body.back() == ' ') {
      body.pop_back();
    }
  }
  return 0;
}

int Song::read_frames()
{
  int failed = 0;
  failed |= this->read_frame(this->title, this->title_identifier());
  failed |= this->read_frame(this->artist, this->artist_identifier());
  failed |= this->read_frame(this->album, this->album_identifier());
  return failed;
}

void Song::print()
{
  printf("%s, %s, %s\n", this->title.c_str(), this->artist.c_str(), this->album.c_str());
}

int Id3v1::read_frame(std::string & buffer, const std::string & tag)
{
  int failed = this->read_frame_body(buffer, 30);
  return failed;
}

int Id3v2::eat_frame_header(const std::string & frame_id)
{
  FILE * fp = (this->media_file)->get_file_pointer();
  int start = ftell(fp);
  const int max_search = 500000;
  int frame_id_length = frame_id.length();
  int current;
  int frame_id_byte_index = 0;
  std::string chars_read;
  while((current = ftell(fp)) < max_search && frame_id_byte_index < frame_id_length) {
    frame_id_byte_index = 0;
    char c;
    while((ftell(fp) - current) < frame_id_length && (c = fgetc(fp)) == frame_id[frame_id_byte_index]) {
      chars_read += c;
      ++frame_id_byte_index;
    }
    // There was a partial false positive, so backtrack
    if(frame_id_byte_index > 0 && frame_id_byte_index < frame_id_length) {
      ungetc(c, fp);
      --frame_id_byte_index;
      while(frame_id_byte_index > 0) {
        ungetc(chars_read.back(), fp);
        chars_read.pop_back();
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

int Id3v2_2::get_frame_size(FILE * fp)
{
  int size = 0;
  fread(&size, 3, 1, fp);
  size = ntohl(size);
  size >>= 8;
  return size;
}

void Id3v2_2::get_frame_flags(FILE * fp)
{
  // ID3v2.2 does not have any flag bytes
  return;
}

int Id3v2_2::get_unicode_encoding(FILE * fp)
{
  // Read either 00 or 01 indicating non-Unicode or Unicode
  // We don't do anything with this information for now
  fgetc(fp);
  return 1;
}

int Id3v2_3::get_frame_size(FILE * fp)
{
  int size = 0;
  fread(&size, 4, 1, fp);
  size = ntohl(size);
  return size;
}

void Id3v2_3::get_frame_flags(FILE * fp)
{
  // ID3v2.3 has two flag bytes
  // We don't do anything with this information for now
  fgetc(fp); fgetc(fp);
}

int Id3v2_3::get_unicode_encoding(FILE * fp)
{
  int is_unicode = fgetc(fp);
  // If we're in Unicode, there are two more bytes indicating the unicode BOM
  if(is_unicode == 1) {
    fgetc(fp); fgetc(fp);
    return 3;
  }
  return 1;
}

int Id3v2_4::get_frame_size(FILE * fp)
{
  int size = 0;
  fread(&size, 4, 1, fp);
  size = ntohl(size);
  return size;
}

void Id3v2_4::get_frame_flags(FILE * fp)
{
  // ID3v2.3 has two flag bytes
  // We don't do anything with this information for now
  fgetc(fp); fgetc(fp);
}

int Id3v2_4::get_unicode_encoding(FILE * fp)
{
  int is_unicode = fgetc(fp);
  // If we're in Unicode, there are two more bytes indicating the unicode BOM
  if(is_unicode == 1) {
    fgetc(fp); fgetc(fp);
    return 3;
  }
  return 1;
}

int Id3v2::read_frame(std::string & buffer, const std::string & tag)
{
  FILE * fp = (this->media_file)->get_file_pointer();
  int failed = this->eat_frame_header(tag.c_str());
  if(failed) {
    return 1;
  }
  int size = this->get_frame_size(fp);
  this->get_frame_flags(fp);
  size -= this->get_unicode_encoding(fp);
  failed = this->read_frame_body(buffer, size);
  if(failed) {
    return 1;
  }
  fseek(fp, 0, SEEK_SET);
  return 0;
}

int Mp4::find_atom(const std::string & atom_name, int parent_size)
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
    if (! strncmp(buffer, atom_name.c_str(), 4)) {
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

int Mp4::read_frame(std::string & buffer, const std::string & tag)
{
  char atom_name[5] = "\0";
  FILE *fp = (this->media_file)->get_file_pointer();
  if (! this->find_atom(tag, this->ilst_size)) {
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
  int failed = this->read_frame_body(buffer, size - 16);
  if(failed) {
    return 1;
  }
  return 0;
}

int Mp4::read_frames()
{
  this->ilst_size = this->seek_ilst();
  return Song::read_frames();
}

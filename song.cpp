#include "song.h"

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
  std::ifstream * fs = this->media_file->get_file_stream();
  unsigned int source_index = 0;
  while(source_index < size) {
    char c = fs->get();
    if(fs->eof()) {
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
  std::cout << this->title.c_str() << ", " <<  this->artist.c_str() << ", " << this->album.c_str() << std::endl;
}

int Id3v1::read_frame(std::string & buffer, const std::string & tag)
{
  int failed = this->read_frame_body(buffer, 30);
  return failed;
}

int Id3v2::eat_frame_header(const std::string & frame_id)
{
  std::ifstream * fs = this->media_file->get_file_stream();
  int start = fs->tellg();
  const int max_search = 500000;
  int frame_id_length = frame_id.length();
  int frame_id_byte_index = 0;
  std::string chars_read;
  int current = fs->tellg();
  while((current) < max_search && frame_id_byte_index < frame_id_length) {
    frame_id_byte_index = 0;
    int frame_id_bytes_read = 0;
    char c;
    while((frame_id_bytes_read) < frame_id_length) {
      fs->get(c);
      if((c) == frame_id[frame_id_byte_index]) {
        chars_read += c;
        ++frame_id_byte_index;
        frame_id_bytes_read = (int)fs->tellg() - current;
      } else {
        break;
      }
    }
    // There was a partial false positive, so backtrack
    if(frame_id_byte_index > 0 && frame_id_byte_index < frame_id_length) {
      fs->putback(c);
      --frame_id_byte_index;
      while(frame_id_byte_index > 0) {
        fs->putback(chars_read.back());
        chars_read.pop_back();
        --frame_id_byte_index;
      }
    }
    current = fs->tellg();
  }
  if(current == max_search) {
    //TODO: turn this into debug logging
    //printf("Failed to find frame id %s.\n", frame_id);
    fs->seekg(start, std::ios::beg);
    return 1;
  }
  return 0;
}

int Id3v2_2::get_frame_size()
{
  int size = 0;
  std::ifstream * fs = this->media_file->get_file_stream();
  fs->read((char*)&size, 3);
  size = ntohl(size);
  size >>= 8;
  return size;
}

void Id3v2_2::get_frame_flags()
{
  // ID3v2.2 does not have any flag bytes
  return;
}

int Id3v2_2::get_unicode_encoding()
{
  // Read either 00 or 01 indicating non-Unicode or Unicode
  // We don't do anything with this information for now
  std::ifstream * fs = this->media_file->get_file_stream();
  fs->get();
  return 1;
}

int Id3v2_3::get_frame_size()
{
  int size = 0;
  std::ifstream * fs = this->media_file->get_file_stream();
  fs->read((char*)&size, 4);
  size = ntohl(size);
  return size;
}

void Id3v2_3::get_frame_flags()
{
  // ID3v2.3 has two flag bytes
  // We don't do anything with this information for now
  std::ifstream * fs = this->media_file->get_file_stream();
  fs->get(); fs->get();
}

int Id3v2_3::get_unicode_encoding()
{
  std::ifstream * fs = this->media_file->get_file_stream();
  int is_unicode = fs->get();
  // If we're in Unicode, there are two more bytes indicating the unicode BOM
  if(is_unicode == 1) {
    fs->get(); fs->get();
    return 3;
  }
  return 1;
}

int Id3v2_4::get_frame_size()
{
  int size = 0;
  std::ifstream * fs = this->media_file->get_file_stream();
  fs->read((char*)&size, 4);
  size = ntohl(size);
  return size;
}

void Id3v2_4::get_frame_flags()
{
  // ID3v2.3 has two flag bytes
  // We don't do anything with this information for now
  std::ifstream * fs = this->media_file->get_file_stream();
  fs->get(); fs->get();
}

int Id3v2_4::get_unicode_encoding()
{
  std::ifstream * fs = this->media_file->get_file_stream();
  int is_unicode = fs->get();
  // If we're in Unicode, there are two more bytes indicating the unicode BOM
  if(is_unicode == 1) {
    fs->get(); fs->get();
    return 3;
  }
  return 1;
}

int Id3v2::read_frame(std::string & buffer, const std::string & tag)
{
  int failed = this->eat_frame_header(tag.c_str());
  if(failed) {
    return 1;
  }
  int size = this->get_frame_size();
  this->get_frame_flags();
  size -= this->get_unicode_encoding();
  failed = this->read_frame_body(buffer, size);
  if(failed) {
    return 1;
  }
  this->media_file->get_file_stream()->seekg(0, std::ios::beg);
  return 0;
}

int Mp4::find_atom(const std::string & atom_name, int parent_size)
{
  std::ifstream * fs = this->media_file->get_file_stream();
  char buffer[5];
  int start = fs->tellg();
  int size;
  while (1) {
    fs->read((char*)&size, sizeof(int));
    if (fs->eof()) {
      return 1;
    }
    size = ntohl(size);
    if (size == 0) {
      continue;
    }
    fs->read(buffer, 4);
    if (fs->eof()) {
      return 1;
    }
    if (! strncmp(buffer, atom_name.c_str(), 4)) {
      break;
    } else {
      fs->seekg(size - 8, std::ios::cur);
    }
    if ((int)fs->tellg() - start > parent_size) {
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
  std::ifstream * fs = this->media_file->get_file_stream();
  if (! this->find_atom(tag, this->ilst_size)) {
    return 1;
  }
  // read data
  int size;
  fs->read((char*)&size, sizeof(int));
  size = ntohl(size);
  fs->read(atom_name, sizeof(int));
  if (strncmp(atom_name, "data", 4)) {
    return 1;
  }
  // skip 00 00 00 01 00 00 00 00
  fs->seekg(8, std::ios::cur);
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

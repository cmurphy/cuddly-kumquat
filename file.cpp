#include <string.h>
#include "file.h"

File::File(const char * file_path)
{
  this->file_path = file_path;
  this->file_pointer = fopen(file_path, "r");
}

File::File(std::string & file_path)
{
  this->file_path = file_path;
  this->file_pointer = fopen(file_path.c_str(), "r");
}

File::~File()
{
  fclose(this->file_pointer);
}

FILE * File::get_file_pointer()
{
  return this->file_pointer;
}

int File::compare_extension(const std::string & extension)
{

  int string_length = (this->file_path).length();
  int extension_length = extension.length();
  int pos = string_length - extension_length;
  return (this->file_path).compare(pos, string_length, extension);
}

Container File::container_type()
{
  if (compare_extension(".mp3") == 0) {
    return Container::MP3;
  } else if (compare_extension(".mp4") == 0 || compare_extension(".m4a") == 0) {
    return Container::MP4;
  } else {
    return Container::ERROR;
  }
}

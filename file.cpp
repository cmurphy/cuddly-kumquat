#include "file.h"
#include <string.h>

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

MetadataFormat File::metadata_type()
{
  Container container = this->container_type();
  if (container == Container::MP3) {
    fseek(this->get_file_pointer(), 0, SEEK_SET);
    char buffer[5];
    fgets(buffer, 5, this->get_file_pointer());
    if (strncmp(buffer, "ID3", 3) == 0) {
      if (buffer[3] == 2) {
        return MetadataFormat::ID3V2_2;
      } else if (buffer[3] == 3) {
        return MetadataFormat::ID3V2_3;
      } else if (buffer[3] == 4) {
        return MetadataFormat::ID3V2_4;
      } else {
        return MetadataFormat::ERROR;
      }
    } else {
      // TODO: check for TAG at SEEK_END-128
      return MetadataFormat::ID3V1;
    }
  } else if (container == Container::MP4) {
    return MetadataFormat::MPEG4;
  } else {
    return MetadataFormat::ERROR;
  }
}

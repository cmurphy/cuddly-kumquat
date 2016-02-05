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

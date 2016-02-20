#ifndef FILE_H
#define FILE_H

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>

enum class Container {
  ERROR = -1,
  MP3,
  MP4
};

enum class MetadataFormat {
  ERROR = -1,
  ID3V1,
  ID3V2_2,
  ID3V2_3,
  ID3V2_4,
  MPEG4
};

class File
{
  private:
    std::ifstream file_stream;
    std::string file_path;
    unsigned char * format_header;
    int compare_extension(const std::string & extension);
  public:
    explicit File(const std::string & file_path);
    ~File();
    Container container_type();
    MetadataFormat metadata_type();
    std::ifstream * get_file_stream();
};
#endif

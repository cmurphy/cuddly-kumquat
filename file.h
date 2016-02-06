#ifndef FILE_H
#define FILE_H

#include <iostream>
#include <string>

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
    FILE * file_pointer;
    std::string file_path;
    unsigned char * format_header;
    int compare_extension(const std::string & extension);
  public:
    explicit File(const char * file_path);
    explicit File(std::string & file_path);
    ~File();
    Container container_type();
    MetadataFormat metadata_type();
    FILE * get_file_pointer();
};
#endif

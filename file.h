#include <iostream>
#include <string>

enum class Container { ERROR = -1, MP3, MP4 };

class File
{
  private:
    FILE * file_pointer;
    std::string file_path;
    unsigned char * format_header;
    int compare_extension(const std::string & extension);
  public:
    File(const char * file_path);
    File(std::string & file_path);
    ~File();
    Container container_type();
    int metadata_type();
    FILE * get_file_pointer();
};

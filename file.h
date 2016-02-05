#include <iostream>
#include <string>

class File
{
  protected:
    FILE * file_pointer;
    std::string file_path;
    unsigned char * format_header;
  public:
    File(const char * file_path);
    File(std::string & file_path);
    ~File();
    int container_type();
    int metadata_type();
    FILE * get_file_pointer();
};

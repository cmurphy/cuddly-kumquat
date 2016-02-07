#ifndef SONG_H
#define SONG_H

#include "file.h"

class Song
{
  protected:
    std::string title;
    std::string artist;
    std::string album;
    virtual int read_frame_body(std::string & buffer, int size) = 0;
  public:
    File * media_file;
    explicit Song(File * media_file);
    virtual ~Song();
    //virtual int eat_frame_header() = 0;
    virtual int read_frames(char * title, char * artist, char * album) = 0;
    void print();

};

class Id3v1: public Song
{
  private:
    int find_start();
  protected:
    int read_frame_body(std::string & buffer, int size);
  public:
    ~Id3v1() {};
    explicit Id3v1(File * media_file): Song(media_file) {};
    //int eat_frame_header(const char * frame_id, int frame_id_length);
    int read_frames(char * title, char * artist, char * album);
};

class Id3v2: public Song
{
  protected:
    int eat_garbage();
    int read_frame_body(std::string & buffer, int size);
  public:
    explicit Id3v2(File * media_file): Song(media_file) {};
    ~Id3v2() {};
    int eat_frame_header(const char * frame_id, int frame_id_length);
    virtual int read_frame(char * buffer, const char * tag) = 0;
    virtual int read_frames(char * title, char * artist, char * album) = 0;
};

class Id3v2_2: public Id3v2
{
  public:
    explicit Id3v2_2(File * media_file): Id3v2(media_file) {};
    ~Id3v2_2() {};
    int read_frame(char * buffer, const char * tag);
    int read_frames(char * title, char * artist, char * album);
};

class Id3v2_3: public Id3v2
{
  public:
    explicit Id3v2_3(File * media_file): Id3v2(media_file) {};
    ~Id3v2_3() {};
    int read_frame(char * buffer, const char * tag);
    int read_frames(char * title, char * artist, char * album);
};

class Id3v2_4: public Id3v2
{
  public:
    explicit Id3v2_4(File * media_file): Id3v2(media_file) {};
    ~Id3v2_4() {};
    int read_frame(char * buffer, const char * tag);
    int read_frames(char * title, char * artist, char * album);
};
#endif

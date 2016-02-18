#ifndef SONG_H
#define SONG_H

#include "file.h"

class Song
{
  protected:
    std::string title = "title not found";
    std::string artist = "artist not found";
    std::string album = "album not found";
    virtual std::string title_identifier() = 0;
    virtual std::string artist_identifier() = 0;
    virtual std::string album_identifier() = 0;
    int read_frame_body(std::string & buffer, int size);
  public:
    File * media_file;
    explicit Song(File * media_file);
    virtual ~Song();
    virtual int read_frame(std::string & buffer, const std::string & tag) = 0;
    virtual int read_frames();
    void print();

};

class Id3v1: public Song
{
  private:
    const std::string title_id = "";
    const std::string artist_id = "";
    const std::string album_id = "";
  protected:
    std::string title_identifier() { return this->title_id; };
    std::string artist_identifier() { return this->artist_id; };
    std::string album_identifier() { return this->album_id; };
  public:
    ~Id3v1() {};
    explicit Id3v1(File * media_file): Song(media_file) {};
    int read_frame(std::string & buffer, const std::string & tag);
};

class Id3v2: public Song
{
  protected:
    virtual void get_frame_flags(FILE * fp) = 0;
    virtual int get_unicode_encoding(FILE * fp) = 0;
  public:
    explicit Id3v2(File * media_file): Song(media_file) {};
    ~Id3v2() {};
    virtual int get_frame_size(FILE * fp) = 0;
    int read_frame(std::string & buffer, const std::string & tag);
    int eat_frame_header(const std::string & frame_id);
};

class Id3v2_2: public Id3v2
{
  private:
    const std::string title_id = "TT2";
    const std::string artist_id = "TP1";
    const std::string album_id = "TAL";
  protected:
    std::string title_identifier() { return this->title_id; };
    std::string artist_identifier() { return this->artist_id; };
    std::string album_identifier() { return this->album_id; };
    void get_frame_flags(FILE * fp);
    int get_unicode_encoding(FILE * fp);
  public:
    explicit Id3v2_2(File * media_file): Id3v2(media_file) {};
    ~Id3v2_2() {};
    int get_frame_size(FILE * fp);
};

class Id3v2_3: public Id3v2
{
  private:
    const std::string title_id = "TIT2";
    const std::string artist_id = "TPE1";
    const std::string album_id = "TALB";
  protected:
    std::string title_identifier() { return this->title_id; };
    std::string artist_identifier() { return this->artist_id; };
    std::string album_identifier() { return this->album_id; };
    void get_frame_flags(FILE * fp);
    int get_unicode_encoding(FILE * fp);
  public:
    explicit Id3v2_3(File * media_file): Id3v2(media_file) {};
    ~Id3v2_3() {};
    int get_frame_size(FILE * fp);
};

class Id3v2_4: public Id3v2
{
  private:
    const std::string title_id = "TIT2";
    const std::string artist_id = "TPE1";
    const std::string album_id = "TALB";
  protected:
    std::string title_identifier() { return this->title_id; };
    std::string artist_identifier() { return this->artist_id; };
    std::string album_identifier() { return this->album_id; };
    void get_frame_flags(FILE * fp);
    int get_unicode_encoding(FILE * fp);
  public:
    explicit Id3v2_4(File * media_file): Id3v2(media_file) {};
    ~Id3v2_4() {};
    int get_frame_size(FILE * fp);
};

class Mp4: public Song
{
  private:
    const std::string title_id = "\xa9nam";
    const std::string artist_id = "\xa9""ART";
    const std::string album_id = "\xa9""alb";
    int ilst_size = 0;
  protected:
    std::string title_identifier() { return this->title_id; };
    std::string artist_identifier() { return this->artist_id; };
    std::string album_identifier() { return this->album_id; };
    int seek_ilst();
  public:
    explicit Mp4(File * media_file): Song(media_file) {};
    ~Mp4() {};
    int find_atom(const std::string & atom_name, int parent_size);
    int read_frame(std::string & buffer, const std::string & tag);
    int read_frames();
};
#endif

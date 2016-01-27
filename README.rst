Music Metadata
==============

This program reads the title, artist, and album from MP3 and MP4 files. The
main use case is for files output from iTunes, which may have nondescriptive
file names like `UUVB.mp3`. If you want to copy these files from a non-iTunes
enabled system onto a device but are unsure what the contents of the files are,
this program will help.

Install
-------

run `make`

Usage
-----

Get metadata from one song::

  ./mm /path/to/song

Get metadata from a directory::

  find /mnt/dib/ -exec printf "%s:" '{}' \; -exec ./mm '{}' \;

You can use similar scripts to sort, filter, and copy files.

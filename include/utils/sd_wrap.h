
#ifndef SD_WRAP_H
#define SD_WRAP_H

#ifndef _SD_H_
#define _SD_H_			// avoid original SD.h from being loaded

#include <Arduino.h>
#include "SdFat.h"

/* use the first define if you are sure SD.cpp is not compiled, as the global
 * "SD" structure would conflict! Otherwise use SDFAT instead of SD in your code */
//#define SD_GLOBAL_NAME	SD
#define SD_GLOBAL_NAME		SDFAT

// code fragments from https://github.com/PaulStoffregen/SD.git

// Use FILE_READ & FILE_WRITE as defined by FS.h
#if defined(FILE_READ) && !defined(FS_H)
#undef FILE_READ
#endif
#if defined(FILE_WRITE) && !defined(FS_H)
#undef FILE_WRITE
#endif
#include <FS.h>
#include <FSImpl.h>
#include <sd_defines.h>


#if defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(__IMXRT1062__)
#define BUILTIN_SDCARD 254
#endif

#define SDFAT_FILE FsFile
#define SDFAT_BASE SdFs
#define MAX_FILENAME_LEN 256


#define SD_FAT_TYPE 0


namespace fs
{

  class SdFileImpl;
  typedef std::shared_ptr<SdFileImpl> SdFileImplPtr;
  class SdFatImpl;
  typedef std::shared_ptr<SdFatImpl> SdFatImplPtr;


  class SdFileImpl : public FileImpl {
    public:
      SdFileImpl(char *path, const SDFAT_FILE &file);
      ~SdFileImpl() override;
      size_t write(const uint8_t *buf, size_t size) override;
      int peek();
      int available();
      void flush() override;
      size_t read(uint8_t *buf, size_t nbyte) override;
      bool truncate(uint64_t size=0);
      bool seek(uint32_t pos, SeekMode mode = SeekSet);
      size_t position() const override;
      size_t size() const override;
      void close() override;
      operator bool() override;
      const char * name() const override;
      boolean isDirectory(void) override;
      FileImplPtr openNextFile(const char *mode) override;
      void rewindDirectory(void) override;
      time_t getLastWrite() override;
      bool sync();
      static char* getDirNameOfFullPath(const char *path);
      bool setBufferSize(size_t size) override;
      const char* path() const override;
      boolean seekDir(long position);
      String getNextFileName(void);
      String getNextFileName(bool *isDir);


    private:
      SDFAT_FILE sdfatfile;
      char *_fullpath;
  };

  class SdFatImpl : public FSImpl {

    public:
      SdFatImpl();
      ~SdFatImpl() override;
      FileImplPtr open(const char* path, const char* mode, const bool create) override;
      bool exists(const char* path) override;
      bool rename(const char* pathFrom, const char* pathTo) override;
      bool remove(const char* path) override;
      bool mkdir(const char *path) override;
      bool rmdir(const char *path) override;
      void setSd(SDFAT_BASE *sd);
  };


  class SdFATFs : public FS {
    protected:
      SdFatImplPtr _impl;

    public:
      SdFATFs(SdFatImplPtr impl);
      bool begin();
      bool begin(SdSpiConfig cfg);
      bool begin(uint8_t ssPin, SPIClass &spi, uint32_t frequency, const char * mountpoint, uint8_t max_files);
      void end();
      sdcard_type_t cardType();
      uint64_t cardSize();
      uint64_t totalBytes();
      uint64_t usedBytes();
  };

}

extern fs::SdFATFs SD_GLOBAL_NAME;
#endif
#endif

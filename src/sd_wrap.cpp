

#include "sd_wrap.h"
#include <FSImpl.h>

using namespace fs;


static SDFAT_BASE sd;



SdFileImpl::SdFileImpl(char *path, const SDFAT_FILE &file) : sdfatfile(file) {
  char filename[MAX_FILENAME_LEN];
  size_t namelen = sdfatfile.getName(filename, MAX_FILENAME_LEN);
  size_t plen = strlen(path);
  namelen = strlen(filename);		/*TODO hopefully this gets fixed */

  if (strcmp(path, "/") == 0 && strcmp(filename, "/") == 0) {
    _fullpath = strdup(filename);
  } else {
    // make sure path ends with "/"
    if (path[plen-1] != '/') {
      Serial.println("ERROR: path should end with \"/\"");
    }
    _fullpath = (char *)malloc(plen + namelen + 1);
    sprintf(_fullpath, "%s%s", path, filename);
  }
  Serial.println(_fullpath);
}

SdFileImpl::~SdFileImpl() {
  if (sdfatfile) sdfatfile.close();
  if (_fullpath) free(_fullpath);
}


size_t SdFileImpl::write(const uint8_t *buf, size_t size) {
  return sdfatfile.write(buf, size);
}

int SdFileImpl::peek() {
  return sdfatfile.peek();
}

int SdFileImpl::available() {
  return sdfatfile.available();
}

void SdFileImpl::flush() {
  sdfatfile.flush();
}

size_t SdFileImpl::read(uint8_t *buf, size_t nbyte) {
  return sdfatfile.read(buf, nbyte);
}

bool SdFileImpl::truncate(uint64_t size) {
  return sdfatfile.truncate(size);
}

bool SdFileImpl::seek(uint32_t pos, SeekMode mode) {
  if (mode == SeekSet) return sdfatfile.seekSet(pos);
  if (mode == SeekCur) return sdfatfile.seekCur(pos);
  if (mode == SeekEnd) return sdfatfile.seekEnd(pos);
  return false;
}

size_t SdFileImpl::position() const {
  return sdfatfile.curPosition();
}

size_t SdFileImpl::size() const {
  return const_cast<SDFAT_FILE*>(&sdfatfile)->size();
}

void SdFileImpl::close() {
  if (_fullpath) {
    free(_fullpath);
    _fullpath = nullptr;
  }
  sdfatfile.close();
}

SdFileImpl::operator bool() {
  return sdfatfile.isOpen();
}
const char * SdFileImpl::name() const {
  return const_cast<char *>(_fullpath);
}

boolean SdFileImpl::isDirectory(void) {
  return sdfatfile.isDirectory();
}

FileImplPtr SdFileImpl::openNextFile(const char *mode) {
  oflag_t flag = O_READ;
  if (mode && strncmp(mode, "w", 1) == 0)
    flag = O_WRITE;
  if (mode && strncmp(mode, "a", 1) == 0)
    flag = O_APPEND;
  SDFAT_FILE file = sdfatfile.openNextFile(flag);
  if (file) {
    char *path = getDirNameOfFullPath(_fullpath);
    std::shared_ptr<SdFileImpl> ptr = std::shared_ptr<SdFileImpl>(new SdFileImpl(path, file));
    return ptr;
  }
  return FileImplPtr();
}

void SdFileImpl::rewindDirectory(void) {
  sdfatfile.rewindDirectory();
}

time_t SdFileImpl::getLastWrite() {
  uint16_t date, time;
  time_t rettime = 0;
  bool ret = sdfatfile.getModifyDateTime(&date, &time);
  if (ret)
    rettime = (date << 16) | time;
  return rettime;
}

bool SdFileImpl::sync() {
  return sdfatfile.sync();
}

char* SdFileImpl::getDirNameOfFullPath(const char *path) {
  char *newpath = NULL;
  size_t len = strlen(path);
  if ((strcmp(path, "/") != 0) && (len > 1)) {
    for (len = len-2; len>=0; len--) {  // skip ending "/"
      if (path[len] == '/') {
        break;
      }
    }
    len++;
    newpath = (char *)malloc(len+1);
    memcpy(newpath, path, len);
    newpath[len] = 0;
  } else {
    newpath = strdup(path);
  }
  return newpath;
}





/*
 * SdFatImpl
 */

SdFatImpl::SdFatImpl() {};

SdFatImpl::~SdFatImpl() {}


FileImplPtr SdFatImpl::open(const char* path, const char* mode, const bool create) {
  Serial.print("Opening file: ");
  Serial.println(path);
  if (!path || strlen(path) == 0) {
    return FileImplPtr();
  }
  oflag_t flag = O_READ;
  if (mode && strncmp(mode, "w", 1) == 0)
    flag = O_WRITE | O_CREAT;
  if (mode && strncmp(mode, "a", 1) == 0)
    flag = O_APPEND | O_CREAT;
  SDFAT_FILE file = sd.open(path, flag);
  if (file) {
    char *newpath = SdFileImpl::getDirNameOfFullPath(path);
    return std::make_shared<SdFileImpl>(newpath, file);
  }
  return FileImplPtr();
}

bool SdFatImpl::exists(const char* path) {
  return sd.exists(path);
}

bool SdFatImpl::rename(const char* pathFrom, const char* pathTo) {
  return sd.rename(pathFrom, pathTo);
}

bool SdFatImpl::remove(const char* path) {
  return sd.remove(path);
}

bool SdFatImpl::mkdir(const char *path) {
  return sd.mkdir(path);
}

bool SdFatImpl::rmdir(const char *path) {
  return sd.rmdir(path);
}






SdFATFs::SdFATFs(SdFatImplPtr impl) : FS(impl), _impl(impl) {
}

bool SdFATFs::begin() {
  return sd.begin();
}

bool SdFATFs::begin(SdSpiConfig cfg) {
  bool ret = sd.begin(cfg);
  if (!ret)
    sd.initErrorHalt(&Serial);
  return ret;
}


bool SdFATFs::begin(uint8_t ssPin=SS, SPIClass &spi=SPI, uint32_t frequency=4000000, const char * mountpoint="/sd", uint8_t max_files=5)
{
  uint8_t opt = SHARED_SPI;
  SdSpiConfig cfg = SdSpiConfig(ssPin, opt, frequency, &spi);
  _impl->mountpoint(mountpoint);
  return sd.begin(cfg);
}

void SdFATFs::end()
{
  _impl->mountpoint(NULL);
  sd.end();
}

sdcard_type_t SdFATFs::cardType()
{
  switch(sd.card()->type()) {
    case 0:
    case 1:
      return CARD_SD;
    case 3:
      return CARD_SDHC;
    default:
      return CARD_UNKNOWN;
  }
}

uint64_t SdFATFs::cardSize()
{
  return sd.clusterCount() * sd.bytesPerCluster();
}

uint64_t SdFATFs::totalBytes()
{
  return sd.freeClusterCount() * sd.bytesPerCluster();
}

uint64_t SdFATFs::usedBytes()
{
  return (sd.clusterCount() - sd.freeClusterCount()) * sd.bytesPerCluster();
}

SdFATFs SD_GLOBAL_NAME = SdFATFs(SdFatImplPtr(new SdFatImpl()));

bool SdFileImpl::setBufferSize(size_t size) {
  return false;
}

const char *SdFileImpl::path() const {
  return nullptr;
}

void SdFatImpl::setSd(SdFs *sd) {

}

boolean SdFileImpl::seekDir(long position) {
  return false;
}

String SdFileImpl::getNextFileName(void) {
  return "";
}

String SdFileImpl::getNextFileName(bool *isDir) {
  return "";
}

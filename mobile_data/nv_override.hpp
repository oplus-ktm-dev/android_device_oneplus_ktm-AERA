/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include "qmi.hpp"
#include "oem_backing.h"
#include <openssl/sha.h>
#include <fcntl.h>
#include <sys/stat.h>

namespace aera::mobile {
inline bool KtmCnNvImageValid(const Bytes &image) {
  static constexpr unsigned char expected[SHA256_DIGEST_LENGTH]={
    0xba,0x11,0xdb,0x2b,0x08,0x33,0x98,0x2f,0xfc,0xf3,0x4c,0x25,0xde,0xcc,0x5f,0x84,
    0xff,0xbd,0xb5,0x94,0xaf,0x87,0x11,0x20,0xbd,0x71,0xfe,0x53,0x21,0xfd,0x2d,0xe9};
  if(image.size()!=KTM_CN_NV_SIZE)return false;
  unsigned char digest[SHA256_DIGEST_LENGTH];
  return SHA256(image.data(),image.size(),digest)&&!memcmp(digest,expected,sizeof(expected));
}

inline void ValidateKtmNvOverride(unsigned long project) {
  if(project!=23821)return;
  Fd fd(open(KTM_CN_NV_PATH,O_RDONLY|O_CLOEXEC|O_NOFOLLOW));struct stat st{};
  if(fd.get()<0||fstat(fd.get(),&st)||!S_ISREG(st.st_mode)||st.st_uid!=0||
      (st.st_mode&0022)||st.st_size!=KTM_CN_NV_SIZE)
    throw std::runtime_error("Project 23821 CN modem image is missing or invalid");
  Bytes image(KTM_CN_NV_SIZE);size_t offset=0;
  while(offset<image.size()) {
    const auto n=read(fd.get(),image.data()+offset,image.size()-offset);
    if(n<0&&errno==EINTR)continue;
    if(n<=0)throw std::runtime_error("Cannot read project 23821 CN modem image");
    offset+=n;
  }
  if(!KtmCnNvImageValid(image))
    throw std::runtime_error("Project 23821 CN modem image checksum mismatch");
}
}

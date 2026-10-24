/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include "modem_features.hpp"
#include <array>
#include <cstdlib>

namespace aera::mobile {
// KTM's stock rmt_storage serves a synthetic 512-byte sector at the end of
// the 20 MiB static OEM NV image. These packed fields match its miscinfo ABI;
// they describe this device, not a replacement IMEI or generated identity.
inline Bytes KtmOemMiscInfo(const std::array<uint8_t,4> &serial,unsigned svn,
                             uint64_t project,unsigned rf,unsigned pcb) {
  if(svn>98||rf>255||pcb>255||!project)
    throw std::runtime_error("Invalid OEM modem hardware metadata");
  Bytes page(512,0);
  const uint32_t magic=htole32(0x87654321);memcpy(page.data(),&magic,sizeof(magic));
  std::copy(serial.begin(),serial.end(),page.begin()+0x1a7);
  page[0x1ab]=1;page[0x1ac]=svn;
  project=htole64(project);memcpy(page.data()+0x1af,&project,sizeof(project));
  page[0x1b7]=rf;page[0x1b8]=pcb;
  return page;
}

inline unsigned long KtmHardwareNumber(const char *path) {
  Fd fd(open(path,O_RDONLY|O_CLOEXEC|O_NOFOLLOW));char text[128]{};
  if(fd.get()<0||read(fd.get(),text,sizeof(text)-1)<=0)
    throw std::runtime_error("OEM modem hardware metadata is unavailable");
  char *end=nullptr;errno=0;const auto n=strtoul(text,&end,10);
  if(errno||end==text||text[0]<'0'||text[0]>'9')
    throw std::runtime_error("Invalid OEM modem hardware metadata");
  while(*end==' '||*end=='\t'||*end=='\r'||*end=='\n')++end;
  if(*end)throw std::runtime_error("Invalid OEM modem hardware metadata");
  return n;
}

inline Bytes ReadKtmOemMiscInfo() {
  const auto xml=ReadModemFeatures();tinyxml2::XMLDocument doc;
  if(doc.Parse(xml.c_str())!=tinyxml2::XML_SUCCESS)
    throw std::runtime_error("Invalid modem feature record");
  const auto *svn=doc.RootElement()->FirstChildElement("imeiSv");
  if(!svn||!svn->GetText())throw std::runtime_error("Missing modem software version");
  std::array<uint8_t,4> serial{};
  Fd fd(open("/sys/devices/soc0/serial_number",O_RDONLY|O_CLOEXEC));
  if(fd.get()<0||read(fd.get(),serial.data(),serial.size())!=ssize_t(serial.size()))
    throw std::runtime_error("OEM modem hardware serial is unavailable");
  return KtmOemMiscInfo(serial,strtoul(svn->GetText(),nullptr,10),
      KtmHardwareNumber("/proc/oplusVersion/prjName"),
      KtmHardwareNumber("/proc/oplusVersion/RFType"),
      KtmHardwareNumber("/proc/oplusVersion/pcbVersion"));
}
}

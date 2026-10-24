/* SPDX-License-Identifier: Apache-2.0 */
#include "../oem_miscinfo.hpp"
#include <cassert>
#include <cstdio>
using namespace aera::mobile;
int main() {
  const std::array<uint8_t,4> serial{'1','2','3','4'};
  const auto page=KtmOemMiscInfo(serial,42,24851,7,40);
  assert(page.size()==512&&U32(page.data())==0x87654321);
  assert(std::equal(serial.begin(),serial.end(),page.begin()+0x1a7));
  assert(page[0x1ab]==1&&page[0x1ac]==42&&page[0x1ad]==0);
  assert(U32(page.data()+0x1af)==24851&&U32(page.data()+0x1b3)==0);
  assert(page[0x1b7]==7&&page[0x1b8]==40);
  assert(std::all_of(page.begin()+441,page.end(),[](uint8_t b){return b==0;}));
  for(unsigned bad:{99u,256u}) {
    bool rejected=false;
    try{KtmOemMiscInfo(serial,bad,24851,7,40);}catch(const std::exception &){rejected=true;}
    assert(rejected);
  }
  puts("KTM OEM metadata ABI and bounds passed");
}

/* SPDX-License-Identifier: Apache-2.0 */
#include "../nv_override.hpp"
#include <cassert>
#include <fstream>
#include <iterator>
#include <cstdio>
using namespace aera::mobile;
int main(int argc,char **argv) {
  assert(argc==2);
  for(const char *slot:{"_a","_b"}) {
    assert(!strcmp(ktm_static_nv_backing("23821",slot),KTM_CN_NV_PATH));
    for(const char *project:{"23822","238210","023821","23821extra","","unknown"}) {
      const std::string expected=std::string("/dev/block/bootdevice/by-name/oplusstanvbk")+slot;
      assert(expected==ktm_static_nv_backing(project,slot));
    }
  }
  assert(!ktm_static_nv_backing("23821",nullptr));
  assert(!ktm_static_nv_backing("23821","_c"));
  assert(!ktm_static_nv_backing("23821",""));
  assert(!strcmp(ktm_static_nv_backing(nullptr,"_b"),"/dev/block/bootdevice/by-name/oplusstanvbk_b"));
  std::ifstream in(argv[1],std::ios::binary);
  Bytes image((std::istreambuf_iterator<char>(in)),{});
  assert(KtmCnNvImageValid(image));
  image[0]^=1;assert(!KtmCnNvImageValid(image));
  image[0]^=1;image.pop_back();assert(!KtmCnNvImageValid(image));
  ValidateKtmNvOverride(23822); // No file required for other projects.
  puts("KTM exact-project selection, slot fallback, and CN image integrity passed");
}

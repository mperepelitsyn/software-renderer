#include <fstream>

#include "app/error.h"
#include "app/tga_loader.h"

namespace {

#pragma pack(push, 1)
struct TGAHeader {
   char  id_len;
   char  colormap_type;
   char  datatype_code;
   short int colormap_orig;
   short int colormap_len;
   char  colormap_depth;
   short int x_origin;
   short int y_origin;
   short width;
   short height;
   char  bpp;
   char  img_desc;
};
#pragma pack(pop)

struct Pixel24 {
  unsigned char b;
  unsigned char g;
  unsigned char r;
};

} // namespace

namespace app {

std::vector<renderer::UNorm> loadTGA(const std::string &path) {
  std::ifstream ifs(path, std::ios::binary);
  if (!ifs.good())
    throw Error{"failed to open TGA '" + path + '\''};

  TGAHeader header;
  ifs.read(reinterpret_cast<char*>(&header), sizeof header);
  unsigned size = header.width * header.height;
  std::vector<renderer::UNorm> out(size);

  for (auto i = 0u; i < size; ++i) {
    Pixel24 pix;
    ifs.read(reinterpret_cast<char*>(&pix), sizeof pix);
    out[i] = {pix.r, pix.g, pix.b, 255};
  }

  return out;
}

} // namespace app

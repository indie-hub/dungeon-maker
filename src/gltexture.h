#pragma once

#include <string>

class GLTexture
{
private:
  std::string   Name;
  unsigned int  Id;

public:
  using Handle_t = std::unique_ptr<uint8_t[], void(*)(void*)>;

  GLTexture() = default;
  GLTexture(const std::string& TextureName)
    : Name(TextureName)
    , Id(0)
  {}

  ~GLTexture() noexcept;
  unsigned int Texture() const { return Id; }

  Handle_t Load(const std::string& FileName);
};

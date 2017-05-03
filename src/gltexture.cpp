#include "gltexture.h"

#include <nanogui/opengl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

GLTexture::~GLTexture() noexcept
{
  if(Id)
  {
    glDeleteTextures(1, &Id);
  }
}


GLTexture::Handle_t GLTexture::Load(const std::string& FileName)
{
  if(Id)
  {
    glDeleteTextures(1, &Id);
    Id = 0;
  }

  int ForceChannels = 0;
  int Width, Height, NChannels;

  Handle_t TextureData(stbi_load(FileName.c_str(), &Width, &Height, &NChannels, ForceChannels), stbi_image_free);
  if(nullptr == TextureData)
  {
    throw std::invalid_argument("Could not load texture data from " + FileName);
  }
  glGenTextures(1, &Id);
  glBindTexture(GL_TEXTURE_2D, Id);
  GLint InternalFormat;
  GLint Format;
  switch(NChannels)
  {
    case 1: InternalFormat = GL_R8; Format = GL_RED; break;
    case 2: InternalFormat = GL_RG8; Format = GL_RG; break;
    case 3: InternalFormat = GL_RGB8; Format = GL_RGB; break;
    case 4: InternalFormat = GL_RGBA8; Format = GL_RGBA; break;
    default: InternalFormat = 0; Format = 0; break;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, Format,
                GL_UNSIGNED_BYTE, TextureData.get());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  return TextureData;
}

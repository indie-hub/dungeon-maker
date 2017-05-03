#pragma once

#include <nanogui/glutil.h>
#include <json.hpp>
#include "map.h"
#include "gltexture.h"

using JSON = nlohmann::json;

class MapRepresentation
{
private:
  std::string                          ConfigFile;
  Map::Map_ptr                         TheMap;
  JSON                                 Configuration;

  GLTexture                            TileTexture;
  GLTexture::Handle_t                  TextureHandle;
  nanogui::GLShader                    GridRenderingShader;
  std::vector<nanogui::GLShader>       TilesRenderingLayers;

  bool                    ResourcesLoaded; //safekeep to only load the resources with an active context
  Eigen::Vector2i         ViewportSize;
  bool                    DisplayGrid;

public:
  using MapRepresentation_ptr = std::unique_ptr<MapRepresentation>;

  MapRepresentation(const std::string &ConfigurationFileName, Map::Map_ptr &Map)
    : ConfigFile(ConfigurationFileName)
    , TheMap(Map)
    , Configuration()
    , TileTexture()
    , TextureHandle(nullptr, nullptr)
    , TilesRenderingLayers()
    , ResourcesLoaded(false)
    , ViewportSize()
    , DisplayGrid(false)
  {}

  ~MapRepresentation() = default;
  bool Init(const Eigen::Vector2i& Viewport);
  void SetViewportSize(const Eigen::Vector2i& Viewport) { ViewportSize = Viewport; }

  void DrawUsingCamera(const Eigen::Matrix4f &CameraMatrix);

  void SetDisplayGrid(bool Value) { DisplayGrid = Value; }

private:
  MapRepresentation() = delete;
  MapRepresentation(const MapRepresentation&) = delete;

  void LoadResources();
  void ComputeMapGeometry();
};

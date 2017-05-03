#include "maprepresentation.h"

#include <nanogui/opengl.h>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <random>
#include <functional>



/**
I'll put this here, for the time being...
Later, I'll create  JSON utils
*/

std::string ConcatShaderArrayIntoString(const std::string& Key, const JSON& From, const std::string& Shader)
{
  std::stringstream str;
  for(const auto& Line : From["shaders"][Shader][Key])
  {
    str << Line.get<std::string>() << "\n";
  }
  return str.str();
}

bool MapRepresentation::Init(const Eigen::Vector2i& Viewport)
{
    try
    {
      std::ifstream File(ConfigFile);
      File >> Configuration;
      File.close();
    }
    catch(const std::exception &e)
    {
      std::string Message("Exception caught " + std::string(e.what()));
      std::cerr << Message << std::endl;
      return false;
    }
    std::cout << std::setw(4) << Configuration << std::endl;

    TilesRenderingLayers.resize(Configuration["map"]["layers"].size());
    SetViewportSize(Viewport);
    return true;
}

void MapRepresentation::DrawUsingCamera(const Eigen::Matrix4f &CameraMatrix)
{
  if(!ResourcesLoaded)
  {
    LoadResources();
    ResourcesLoaded = true;
  }

  if(TheMap->IsDirty())
  {
    ComputeMapGeometry();
    TheMap->SetDirty(false);
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, TileTexture.Texture());

  auto& FloorLayer = TilesRenderingLayers[0];
  FloorLayer.bind();
  FloorLayer.setUniform("modelViewProj", CameraMatrix);
  FloorLayer.setUniform("tiles", 0);
  FloorLayer.drawIndexed(GL_TRIANGLES, 0, TheMap->GetWidth() * TheMap->GetHeight() * 2);


  //TODO: Add a layer type: Grid
  #if 1
    if(DisplayGrid)
    {
      //Draw grid
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      GridRenderingShader.bind();
      GridRenderingShader.setUniform("modelViewProj", CameraMatrix);

      GridRenderingShader.drawIndexed(GL_TRIANGLES, 0, TheMap->GetWidth() * TheMap->GetHeight() * 2);
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
  #endif


  for(int layer = 1; layer < TilesRenderingLayers.size(); ++layer)
  {
    auto& Layer = TilesRenderingLayers[layer];
    Layer.bind();
    Layer.setUniform("modelViewProj", CameraMatrix);
    Layer.setUniform("tiles", 0);
    Layer.drawIndexed(GL_TRIANGLES, 0, TheMap->GetWidth() * TheMap->GetHeight() * 2);
  }

  glDisable(GL_BLEND);
  //Draw tiles

}

void MapRepresentation::LoadResources()
{
  TileTexture = GLTexture(Configuration["tileset"]["file"].get<std::string>());
  TextureHandle = TileTexture.Load(Configuration["tileset"]["file"].get<std::string>());

  std::string VertexShader = ConcatShaderArrayIntoString("vertex_shader", Configuration, "grid");
  std::string FragmentShader = ConcatShaderArrayIntoString("fragment_shader", Configuration, "grid");

  GridRenderingShader.init(Configuration["shaders"]["grid"]["id"].get<std::string>(),
              VertexShader,
              FragmentShader);

  int LayerCount = 0;
  for(const auto& LayerConf: Configuration["map"]["layers"])
  {
    const std::string ShaderName = LayerConf["shader"].get<std::string>();
    auto& Layer = TilesRenderingLayers[LayerCount++];

    VertexShader = ConcatShaderArrayIntoString("vertex_shader", Configuration, ShaderName);
    FragmentShader = ConcatShaderArrayIntoString("fragment_shader", Configuration, ShaderName);

    Layer.init(Configuration["shaders"][ShaderName]["id"].get<std::string>(),
                VertexShader,
                FragmentShader);
  }
}

/**
  !!!IMPORTANT!!! Assumes active opengl context
  !!!IMPORTANT!!! This method is becoming *HUGE*
*/
void MapRepresentation::ComputeMapGeometry()
{
  const int MapWidth = TheMap->GetWidth();
  const int MapHeight = TheMap->GetHeight();

  /**
  BACK-LOG:
  This looked like a good idea, but it is not. It introduces more unnecessary computations.
  Work, instead, with zoom-in, zoom-out

  const float TilesWidthMap = Configuration["map"]["tilesize"]["width"].get<float>();
  const float TilesHeightMap = Configuration["map"]["tilesize"]["height"].get<float>();


  const float TilesWidthViewport = ViewportSize.x() / (float)MapWidth;
  const float TilesHeightViewport = ViewportSize.y() / (float)MapHeight;

  const float TilesWidth = std::min(TilesWidthViewport, TilesWidthMap);
  const float TilesHeight = TilesWidth * ((float)TilesHeightMap / (float)TilesWidthMap);
  */

  const float TilesWidth = Configuration["map"]["tilesize"]["width"].get<float>();
  const float TilesHeight = Configuration["map"]["tilesize"]["height"].get<float>();

  const float TilesWidthHalfSize = TilesWidth * 0.5f;
  const float TilesHeightHalfSize = TilesHeight * 0.5f;

  std::cout << "Tile size: " << TilesWidth << " x " << TilesHeight << std::endl;

  const float MapHeightInPixels = MapHeight * TilesHeight;
  const float HalfMapHeightInPixels = MapHeightInPixels * 0.5f;

  const Map& MapRef(*TheMap);

  /**
    Create grid.
    TODO: this isn't actually necessary. A geometry/tessellation shader solves
    this, but, for now, this is easier to implement
  */

  int VertexCounter(0);
  nanogui::MatrixXf Positions(3, MapWidth  * MapHeight * 4);
  nanogui::MatrixXf Colors(3, MapWidth * MapHeight * 4);

  /**
    Create 2 triangles for each tile, sharing 2 vertices
    It is unefficient, but the tex coords are not continuous
  */
  for(int y = 0, x = 0; y < MapHeight; ++y)
  {
    for(x = 0; x < MapWidth; ++x)
    {
      //Color will be the same across the triangles' vertices
      Eigen::Vector3f Color;

      Color << 0.0f, 1.0f, 0.0f;
      if(MapRef(x, y) != Map::ETiles::Floor)
      {
        Color << 1.0f, 0.0f, 0.0f;
      }

      //Vertices created counter-clockwise
      Positions.col(VertexCounter) << (x - y) * TilesWidthHalfSize, (x + y) * TilesHeightHalfSize - HalfMapHeightInPixels, 0.f;
      Colors.col(VertexCounter++) << Color;

      Positions.col(VertexCounter) << (x - (y + 1)) * TilesWidthHalfSize, (x + (y + 1)) * TilesHeightHalfSize - HalfMapHeightInPixels, 0.f;
      Colors.col(VertexCounter++) << Color;

      Positions.col(VertexCounter) << ((x + 1) - (y + 1)) * TilesWidthHalfSize, ((x + 1) + (y + 1)) * TilesHeightHalfSize - HalfMapHeightInPixels, 0.f;
      Colors.col(VertexCounter++) << Color;

      Positions.col(VertexCounter) << ((x + 1) - y) * TilesWidthHalfSize, ((x + 1) + y) * TilesHeightHalfSize - HalfMapHeightInPixels, 0.f;
      Colors.col(VertexCounter++) << Color;
    }
  }

  //6 indices per tile
  VertexCounter = 0;
  nanogui::MatrixXu Indices(3, MapWidth * MapHeight * 2);
  int Offset(0);
  for(int y = 0, x = 0; y < MapHeight; ++y)
  {
    for(x = 0, Offset = 0; x < MapWidth; ++x, Offset += 3)
    {
      //1st triangle
      Indices.col(VertexCounter++) << x + y * (MapWidth * 4) + Offset, (x + 1) + y * (MapWidth * 4) + Offset, (x + 3) + y * (MapWidth * 4) + Offset;

      //2nd triangle
      Indices.col(VertexCounter++) << (x + 3) + y * (MapWidth * 4) + Offset, (x + 1) + y * (MapWidth * 4) + Offset, (x + 2) + y * (MapWidth * 4) + Offset;
    }
  }

  GridRenderingShader.bind();
  GridRenderingShader.uploadIndices(Indices);
  GridRenderingShader.uploadAttrib("position", Positions);
  GridRenderingShader.uploadAttrib("color", Colors);



  /**
    Create tiled layers.
    Manipulate the previously computed grid positions and use the same indices
  */

  using NodeList_t = std::vector<int>;
  using NodeListMap_t = std::map<int, NodeList_t>;
  using NodeListMapPerLayer_t = std::map<std::string, NodeListMap_t>;

  NodeListMapPerLayer_t NodeListMapPerLayer;

  auto GetTileWithDirection = [&](int x, int y, const std::string& LayerType)
                              {
                                if(!NodeListMapPerLayer.count(LayerType))
                                {
                                  NodeListMapPerLayer.insert(std::make_pair(LayerType, NodeListMap_t()));
                                  auto& NodeListMap = NodeListMapPerLayer[LayerType];

                                  const int AvailableTiles = Configuration["tileset"]["tiles"]["tiles"][LayerType].size();
                                  const auto& Tiles = Configuration["tileset"]["tiles"]["tiles"][LayerType];
                                  for(int t = 0; t < AvailableTiles; ++t)
                                  {
                                    const int Orientation = Tiles[t]["orientation"];
                                    if(!NodeListMap.count(Orientation))
                                    {
                                      NodeListMap.insert(std::make_pair(Orientation, NodeList_t()));
                                    }
                                    NodeListMap[Orientation].push_back(t);
                                  }

                                }

                                const int Code = MapRef.GetNeighboursCode(x, y);
                                if(!NodeListMapPerLayer[LayerType].count(Code))
                                {
                                  if(Code != 255) std::cout << Code << ", ";
                                  return -1;
                                }

                                return NodeListMapPerLayer[LayerType][Code][0];
                              };



  const Eigen::Vector3f WhiteColor(1.0f, 1.0f, 1.0f);
  const Eigen::Vector3f BlackColor(0.0f, 0.0f, 0.0f);

  int LayerCount(0);
  for(const auto& LayerConf: Configuration["map"]["layers"])
  {
    auto& Layer = TilesRenderingLayers[LayerCount++];

    const std::string LayerType = LayerConf["type"].get<std::string>();
    const bool CoversAll = LayerConf["covers_all"].get<bool>();
    const bool HasDirection = LayerConf["has_direction"].get<bool>();

    const float TextureWidth = Configuration["tileset"]["size"]["width"].get<float>();
    const float TextureHeight = Configuration["tileset"]["size"]["height"].get<float>();
    const float TileSetTileHeight = Configuration["tileset"]["tilesize"]["height"].get<float>();

    {
      //TODO: Remove repeated code...
      const int AvailableTiles = Configuration["tileset"]["tiles"]["tiles"][LayerType].size();

      std::mt19937::result_type Seed =
          std::chrono::high_resolution_clock::now().time_since_epoch().count();
      auto RandomNumber = std::bind(std::uniform_int_distribution<int>(0, AvailableTiles - 1),
                                  std::mt19937(Seed));
      nanogui::MatrixXf TexCoords(2, MapWidth * MapHeight * 4);
      VertexCounter = 0;

      auto SetArrays = [&](const float& UVu, const float& UVv, const float& UVu1, const float& UVv1, const Eigen::Vector3f& Color)
      {
        Positions.col(VertexCounter).x() = Positions.col(VertexCounter + 1).x();
        TexCoords.col(VertexCounter) << UVu, UVv;
        Colors.col(VertexCounter) << Color;
        ++VertexCounter;

        Positions.col(VertexCounter).y() = Positions.col(VertexCounter + 1).y();
        TexCoords.col(VertexCounter) << UVu, UVv1;
        Colors.col(VertexCounter) << Color;
        ++VertexCounter;

        Positions.col(VertexCounter).x() = Positions.col(VertexCounter + 1).x();
        TexCoords.col(VertexCounter) << UVu1, UVv1;
        Colors.col(VertexCounter) << Color;
        ++VertexCounter;

        Positions.col(VertexCounter).y() = Positions.col(VertexCounter - 3).y();
        TexCoords.col(VertexCounter) << UVu1, UVv;
        Colors.col(VertexCounter) << Color;
        ++VertexCounter;
      };

      for(int y = 0, x = 0; y < MapHeight; ++y)
      {
        for(x = 0; x < MapWidth; ++x)
        {

          bool ShouldContinue(true);
          if(!CoversAll)
          {
            //Check if LayerType is the correct type
            if(LayerType != Map::TileTypeAsString(MapRef(x, y)))
            {
              //std::cout << "cnt, ";
              SetArrays(0.f, 0.f, 0.f, 0.f, WhiteColor);
              continue;
            }
          }
          /**
            Although the layer should cover the entire map, we don't want to cover
            back faced tiles
          */
          else
          {
            for(const auto& LayerConfOther: Configuration["map"]["layers"])
            {
              if(&LayerConf == &LayerConfOther)
              {
                continue;
              }
              const std::string LayerTypeOther = LayerConfOther["type"].get<std::string>();
              if(LayerType == Map::TileTypeAsString(MapRef(x, y)))
              {
                continue;
              }
              const int TileNumber = GetTileWithDirection(x, y, LayerTypeOther);
              if(TileNumber < 0)
              {
                ShouldContinue = false;
                break;
              }
              const auto& Tile = Configuration["tileset"]["tiles"]["tiles"][LayerTypeOther][TileNumber];
              if(Tile["back-faced"].get<bool>())
              {
                ShouldContinue = false;
                break;
              }
            }
          }

          if(!ShouldContinue)
          {
            SetArrays(0.f, 0.f, 0.f, 0.f, WhiteColor);
            continue;
          }

          const int TileNumber = HasDirection ? GetTileWithDirection(x, y, LayerType) : RandomNumber();
          if (TileNumber < 0)
          {
            SetArrays(0.f, 0.f, 0.f, 0.f, WhiteColor /*BlackColor*/);
            continue;
          }

          const auto& Tile = Configuration["tileset"]["tiles"]["tiles"][LayerType][TileNumber]; //Not my brightest moment, I must confess

          const float CurrentTileWidth = Tile["size"]["width"].get<float>();
          const float CurrentTileHeight = Tile["size"]["height"].get<float>();

          const float CurrentTileLowerLeftX = Tile["coord"]["x"].get<float>();
          const float CurrentTileLowerLeftY = Tile["coord"]["y"].get<float>();

          const float UVu = CurrentTileLowerLeftX / TextureWidth;
          const float UVv = (TextureHeight - (CurrentTileLowerLeftY + CurrentTileHeight)) / TextureHeight;
          const float UVu1 = (CurrentTileLowerLeftX + CurrentTileWidth) / TextureWidth;
          const float UVv1 =  (TextureHeight - CurrentTileLowerLeftY) / TextureHeight; //size of tex - lower left corner + height of tile

          SetArrays(UVu, UVv, UVu1, UVv1, WhiteColor);

          float TileHeight = (Positions.col(VertexCounter - 3) - Positions.col(VertexCounter - 4)).norm();
          if (TileHeight != CurrentTileHeight)
          {
            float Diff = CurrentTileHeight - TileHeight;
            Positions.col(VertexCounter - 4).y() = Positions.col(VertexCounter - 4).y() - Diff;
            Positions.col(VertexCounter - 1).y() = Positions.col(VertexCounter - 1).y() - Diff;
          }
        }

        if(LayerType != "floor")
        {
          std::cout << std::endl;
        }
      }

      #if 0 //Just for testing
        Positions.col(0) << -300, -300, 0;
        Positions.col(1) << -300, 300, 0;
        Positions.col(2) << 300, 300, 0;
        Positions.col(3) << 300, -300, 0;
      #endif

      Layer.bind();
      Layer.uploadIndices(Indices);
      Layer.uploadAttrib("position", Positions);
      Layer.uploadAttrib("texCoords", TexCoords);
      Layer.uploadAttrib("color", Colors);

        // std::cout << Positions << std::endl;
        // std::cout << TexCoords << std::endl;
    }
  }

  std::cout << "done" << std::endl;
}

#include "map.h"

#include <chrono>
#include <random>
#include <functional>


//TODO: swap loops to reduce cache misses

Map::Map(int MapWidth, int MapHeight, float MapWallPercentage)
  : Width(MapWidth)
  , Height(MapHeight)
  , WallPercentage(MapWallPercentage)
  , Tiles(nullptr)
{}


Map::~Map() {}

bool Map::GenerateMap()
{
  if(nullptr == Tiles)
  {
    Tiles = Map_t(new ETiles[Width * Height]);
    //check for malloc error
    if (nullptr == Tiles)
    {
      return false;
    }
  }

  std::mt19937::result_type Seed =
      std::chrono::high_resolution_clock::now().time_since_epoch().count();

  auto RandomNumber = std::bind(std::uniform_real_distribution<float>(0,1),
                              std::mt19937(Seed));

  auto MaybeWall = [&](float WallProbability)
                    {
                      if(WallProbability >= RandomNumber())
                          return ETiles::Wall;
                      return ETiles::Floor;
                    };

  Map& TheMap = (*this);

  int MapMiddle = Height / 2;
  for(int y = 0, x = 0; y < Height; ++y)
  {
    for(x = 0; x < Width; ++x)
    {
      auto& CurrentMapTile = TheMap(x, y);

      //Fill border with Wall
      if (0 == x)
      {
        CurrentMapTile = ETiles::Wall;
      }
      else if (0 == y)
      {
        CurrentMapTile = ETiles::Wall;
      }
      else if(Width - 1 == x)
      {
        CurrentMapTile = ETiles::Wall;
      }
      else if(Height - 1 == y)
      {
        CurrentMapTile = ETiles::Wall;
      }

      else
      {
        if(MapMiddle == y)
        {
          CurrentMapTile = ETiles::Floor;
        }
        else
        {
          CurrentMapTile = MaybeWall(WallPercentage);
        }
      }
    }
  }
  return true;
}

void Map::MakeCaverns()
{
  Map& TheMap = (*this);
  for(int y = 0, x = 0; y < Height; ++y)
  {
    for(x = 0; x < Width; ++x)
    {
      TheMap(x, y) = GetTileFor(x, y);
    }
  }
}

std::ostream& operator<< (std::ostream& OutputStream, const Map& MapToPrint)
{
  for(int x = 0, y = 0; y < MapToPrint.GetHeight(); ++y)
  {
    for(x = 0; x < MapToPrint.GetWidth(); ++x)
    {
      OutputStream << MapToPrint(x, y);
    }
    OutputStream << std::endl;
  }
  return OutputStream;
}

Map::ETiles Map::GetTileFor(int x, int y) const
{
  int NumAdjWalls = GetAdjacentWallCount(x, y, Window(1, 1));

  if(IsWall(x, y))
  {
    if(NumAdjWalls >= 4)
    {
      return Map::ETiles::Wall;
    }
    return Map::ETiles::Floor;
  }
  else
  {
    if(NumAdjWalls >= 5)
    {
      return Map::ETiles::Wall;
    }
  }

  return Map::ETiles::Floor;
}

int Map::GetAdjacentWallCount(int x, int y, const Window& SearchWindow) const
{
  const int StartX = x - SearchWindow.first;
  const int StartY = y - SearchWindow.second;

  const int EndX = x + SearchWindow.first;
  const int EndY = y + SearchWindow.second;

  int iX = StartX;
  int iY = StartY;

  int WallCounter = 0;

  for(; iX <= EndX; ++iX)
  {
    for(iY = StartY; iY <= EndY; ++iY)
    {
      if(!(x == iX && y == iY))
      {
        if(IsWall(iX, iY))
        {
          ++WallCounter;
        }
      }
    }
  }

  return WallCounter;
}




bool Map::IsWall(int x, int y) const
{
  if(OutOfBounds(x, y))
  {
    return true;
  }
  if(ETiles::Wall == (*this)(x,y))
  {
    return true;
  }
  return false;
}

bool Map::OutOfBounds(int x, int y) const
{
  if (x < 0 || x >= Width || y < 0 || y >= Height)
  {
    return true;
  }

  return false;
}

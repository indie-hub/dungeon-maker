#include "map.h"

#include <chrono>
#include <random>
#include <functional>


/**
  TODO:
    * merge RemoveStray and RemoveStray (plus corresponding methods)
*/

std::string Map::TileTypeAsStringCache[] = {
 #define X(a) #a,
 ENUM_
 #undef X
};

bool Map::TileNamesLowered(false);
std::set<int> Map::StrayCodes = {209, 219, 217, 139, 126, 155, 110, 118, 211, 203, 62, 124, 216, 153, 27};


std::string Map::TileTypeAsString(const ETiles& Tile)
{
  if(!TileNamesLowered)
  {
    for(int i = 0; i < ETiles::nTiles; ++i)
    {
      STRTOLOWER(TileTypeAsStringCache[i]);
    }
    TileNamesLowered = true;
  }
  return TileTypeAsStringCache[Tile];
}

Map::Map(int MapWidth, int MapHeight, float MapWallPercentage)
  : Width(MapWidth)
  , Height(MapHeight)
  , WallPercentage(MapWallPercentage)
  , Tiles(nullptr)
  , Dirty(false)
{}

Map::~Map() {}

bool Map::Generate(int Steps)
{

  /**
    This is a brute force (and ugly) approach to generate a sound map;
    In the future, a more elegant solution *must* be created!
  */

  bool MapIsSound = false;
  while(!MapIsSound)
  {
    bool ReturnValue = GenerateMap();
    if(ReturnValue)
    {
      for(int i = 0; i < Steps; ++i)
      {
        MakeCaverns();
      }
      RemoveStray();

      int Attempt = 0; //To prevent infinite loops
      while(false == (MapIsSound = MapSound()) && Attempt < 4)
      {
        MakeCaverns();
        RemoveStray();
        ++Attempt;
      }
    }
    PlaceWater();
    RemoveStraySame(ETiles::Water);
    PlaceProps();
    Dirty = true;
  }
  return MapIsSound;
}

int Map::GetNeighboursCode(int x, int y) const
{
  const int StartX = x - 1;
  const int StartY = y - 1;

  const int EndX = x + 1;
  const int EndY = y + 1;

  int iX = StartX;
  int iY = StartY;

  const Map& TheMap(*this);
  const ETiles TileType = TheMap(x, y);

  int Code = 0;
  for(; iX <= EndX; ++iX)
  {
    for(iY = StartY; iY <= EndY; ++iY)
    {
      if(!(x == iX && y == iY))
      {
        if(IsTheSame(iX, iY, TileType))
        {
          Code += 1;
        }
        if(!(iX == EndX && iY == EndY))
        {
          Code <<= 1;
        }
      }
    }
  }

  return Code;
}


bool Map::GenerateMap()
{
  if(!Valid())
  {
    Tiles = Tiles_t(new ETiles[Width * Height]);
    //check for malloc error
    if (!Valid())
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
      if (0 == x || 1 == x)
      {
        CurrentMapTile = ETiles::Wall;
      }
      else if (0 == y || 1 == y)
      {
        CurrentMapTile = ETiles::Wall;
      }
      else if(Width - 1 == x || Width - 2 == x)
      {
        CurrentMapTile = ETiles::Wall;
      }
      else if(Height - 1 == y || Height - 2 == y)
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

void Map::RemoveStray()
{
  Map& TheMap = (*this);
  for(int y = 0, x = 0; y < Height; ++y)
  {
    for(x = 0; x < Width; ++x)
    {
      TheMap(x,y) = CheckStray(x, y);
    }
  }
}

void Map::RemoveStraySame(const ETiles& Type)
{
  Map& TheMap = (*this);
  for(int y = 0, x = 0; y < Height; ++y)
  {
    for(x = 0; x < Width; ++x)
    {
      TheMap(x,y) = CheckStraySame(x, y, Type);
    }
  }
}


void Map::PlaceWater()
{
  using TileCoord_t = std::pair<int, int>;
  using TileList_t = std::vector<TileCoord_t>;

  Map& TheMap = (*this);
  std::mt19937::result_type Seed =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();

  auto RandomX = std::bind(std::uniform_int_distribution<int>(0, Width - 1),
                              std::mt19937(Seed));
  auto RandomY = std::bind(std::uniform_int_distribution<int>(0, Height - 1),
                              std::mt19937(RandomX()));


  const int nTilesMax = Width * Height * 0.05f;

  int Attempt(0);
  int TileCount(0);
  TileList_t TilesToAdd;
  while(TilesToAdd.empty() && Attempt < 10) //Try to find a spot for water
  {
    const int X = RandomX();
    const int Y = RandomY();
    if(ETiles::Floor == TheMap(X, Y))
    {
      TheMap(X, Y) = ETiles::Water;
      TilesToAdd.emplace_back(X, Y);
      ++TileCount;
    }
  }

  TileList_t TileList;
  const Window SearchWindow(1, 1);
  while(!TilesToAdd.empty() && TileCount < nTilesMax)
  {
    TileList.insert(TileList.end(), TilesToAdd.begin(), TilesToAdd.end());
    TilesToAdd.clear();
    while(!TileList.empty() && TileCount < nTilesMax)
    {
      TileCoord_t TileCoord = TileList.back();
      TileList.pop_back();

      const int x = TileCoord.first;
      const int y = TileCoord.second;

      const int StartX = x - SearchWindow.first;
      const int StartY = y - SearchWindow.second;

      const int EndX = x + SearchWindow.first;
      const int EndY = y + SearchWindow.second;

      int iX = StartX;
      int iY = StartY;

      for(; iX <= EndX; ++iX)
      {
        for(iY = StartY; iY <= EndY; ++iY)
        {
          if(!(x == iX && y == iY))
          {
            if(IsFloor(iX, iY))
            {
              TheMap(iX, iY) = ETiles::Water;
              TilesToAdd.emplace_back(iX, iY);
              ++TileCount;
            }
          }
        }
      }
    }
  }
}

void Map::PlaceProps()
{
  Map& TheMap = (*this);
  const int nProps = Width * Height * 0.10f;

  std::mt19937::result_type Seed =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();

  auto RandomX = std::bind(std::uniform_int_distribution<int>(0, Width - 1),
                              std::mt19937(Seed));
  auto RandomY = std::bind(std::uniform_int_distribution<int>(0, Height - 1),
                              std::mt19937(RandomX()));

  for(int i = 0; i < nProps; ++i)
  {
    const int X = RandomX();
    const int Y = RandomY();
    if(ETiles::Floor == TheMap(X, Y))
    {
      TheMap(X, Y) = ETiles::Prop;
    }
  }
}

std::ostream& operator<< (std::ostream& OutputStream, const Map& MapToPrint)
{
  if(!MapToPrint.Valid())
  {
    OutputStream << "No map...";
    return OutputStream;
  }

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

Map::ETiles Map::CheckStray(int x, int y) const
{


  if(IsWall(x, y))
  {
    int NumAdjWalls = GetAdjacentWallCount(x, y, Window(1, 1));
    if(NumAdjWalls <= 3)
    {
      if (NumAdjWalls < 3)
      {
        return Map::ETiles::Floor;
      }
      else
      {
        const int Code = GetNeighboursCode(x, y);
        if(224 == Code || 41 == Code || 7 == Code || 148 == Code)
        {
          return Map::ETiles::Floor;
        }
      }
    }
  }

  return (*this)(x, y);
}

Map::ETiles Map::CheckStraySame(int x, int y, const ETiles& Type) const
{

  if(Type == (*this)(x, y))
  {
    int NumAdjSame = GetAdjacentSameCount(x, y, Window(1, 1));
    if(NumAdjSame <= 3)
    {
      if (NumAdjSame < 3)
      {
        return Map::ETiles::Floor;
      }
      else
      {
        const int Code = GetNeighboursCode(x, y);
        if(224 == Code || 41 == Code || 7 == Code || 148 == Code)
        {
          return Map::ETiles::Floor;
        }
      }
    }
  }

  return (*this)(x, y);
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

int Map::GetAdjacentSameCount(int x, int y, const Window& SearchWindow) const
{
  const int StartX = x - SearchWindow.first;
  const int StartY = y - SearchWindow.second;

  const int EndX = x + SearchWindow.first;
  const int EndY = y + SearchWindow.second;

  int iX = StartX;
  int iY = StartY;

  int SameCounter = 0;
  const ETiles TileType = (*this)(x, y);

  for(; iX <= EndX; ++iX)
  {
    for(iY = StartY; iY <= EndY; ++iY)
    {
      if(!(x == iX && y == iY))
      {
        if(IsTheSame(iX, iY, TileType))
        {
          ++SameCounter;
        }
      }
    }
  }

  return SameCounter;
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

bool Map::IsFloor(int x, int y) const
{
  if(OutOfBounds(x, y))
  {
    return false;
  }
  if(ETiles::Floor == (*this)(x,y))
  {
    return true;
  }
  return false;
}

bool Map::IsTheSame(int x, int y, const ETiles& TileTypeOther) const
{
  if(OutOfBounds(x, y))
  {
    if(ETiles::Wall == TileTypeOther)
    {
      return true;
    }
    return false;
  }
  if(TileTypeOther == (*this)(x, y))
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

/**
  This is a lazy and brute force method to check if the map is sound,
  i.e. doesn't have any tile at an odd position, making impossible
  displaying a continuous wall
*/

bool Map::MapSound() const
{
  for(int y = 0, x = 0; y < Height; ++y)
  {
    for(x = 0; x < Width; ++x)
    {
      const int Code = GetNeighboursCode(x, y);
      if(Map::StrayCodes.find(Code) != Map::StrayCodes.end())
      {
        return false;
      }
    }
  }
  return true;
}

#pragma once

#include <iostream>
#include <memory>
#include <cassert>
#include <set>

#include "string_utils.h"

#define ENUM_ \
X(Floor) \
X(Wall) \
X(Water) \
X(Prop) \
X(nTiles) \

typedef std::pair<int,int> Window;

class Map
{
public:

  enum ETiles {
    #define X(a) a,
    ENUM_
    #undef X
  };

  using Tiles_t = std::unique_ptr<ETiles[]>;
  using Map_ptr = std::shared_ptr<Map>;

  static bool TileNamesLowered;
  static std::string TileTypeAsString(const ETiles& Tile);

private:
  int     Width; //Map's width (measured in tiles)
  int     Height; //Map's height (measured in tiles)
  float   WallPercentage; //The percentage of the map that's filled with walls
  Tiles_t   Tiles; //The map
  bool    Dirty;

  static std::set<int> StrayCodes;

public:
  Map(int MapWidth, int MapHeight, float MapWallPercentage);
  ~Map();

  bool Generate(int Steps = 1);

  inline bool Valid() const {return !(nullptr == Tiles); }

  inline bool IsDirty() const {return Dirty;}
  inline void SetDirty(bool Value) {Dirty = Value;}

  inline int GetWidth() const { return Width; }
  inline int GetHeight() const { return Height; } const
  inline float GetWallPercentage() const { return WallPercentage; }

  inline void SetWidth(int MapWidth) { Tiles = nullptr; Width = MapWidth; }
  inline void SetHeight(int MapHeight) { Tiles = nullptr; Height = MapHeight; }
  inline void SetWallPercentage(float MapWallPercentage) { WallPercentage = MapWallPercentage; }

  inline const Tiles_t& GetMap() const { return Tiles; }

  inline const ETiles& operator() (int x, int y) const
  {
    assert(x >= 0 && x < Width);
    assert(y >= 0 && y < Height);
    assert(nullptr != Tiles);

    return Tiles[Width * y + x];
  }

  int GetNeighboursCode(int x, int y) const;

  friend std::ostream& operator<< (std::ostream& OutputStream, const Map& MapToPrint);


private:
  //Make copy constructor private
  Map(const Map&) = delete;
  inline ETiles& operator() (int x, int y)
  {
    assert(x >= 0 && x < Width);
    assert(y >= 0 && y < Height);
    assert(nullptr != Tiles);

    return Tiles[Width * y + x];
  }

  bool GenerateMap();
  void MakeCaverns();
  void RemoveStray();
  void RemoveStraySame(const ETiles& Type);
  void PlaceWater();
  void PlaceProps();

  ETiles GetTileFor(int x, int y) const;
  ETiles CheckStray(int x, int y) const;
  ETiles CheckStraySame(int x, int y, const ETiles& Type) const;
  int GetAdjacentWallCount(int x, int y, const Window& SearchWindow) const;
  int GetAdjacentSameCount(int x, int y, const Window& SearchWindow) const;
  bool IsWall(int x, int y) const;
  bool IsFloor(int x, int y) const;
  bool IsTheSame(int x, int y, const ETiles& TileTypeOther) const;
  bool OutOfBounds(int x, int y) const;

  bool MapSound() const;

  static std::string TileTypeAsStringCache[];
};

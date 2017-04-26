#pragma once

#include <iostream>
#include <memory>
#include <cassert>

typedef std::pair<int,int> Window;

class Map
{
public:

  enum ETiles {
    Floor,
    Wall,
  };

  typedef std::unique_ptr<ETiles []> Map_t;

private:
  int Width; //Map's width (measured in tiles)
  int Height; //Map's height (measured in tiles)
  float   WallPercentage; //The percentage of the map that's filled with walls
  Map_t   Tiles; //The map

public:
  Map(int MapWidth, int MapHeight, float MapWallPercentage);
  ~Map();

  bool GenerateMap();
  void MakeCaverns();

  inline int GetWidth() const { return Width; }
  inline int GetHeight() const { return Height; } const
  inline float GetWallPercentage() const { return WallPercentage; }

  inline void SetWidth(int MapWidth) { Width = MapWidth; }
  inline void SetHeight(int MapHeight) { Height = MapHeight; }
  inline void SetWallPercentage(float MapWallPercentage) { WallPercentage = MapWallPercentage; }

  inline const Map_t& GetMap() const { return Tiles; }

  inline const ETiles& operator() (int x, int y) const
  {
    assert(x >= 0 && x < Width);
    assert(y >= 0 && y < Height);
    assert(nullptr != Tiles);

    return Tiles[Width * y + x];
  }

  friend std::ostream& operator<< (std::ostream& OutputStream, const Map& MapToPrint);

private:
  //Make copy constructor private
  Map(const Map&);
  inline ETiles& operator() (int x, int y)
  {
    assert(x >= 0 && x < Width);
    assert(y >= 0 && y < Height);
    assert(nullptr != Tiles);

    return Tiles[Width * y + x];
  }

  ETiles GetTileFor(int x, int y) const;
  int GetAdjacentWallCount(int x, int y, const Window& SearchWindow) const;
  bool IsWall(int x, int y) const;
  bool OutOfBounds(int x, int y) const;
};

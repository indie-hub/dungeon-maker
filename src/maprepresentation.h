#pragma once

#include "map.h"

class MapRepresentation
{
private:
  Map::Map_ptr TheMap;

public:
  MapRepresentation(Map::Map_ptr &Map)
    : TheMap(Map)
  {}

private:
  MapRepresentation() = delete;
  MapRepresentation(const MapRepresentation&) = delete;
};

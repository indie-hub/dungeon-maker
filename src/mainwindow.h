#include <nanogui/screen.h>
#include "map.h"

class MainWindow : public nanogui::Screen
{
private:
  int MapWidth;
  int MapHeight;
  float MapWallPercentage;
  Map TheMap;

public:
  MainWindow(int Width, int Height, const std::string& Title = "")
    : nanogui::Screen(Eigen::Vector2i(Width, Height), Title)
    , MapWidth(60)
    , MapHeight(40)
    , MapWallPercentage(0.40)
    , TheMap(MapWidth, MapHeight, MapWallPercentage)
  {
    DoLayout();
    TheMap.Generate();
  }

  ~MainWindow() {}

private:
  void DoLayout();
};

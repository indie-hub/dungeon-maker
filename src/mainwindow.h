#pragma once

#include <nanogui/screen.h>
#include "map.h"
#include "maprepresentation.h"


class MainWindow : public nanogui::Screen
{
private:
  int                                       MapWidth;
  int                                       MapHeight;
  float                                     MapWallPercentage;
  int                                       MapAlgorithmSteps;

  Map::Map_ptr                              TheMap;
  MapRepresentation::MapRepresentation_ptr  TheMapRepresentation;

  Eigen::Vector2i                           WindowSize;

  Eigen::Matrix4f                           CameraMatrix;
  bool                                      MapSetUp;
  float                                     Zoom;

public:
  MainWindow(int Width, int Height, const std::string& Title = "")
    : nanogui::Screen(Eigen::Vector2i(Width, Height), Title)
    , MapWidth(60)
    , MapHeight(40)
    , MapWallPercentage(0.40f)
    , MapAlgorithmSteps(1)
    , TheMap(nullptr)
    , TheMapRepresentation(nullptr)
    , WindowSize(Width, Height)
    , CameraMatrix()
    , MapSetUp(false)
    , Zoom(1.f)
  {

    const std::function<void(Eigen::Vector2i)> Callback =
                [=](Eigen::Vector2i WindowSize)
                {
                  this->OnResize(WindowSize);
                };
    setResizeCallback(Callback);
  }

  virtual void drawContents();


  void SetupMap(int Width, int Height, float WallPercentage,
                const std::string& RepresentationConfigFile);

  ~MainWindow() {}

private:
  void DoLayout();
  void OnResize(const Eigen::Vector2i WindowSize);
  void ComputeCamera();
};

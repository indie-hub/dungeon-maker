#include "mainwindow.h"

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>

#include <nanogui/button.h>
#include <nanogui/checkbox.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/textbox.h>
#include <nanogui/window.h>


void MainWindow::DoLayout()
{
  nanogui::Window *Window = new nanogui::Window(this, "Map Parameters");
  Window->setPosition(Eigen::Vector2i(100, 100));

  nanogui::GridLayout *Layout = new nanogui::GridLayout
                (nanogui::Orientation::Horizontal, 2,
                 nanogui::Alignment::Middle, 15, 4);
  Layout->setColAlignment({ nanogui::Alignment::Maximum,
                            nanogui::Alignment::Fill });
  Layout->setSpacing(0, 10);
  Window->setLayout(Layout);

  setBackground(nanogui::Color(0.f, 0.f, 0.f, 1.f));

  {
    new nanogui::Label(Window, "Width: ", "sans-bold");
    auto TextBox = new nanogui::IntBox<int>(Window);
    TextBox->setEditable(true);
    TextBox->setFixedSize(Eigen::Vector2i(100, 20));
    TextBox->setValue(MapWidth);
    TextBox->setUnits("tiles");
    TextBox->setFontSize(16);
    TextBox->setFormat("[1-9][0-9]*");
    TextBox->setSpinnable(true);
    TextBox->setMinValue(1);
    TextBox->setValueIncrement(1);
    TextBox->setCallback(
        [&](const int& Value)
        {
          MapWidth = Value;
        }
      );
  }

  {
    new nanogui::Label(Window, "Height: ", "sans-bold");
    auto TextBox = new nanogui::IntBox<int>(Window);
    TextBox->setEditable(true);
    TextBox->setFixedSize(Eigen::Vector2i(100, 20));
    TextBox->setValue(MapHeight);
    TextBox->setUnits("tiles");
    TextBox->setFontSize(16);
    TextBox->setFormat("[1-9][0-9]*");
    TextBox->setSpinnable(true);
    TextBox->setMinValue(1);
    TextBox->setValueIncrement(1);
    TextBox->setCallback(
        [&](const int& Value)
        {
          MapHeight = Value;
        }
      );
  }

  {
    new nanogui::Label(Window, "Wall %: ", "sans-bold");
    auto TextBox = new nanogui::FloatBox<float>(Window);
    TextBox->setEditable(true);
    TextBox->setFixedSize(Eigen::Vector2i(100, 20));
    TextBox->setValue(MapWallPercentage);
    TextBox->setFontSize(16);
    TextBox->setFormat("0\\.[0-9]+");
    TextBox->setSpinnable(true);
    TextBox->setMinValue(0);
    TextBox->setMaxValue(1);
    TextBox->setValueIncrement(0.05);
    TextBox->setCallback(
        [&](const float& Value)
        {
          MapWallPercentage = Value;
        }
      );
  }

  {
    new nanogui::Label(Window, "Alg. Steps: ", "sans-bold");
    auto TextBox = new nanogui::IntBox<int>(Window);
    TextBox->setEditable(true);
    TextBox->setFixedSize(Eigen::Vector2i(100, 20));
    TextBox->setValue(MapAlgorithmSteps);
    TextBox->setUnits("steps");
    TextBox->setFontSize(16);
    TextBox->setFormat("[1-9][0-9]*");
    TextBox->setSpinnable(true);
    TextBox->setMinValue(1);
    TextBox->setMaxValue(10);
    TextBox->setValueIncrement(1);
    TextBox->setCallback(
        [&](const int& Value)
        {
          MapAlgorithmSteps = Value;
        }
      );
  }

  {
    new nanogui::Label(Window, "", "sans-bold");
    nanogui::Button *GenerateBtn = new nanogui::Button(Window, "Generate");
    GenerateBtn->setCallback([&]
      {
        if(MapWidth != TheMap->GetWidth() || MapHeight != TheMap->GetHeight())
        {
          TheMap->SetWidth(MapWidth);
          TheMap->SetHeight(MapHeight);
        }
        TheMap->SetWallPercentage(MapWallPercentage);
        TheMap->Generate(MapAlgorithmSteps);

        std::cout << *TheMap << std::endl;
      });
  }

  {
    new nanogui::Label(Window, "", "sans-bold");
    nanogui::CheckBox *DisplayGridCheckBox = new nanogui::CheckBox(Window, "Display Grid");
      DisplayGridCheckBox->setCallback(
            [&](bool state)
            {
              TheMapRepresentation->SetDisplayGrid(state);
            });
  }

  {
    new nanogui::Label(Window, "Zoom: ", "sans-bold");
    auto TextBox = new nanogui::IntBox<int>(Window);
    TextBox->setEditable(true);
    TextBox->setFixedSize(Eigen::Vector2i(100, 20));
    TextBox->setValue(100);
    TextBox->setUnits("%");
    TextBox->setFontSize(16);
    TextBox->setFormat("[1-9][0-9]*");
    TextBox->setSpinnable(true);
    TextBox->setMinValue(10);
    TextBox->setMaxValue(200);
    TextBox->setValueIncrement(10);
    TextBox->setCallback(
        [&](const int& Value)
        {
          //MapWallPercentage = Value;
          Zoom = 1.f / (Value / 100.f);
          ComputeCamera();
        }
      );
  }

  performLayout();

  ComputeCamera();
}

void MainWindow::drawContents()
{
  if(MapSetUp)
  {
    TheMapRepresentation->DrawUsingCamera(CameraMatrix);
  }
}

void MainWindow::SetupMap(int Width, int Height, float WallPercentage,
            const std::string& RepresentationConfigFile)
{
  if(nullptr != TheMap)
  {
    TheMap = nullptr;
    TheMapRepresentation = nullptr;
  }
  MapWidth = Width;
  MapHeight = Height;
  MapWallPercentage = WallPercentage;

  TheMap = Map::Map_ptr(new Map(MapWidth, MapHeight, MapWallPercentage));
  TheMapRepresentation = MapRepresentation::MapRepresentation_ptr(
                          new MapRepresentation(RepresentationConfigFile, TheMap));
  if(!TheMapRepresentation->Init(WindowSize))
  {
    std::cerr << "Error initializing map representation" << std::endl;
  }
  else
  {
    MapSetUp = true;
  }

  DoLayout();
}

void MainWindow::OnResize(Eigen::Vector2i NewWindowSize)
{
  WindowSize = NewWindowSize;

  ComputeCamera();
}


void MainWindow::ComputeCamera()
{
  const int HalfWidth = WindowSize.x() / 2;
  const int HalfHeight = WindowSize.y() / 2;

  CameraMatrix = nanogui::ortho(-HalfWidth * Zoom, HalfWidth * Zoom,
                                HalfHeight * Zoom, -HalfHeight * Zoom,
                                1.0f, -1.0f);
}

#include "mainwindow.h"

#include <nanogui/button.h>
#include <nanogui/glutil.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/opengl.h>
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
    new nanogui::Label(Window, "", "sans-bold");
    nanogui::Button *GenerateBtn = new nanogui::Button(Window, "Generate");
    GenerateBtn->setCallback([&]
      {
        if(MapWidth != TheMap.GetWidth() || MapHeight != TheMap.GetHeight())
        {
          TheMap.SetWidth(MapWidth);
          TheMap.SetHeight(MapHeight);
        }
        TheMap.SetWallPercentage(MapWallPercentage);
        TheMap.Generate();

        std::cout << TheMap << std::endl;
      });
  }

  performLayout();

}

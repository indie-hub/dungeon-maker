#include "mainwindow.h"

#define ERROR_CODE -1

int main(int argc, char** argv)
{
  try
  {
    nanogui::init();
    {
      nanogui::ref<MainWindow> App = new MainWindow(1024, 768, "Dungeon Maker");

      App->drawAll();
      App->setVisible(true);
      App->SetupMap(60, 40, 0.40f, "data/maprepresentation.json"),

      nanogui::mainloop();
    }

    nanogui::shutdown();
  }
  catch(const std::runtime_error &e)
  {
    std::string ErrorMsg = std::string("Fatal error: " + std::string(e.what()));
    std::cerr << ErrorMsg << std::endl;
    return ERROR_CODE;
  }

  return 0;
}

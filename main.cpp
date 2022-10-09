
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QVBoxLayout>

#include "window.h"
#include <fenv.h>

int main (int argc, char *argv[])
{

  feenableexcept (FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);

  QApplication app (argc, argv);

  QMainWindow *window = new QMainWindow;
  Window *graph_area = new Window (window);

  if (graph_area->parse_command_line (argc, argv))
    {
      QMessageBox::warning (0, "Wrong input arguments!", "Wrong input arguments!");
      return -1;
    }

  graph_area->create_toolbar (window);

  window->setCentralWidget (graph_area);
  window->setWindowTitle ("Graph");

  window->show ();
  app.exec ();
  delete window;
  return 0;
}

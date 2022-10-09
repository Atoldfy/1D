#pragma once

#include <QtWidgets/QtWidgets>

#include "functions_and_derivatives.h"
#include "calculator.h"
#include <functional>
#include <memory>

enum class paint_event_t
{
  newton,
  newton_resid,
  spline,
  spline_resid,
  two_at_once,
  two_at_once_resid,

  COUNT
};

const char *enum_to_string (paint_event_t type);
void set_next (paint_event_t &cur_func);


class Window : public QWidget
{
  Q_OBJECT

private:
  paint_event_t m_cur_event = paint_event_t::spline;
  common_params_t m_params;

  precision_t m_precision;

  std::unique_ptr<calculator_t> m_base_func;
  std::unique_ptr<newton_t> m_newton;
  std::unique_ptr<spline_t> m_spline;
  std::unique_ptr<residual_t> m_newton_resid;
  std::unique_ptr<residual_t> m_spline_resid;

  int get_u (double x);
  int get_v (double y, std::pair<double, double> y_limits);

  double get_x (int u);
  double get_y (int v, std::pair<double, double> y_limits);

public:
  Window (QWidget *parent);

  QSize minimumSizeHint () const;
  QSize sizeHint () const;

  int parse_command_line (int argc, char *argv[]);
  void create_toolbar (QMainWindow *window);


  const char *change_func_real ();
  const char *change_event_real ();

  const char *zoom_in_real ();
  const char *zoom_out_real ();

  const char *rais_N_real ();
  const char *reduce_N_real ();

  const char *add_precision_real ();
  const char *remove_precision_real ();

public slots:
  void change_func ();
  void change_event ();

  void zoom_in ();
  void zoom_out ();

  void rais_N ();
  void reduce_N ();

  void add_precision ();
  void remove_precision ();


protected:
  void paintEvent (QPaintEvent *event);
  void drow_calculator (QPainter &painter, calculator_t *calculator, std::pair<double, double> y_limits);
};


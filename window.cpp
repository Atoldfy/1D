
#include <QPainter>

#include "calculator.h"
#include "functions_and_derivatives.h"
#include "window.h"

const char *enum_to_string (paint_event_t type)
{
  switch (type)
    {
      case paint_event_t::newton:
        return "Newton";
      case paint_event_t::spline:
        return "Spline";
      case paint_event_t::two_at_once:
        return "Spline and Newton";
      case paint_event_t::newton_resid:
        return "Newton's residual";
      case paint_event_t::spline_resid:
        return "Spline's residual";
      case paint_event_t::two_at_once_resid:
        return "Newton's and Spline's residual";

      case paint_event_t::COUNT:
        return "Invalid event type";
    }
  return "";
}

void set_next (paint_event_t &cur_event)
{
  int next_event = (static_cast<int> (cur_event) + 1) % static_cast<int> (paint_event_t::COUNT);
  cur_event = static_cast<paint_event_t> (next_event);
}

int Window::get_u (double x) { return 1 + (width () - 1) * ((x - m_params.left_bound) / (m_params.right_bound - m_params.left_bound)); }

int Window::get_v (double y, std::pair<double, double> y_limits)
{
  return height () - (1 + (height () - 1) * ((y - y_limits.first) / (y_limits.second - y_limits.first)));
}

double Window::get_x (int u) { return m_params.left_bound + (u - 1) * (m_params.right_bound - m_params.left_bound) / (width () - 1); }

double Window::get_y (int v, std::pair<double, double> y_limits)
{
  return y_limits.first + (v - 1) * (y_limits.second - y_limits.first) / (height () - 1);
}

Window::Window (QWidget *parent) : QWidget (parent), m_precision (m_params)
{
  change_func ();
  m_base_func = std::make_unique<calculator_t> (m_params, m_precision);
  m_newton = std::make_unique<newton_t> (m_params, m_precision);
  m_spline = std::make_unique<spline_t> (m_params, m_precision);
  m_newton_resid = std::make_unique<residual_t> (m_params, m_precision, m_newton.get ());
  m_spline_resid = std::make_unique<residual_t> (m_params, m_precision, m_spline.get ());
}

QSize Window::minimumSizeHint () const { return QSize (100, 100); }

QSize Window::sizeHint () const { return QSize (1000, 1000); }

int Window::parse_command_line (int argc, char *argv[])
{
  if (argc != 5)
    return -1;

  int formul_type;

  if (sscanf (argv[1], "%lf", &m_params.left_bound) != 1 || sscanf (argv[2], "%lf", &m_params.right_bound) != 1
      || m_params.right_bound - m_params.left_bound < 1.e-6 || sscanf (argv[3], "%d", &m_params.number_points) != 1
      || m_params.number_points <= 0 || sscanf (argv[4], "%d", &formul_type) != 1 || (formul_type < 0 || formul_type > 6))
    return -2;

  m_params.func_type = static_cast<function_type> (formul_type);

  return 0;
}

void Window::create_toolbar (QMainWindow *window)
{
  QMenuBar *tool_bar = new QMenuBar (window);

  tool_bar->addAction ("&Change function", this, SLOT (change_func ()))->setShortcut (QString ("0"));
  tool_bar->addAction ("&Change event", this, SLOT (change_event ()))->setShortcut (QString ("1"));
  tool_bar->addAction ("&Zoom In", this, SLOT (zoom_in ()))->setShortcut (QString ("2"));
  tool_bar->addAction ("&Zoom Out", this, SLOT (zoom_out ()))->setShortcut (QString ("3"));
  tool_bar->addAction ("&Raise N", this, SLOT (rais_N ()))->setShortcut (QString ("4"));
  tool_bar->addAction ("&Reduce N", this, SLOT (reduce_N ()))->setShortcut (QString ("5"));
  tool_bar->addAction ("&Add Precision", this, SLOT (add_precision ()))->setShortcut (QString ("6"));
  tool_bar->addAction ("&Remove Precision", this, SLOT (remove_precision ()))->setShortcut (QString ("7"));
  tool_bar->addAction ("Exit", this, SLOT (close ()))->setShortcut (QString ("Ctrl+X"));

  tool_bar->setMaximumHeight (30);
  window->setMenuBar (tool_bar);
}


const char *Window::change_func_real ()
{
  function_type tmp_type = m_params.func_type;
  set_next (tmp_type);

  auto bounds = get_function_bounds_by_type (tmp_type);
  if (m_params.left_bound < bounds.first || m_params.right_bound > bounds.second)
    return "ERROR! Incorrect limits for function.\n";

  m_params.func_type = tmp_type;
  update ();
  return "";
}

const char *Window::change_event_real ()
{
  paint_event_t tmp_type = m_cur_event;
  set_next (tmp_type);

  if (tmp_type != paint_event_t::spline && tmp_type != paint_event_t::spline_resid && m_params.number_points > newton_limits_max)
    {
      if (tmp_type == paint_event_t::newton || tmp_type == paint_event_t::two_at_once)
        m_cur_event = paint_event_t::spline;

      if (tmp_type == paint_event_t::newton_resid || tmp_type == paint_event_t::two_at_once_resid)
        m_cur_event = paint_event_t::spline_resid;

      return "ERROR! N is too much for Newton.\nNewton is replaced to a Spline.";
    }

  m_cur_event = tmp_type;
  update ();
  return "";
}

const char *Window::zoom_in_real ()
{
  double a = m_params.left_bound;
  double b = m_params.right_bound;
  double mid = (a + b) / 2.;

  b = (b - mid) / 2. + mid;
  a = (a - mid) / 2. + mid;

  if (fabs (b - a) < 1e-5)
    return "ERROR! Too small lenght";

  auto limits = get_function_bounds_by_type (m_params.func_type);
  if (a < limits.first || b > limits.second)
    return "ERROR! Is incorrect limits for function.\n";

  m_params.left_bound = a;
  m_params.right_bound = b;

  m_precision.set_dirty ();
  return "";
}

const char *Window::zoom_out_real ()
{
  double a = m_params.left_bound;
  double b = m_params.right_bound;
  double mid = (a + b) / 2.;

  b = (b - mid) * 2. + mid;
  a = (a - mid) * 2. + mid;

  if (fabs (b - a) > 1e306)
    return "ERROR! Too big length";

  auto limits = get_function_bounds_by_type (m_params.func_type);
  if (a < limits.first || b > limits.second)
    return "ERROR! Is incorrect limits for function.\n";

  m_params.left_bound = a;
  m_params.right_bound = b;

  m_precision.set_dirty ();
  return "";
}

const char *Window::rais_N_real ()
{
  int tmp_n = m_params.number_points * 2.;

  if (m_cur_event == paint_event_t::newton || m_cur_event == paint_event_t::two_at_once || m_cur_event == paint_event_t::newton_resid
      || m_cur_event == paint_event_t::two_at_once_resid)
    {
      if (tmp_n > newton_limits_max)
        return "ERROR! N is too big for Newton.\n";
      else if (tmp_n < newton_limits_min)
        return "ERROR! N is too small for Newton.\n";
    }

  if (m_cur_event == paint_event_t::spline || m_cur_event == paint_event_t::two_at_once || m_cur_event == paint_event_t::spline_resid
      || m_cur_event == paint_event_t::two_at_once_resid)
    {
      if (tmp_n < spline_limits_min)
        return "ERROR! N is too big for Spline.";
    }

  m_params.number_points = tmp_n;
  m_precision.set_dirty ();
  return "";
}

const char *Window::reduce_N_real ()
{
  int tmp_n = m_params.number_points / 2.;

  if (m_cur_event == paint_event_t::newton || m_cur_event == paint_event_t::two_at_once || m_cur_event == paint_event_t::newton_resid
      || m_cur_event == paint_event_t::two_at_once_resid)
    {
      if (tmp_n > newton_limits_max)
        return "ERROR! N is too big for Newton.\n";
      else if (tmp_n < newton_limits_min)
        return "ERROR! N is too small for Newton.\n";
    }

  if (m_cur_event == paint_event_t::spline || m_cur_event == paint_event_t::two_at_once || m_cur_event == paint_event_t::spline_resid
      || m_cur_event == paint_event_t::two_at_once_resid)
    {
      if (tmp_n < spline_limits_min)
        return "ERROR! N is too big for Spline.";
    }

  m_params.number_points = tmp_n;
  m_precision.set_dirty ();
  return "";
}

const char *Window::add_precision_real ()
{
  m_precision.raise_coefficient ();
  return "";
}

const char *Window::remove_precision_real ()
{
  m_precision.reduce_coefficient ();
  return "";
}

void Window::change_func ()
{
  QString ret_err = change_func_real ();
  (void) ret_err;
//  if (ret_err != "")
//    QMessageBox::warning (0, "Warning", ret_err);

  update ();
}

void Window::change_event ()
{
  QString ret_err = change_event_real ();
  (void) ret_err;
//  if (ret_err != "")
//    QMessageBox::warning (0, "Warning", ret_err);

  update ();
}

void Window::zoom_in ()
{
  QString ret_err = zoom_in_real ();
  (void) ret_err;
  if (ret_err != "")
    QMessageBox::warning (0, "Warning", ret_err);

  update ();
}

void Window::zoom_out ()
{
  QString ret_err = zoom_out_real ();
  (void) ret_err;
  if (ret_err != "")
    QMessageBox::warning (0, "Warning", ret_err);

  update ();
}

void Window::rais_N ()
{
  QString ret_err = rais_N_real ();
  (void) ret_err;
//  if (ret_err != "")
//    QMessageBox::warning (0, "Warning", ret_err);

  update ();
}

void Window::reduce_N ()
{
  QString ret_err = reduce_N_real ();
  (void) ret_err;
//  if (ret_err != "")
//    QMessageBox::warning (0, "Warning", ret_err);

  update ();
}

void Window::add_precision ()
{
  QString ret_err = add_precision_real ();
  (void) ret_err;
//  if (ret_err != "")
//    QMessageBox::warning (0, "Warning", ret_err);

  update ();
}

void Window::remove_precision ()
{
  QString ret_err = remove_precision_real ();
  (void) ret_err;
//  if (ret_err != "")
//    QMessageBox::warning (0, "Warning", ret_err);

  update ();
}


void Window::drow_calculator (QPainter &painter, calculator_t *calculator, std::pair<double, double> y_limits)
{
  QPen pen ((Qt::GlobalColor) calculator->get_color (), 1, (Qt::PenStyle) calculator->get_line_style ());
  painter.setPen (pen);

  auto func = calculator->value_getter ();

  int x1 = 1;
  int y1 = get_v (func (get_x (x1)), y_limits);

  int x2 = 0;
  int y2 = 0;

  for (int i = 1; i < width (); i++)
    {
      x2 = x1 + 1;
      y2 = get_v (func (get_x (x2)), y_limits);

      painter.drawLine (QPointF (x1, y1), QPointF (x2, y2));

      x1 = x2;
      y1 = y2;
    }

  painter.drawLine (QPointF (x1, y1), QPointF (x2, y2));
}

/// render graph
void Window::paintEvent (QPaintEvent * /* event */)
{
  QPainter painter (this);

  auto get_scaling_limits = [&] (calculator_t *calculator) {
    auto y_limits = minmax (m_params.left_bound, m_params.right_bound, calculator->value_getter (), m_params.number_points);
    double delta_y = (y_limits.second - y_limits.first) > eps ? 0.01 * (y_limits.second - y_limits.first) : 0.01;
    y_limits.first -= delta_y;
    y_limits.second += delta_y;
    return y_limits;
  };

  std::vector<calculator_t *> calculators_to_paint;
  std::pair<double, double> scaling_limits;
  switch (m_cur_event)
    {
      case paint_event_t::newton:
        calculators_to_paint.push_back (m_newton.get ());
        calculators_to_paint.push_back (m_base_func.get ());
        break;
      case paint_event_t::newton_resid:
        calculators_to_paint.push_back (m_newton_resid.get ());
        break;
      case paint_event_t::spline:
        calculators_to_paint.push_back (m_spline.get ());
        calculators_to_paint.push_back (m_base_func.get ());
        break;
      case paint_event_t::spline_resid:
        calculators_to_paint.push_back (m_spline_resid.get ());
        break;
      case paint_event_t::two_at_once:
        calculators_to_paint.push_back (m_spline.get ());
        calculators_to_paint.push_back (m_newton.get ());
        calculators_to_paint.push_back (m_base_func.get ());
        break;
      case paint_event_t::two_at_once_resid:
        calculators_to_paint.push_back (m_spline_resid.get ());
        calculators_to_paint.push_back (m_newton_resid.get ());
        break;

      case paint_event_t::COUNT:
        abort ();
    }
  for (calculator_t *calculator : calculators_to_paint)
    calculator->update (m_params);

  painter.save ();


  scaling_limits = get_scaling_limits (calculators_to_paint.back ());

  for (calculator_t *calculator : calculators_to_paint)
    drow_calculator (painter, calculator, scaling_limits);

  // draw axis
  QPen pen_black (Qt::black, 3, Qt::SolidLine);
  painter.setPen (pen_black);

  painter.drawLine (0, get_v (0, scaling_limits), width (), get_v (0, scaling_limits));  // x axis
  painter.drawLine (get_u (0), 0, get_u (0), height ());                                 // y axis


  // render function name
  //painter.setPen ("blue");
  painter.drawText (30, 40, "k = " + QString::number (static_cast<int> (m_params.func_type)) + "  " + enum_to_string (m_params.func_type));
  painter.drawText (30, 60, enum_to_string (m_cur_event));
  painter.drawText (30, 80, "N = " + QString::number (m_params.number_points));
  painter.drawText (30, 100, "[a;b] = [" + QString::number (m_params.left_bound) + ";" + QString::number (m_params.right_bound) + "]");
  painter.drawText (
      30, 120,
      "max{|Fmin|, |Fmax|} = "
          + QString::
              number (fabs (scaling_limits.first) > fabs (scaling_limits.second) ? fabs (scaling_limits.first) : fabs (scaling_limits.second), 'e', 2));
  painter.drawText (30, 140, "Precision = " + QString::number (m_precision.get_value ()));

  printf ("%s\n", enum_to_string (m_params.func_type));
  printf ("%s\n", enum_to_string (m_cur_event));
  printf ("N = %d\n", m_params.number_points);
  printf ("[a;b] = [%lf;%lf]\n", m_params.left_bound, m_params.right_bound);
  printf (
      "max{|Fmin|, |Fmax|} = %e\n",
      fabs (scaling_limits.first) > fabs (scaling_limits.second) ? fabs (scaling_limits.first) : fabs (scaling_limits.second));
  printf ("Precision = %lf\n", m_precision.get_value ());
}

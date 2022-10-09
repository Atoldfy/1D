#include "calculator.h"

#include <cmath>

calculator_t::calculator_t (common_params_t init_params, precision_t &precision) : m_precision (precision) { m_cur_params = init_params; }

std::function<double (double)> calculator_t::value_getter ()
{
  return [&] (double x) { return get_value (x); };
}

int calculator_t::get_color () { return 7; }
int calculator_t::get_line_style () { return 1; }

void calculator_t::resize_all () {}
void calculator_t::resolve () {}

void calculator_t::update (common_params_t new_params)
{
  if (m_cur_params.func_type == new_params.func_type && m_cur_params.number_points == new_params.number_points
      && math_equal (m_cur_params.left_bound, new_params.left_bound) && !math_equal (m_cur_params.right_bound, new_params.right_bound))
    return;

  m_cur_params = new_params;
  resize_all ();
  resolve ();
}

double calculator_t::get_value (double x) { return get_func_by_type (m_cur_params.func_type) (x); }


newton_t::newton_t (common_params_t init_params, precision_t &precision) : calculator_t (init_params, precision) {}

int newton_t::get_color () { return 9; /*blue*/ }

int newton_t::get_line_style () { return 1; /*SolidLine*/ }

void newton_t::resize_all ()
{
  x_v.resize (m_cur_params.number_points);
  y_v.resize (m_cur_params.number_points);
}

void newton_t::resolve ()
{
  auto func = get_func_by_type (m_cur_params.func_type);
  double delta = (m_cur_params.left_bound - m_cur_params.right_bound) / (m_cur_params.number_points - 1);

  for (int i = 0; i < m_cur_params.number_points; i++)
    {
      y_v[i] = func (m_cur_params.right_bound + delta * i);
      x_v[i] = m_cur_params.right_bound + delta * i;
    }

  y_v[m_cur_params.number_points / 2] += m_precision.get_value ();

  for (int i = 1; i < m_cur_params.number_points; i++)
    for (int j = m_cur_params.number_points - 1; j >= i; j--)
      y_v[j] = (y_v[j] - y_v[j - 1]) / (x_v[j] - x_v[j - i]);
}

double newton_t::get_value (double x)
{
  double res = 0;
  for (int i = m_cur_params.number_points - 2; i >= 0; i--)
    {
      res += y_v[i + 1];
      res = res * (x - x_v[i]);
    }
  return res + y_v[0];
}


spline_t::spline_t (common_params_t init_params, precision_t &precision) : calculator_t (init_params, precision) {}

int spline_t::get_color () { return 8; /*green*/ }

int spline_t::get_line_style () { return 1; /*SolidLine*/ }

void spline_t::resize_all ()
{
  v_x.resize (m_cur_params.number_points);
  v_y.resize (m_cur_params.number_points);
  v_rside.resize (m_cur_params.number_points + 1);
  v_right.resize (m_cur_params.number_points + 1);
  v_eps.resize (m_cur_params.number_points + 1);
  v_left.resize (m_cur_params.number_points + 1);
  v_diagonal.resize (m_cur_params.number_points + 1);
  v_ans_1.resize (m_cur_params.number_points + 1);
  v_ans_2.resize (m_cur_params.number_points + 1);
  v_ans_3.resize (m_cur_params.number_points + 1);
  help_v.resize (m_cur_params.number_points + 1);
}

void spline_t::resolve ()
{
  auto func = get_func_by_type (m_cur_params.func_type);
  auto deriv = get_second_deriv_by_type (m_cur_params.func_type);
  auto deriv_1 = get_first_deriv_by_type (m_cur_params.func_type);
  double step = (m_cur_params.right_bound - m_cur_params.left_bound) / (m_cur_params.number_points - 1);
  int N = m_cur_params.number_points;

  for (int i = 0; i < N; i++)
    {
      v_y[i] = func (m_cur_params.left_bound + step * i);
      v_x[i] = m_cur_params.left_bound + step * i;
    }
  v_y[N / 2] += m_precision.get_value ();

  for (int i = 1; i < N; i++)
    v_eps[i] = (v_x[i] + v_x[i - 1]) / 2.;
  v_eps[0] = m_cur_params.left_bound - step / 2;
  v_eps[N] = m_cur_params.right_bound + step / 2;

  for (int i = 1; i < N; i++)
    {
      v_left[i] = 1. / (v_x[i - 1] - v_eps[i - 1]);
      v_left[i] -= 1. / (v_eps[i] - v_eps[i - 1]);

      v_diagonal[i] = 1. / (v_eps[i] - v_x[i - 1]);
      v_diagonal[i] += 1. / (v_eps[i] - v_eps[i - 1]);
      v_diagonal[i] += 1. / (v_x[i] - v_eps[i]);
      v_diagonal[i] += 1. / (v_eps[i + 1] - v_eps[i]);

      v_right[i] = 1. / (v_eps[i + 1] - v_x[i]);
      v_right[i] -= 1. / (v_eps[i + 1] - v_eps[i]);

      v_rside[i] = ((1. / (v_x[i - 1] - v_eps[i - 1])) + (1. / (v_eps[i] - v_x[i - 1]))) * v_y[i - 1];
      v_rside[i] += ((1. / (v_x[i] - v_eps[i])) + (1. / (v_eps[i + 1] - v_x[i]))) * v_y[i];
    }

//  v_left[0] = 0;
//  v_diagonal[0] = 2. / ((v_eps[1] - v_eps[0]) * (v_x[0] - v_eps[0]));
//  v_right[0] = 2. / ((v_eps[1] - v_eps[0]) * ((v_eps[1] - v_x[0])));

//  v_left[N] = 2. / ((v_eps[N] - v_eps[N - 1]) * (v_x[N - 1] - v_eps[N - 1]));
//  v_diagonal[N] = 2. / ((v_eps[N] - v_eps[N - 1]) * (v_eps[N] - v_x[N - 1]));
//  v_right[N] = 0;

//  v_rside[0] = deriv (v_x[0]);
//  v_rside[0] += v_y[0] * (1. / (v_x[0] - v_eps[0]) + 1. / (v_eps[1] - v_x[0])) * (2. / (v_eps[1] - v_eps[0]));
//  v_rside[N] = deriv (v_x[N - 1]);
//  v_rside[N] += v_y[N - 1] * (1. / (v_x[N - 1] - v_eps[N - 1]) + 1. / (v_eps[N] - v_x[N - 1])) * (2. / (v_eps[N] - v_eps[N - 1]));

  v_left[0] = 0;
  v_left[N] = 1. / (v_eps[N] - v_eps[N - 1]);
  v_left[N] -= 1. / (v_x[N - 1] - v_eps[N - 1]);

  v_diagonal[0] = 1. / (v_eps[1] - v_eps[0]);
  v_diagonal[0] -= 1. / (v_x[0] - v_eps[0]);
  v_diagonal[N] = 1. / (v_eps[N] - v_x[N - 1]);
  v_diagonal[N] -= 1. / (v_eps[N] - v_eps[N - 1]);

  v_right[0] = 1. / (v_eps[1] - v_x[0]);
  v_right[0] -= 1. / (v_eps[1] - v_eps[0]);
  v_right[N] = 0;

  v_rside[0] = deriv_1 (v_x[0]);
  v_rside[0] -= ((1. / (v_x[0] - v_eps[0])) - (1. / (v_eps[1] - v_x[0]))) * v_y[0];
  v_rside[N] = deriv_1 (v_x[N - 1]);
  v_rside[N] -= v_y[N - 1] * ((1. / (v_x[N - 1] - v_eps[N - 1])) - (1. / (v_eps[N] - v_x[N - 1])));

  for (int i = 0; i < N; i++)
    {
      v_right[i] /= v_diagonal[i];
      v_rside[i] /= v_diagonal[i];
      v_diagonal[i + 1] -= v_right[i] * v_left[i + 1];
      v_rside[i + 1] -= v_rside[i] * v_left[i + 1];
    }
  v_rside[N] /= v_diagonal[N];
  for (int i = N; i > 0; i--)
    v_rside[i - 1] -= v_rside[i] * v_right[i - 1];

  for (int i = 0; i < N + 1; i++)
    help_v[i] = v_rside[i];


  for (int i = 0; i < N; i++)
    {
      v_ans_1[i] = help_v[i];
      v_ans_2[i] = (v_y[i] - help_v[i]) / (v_x[i] - v_eps[i]);
      v_ans_2[i] -= ((v_x[i] - v_eps[i]) / (v_eps[i + 1] - v_eps[i]))
                    * (((help_v[i + 1] - v_y[i]) / (v_eps[i + 1] - v_x[i])) - ((v_y[i] - help_v[i]) / (v_x[i] - v_eps[i])));
      v_ans_3[i] = (1. / (v_eps[i + 1] - v_eps[i])) * (((help_v[i + 1] - v_y[i]) / (v_eps[i + 1] - v_x[i])) - ((v_y[i] - help_v[i]) / (v_x[i] - v_eps[i])));
    }
}

double spline_t::get_value (double x)
{
  int i = static_cast<int> ((m_cur_params.number_points) * (x - v_eps[0]) / (v_eps[m_cur_params.number_points] - v_eps[0]));
  if (i > m_cur_params.number_points)
    i = m_cur_params.number_points;
  return v_ans_1[i] + v_ans_2[i] * (x - v_eps[i]) + v_ans_3[i] * (x - v_eps[i]) * (x - v_eps[i]);
}


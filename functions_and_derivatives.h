#pragma once

#include <functional>

constexpr double default_left_a = -10;
constexpr double default_right_b = 10;
constexpr double default_namber_points = 10;

constexpr double eps = 1e-15;

enum class function_type
{
  constant,
  linear,
  quadric,
  cubic,
  biquadr,
  exponent,
  fraction,

  COUNT
};

const char *enum_to_string (function_type type);
void set_next (function_type &cur_func);
std::function<double (double)> get_func_by_type (function_type type);
std::function<double (double)> get_second_deriv_by_type (function_type type);
std::function<double (double)> get_first_deriv_by_type (function_type type);
std::pair<double, double> get_function_bounds_by_type (function_type type);

struct common_params_t
{
  double left_bound = default_left_a;
  double right_bound = default_right_b;
  int number_points = default_namber_points;
  function_type func_type = function_type::COUNT;
};

class precision_t
{
  bool dirty = true;
  double value = 0;

  int m_coefficient = 0;

  common_params_t &m_params;

public:
  precision_t (common_params_t &params);
  ~precision_t ();

  void set_dirty ();
  void reduce_coefficient ();
  void raise_coefficient ();
  double get_value ();
};

std::pair<double, double> minmax (double from, double to, std::function<double (double)> func, int number_points);
bool math_equal (double x, double y);

double f_constant (double x);
double f_linear (double x);
double f_quadric (double x);
double f_cubic (double x);
double f_biquadr (double x);
double f_exponent (double x);
double f_fraction (double x);

double dd_constant (double x);
double dd_linear (double x);
double dd_quadric (double x);
double dd_cubic (double x);
double dd_biquadr (double x);
double dd_exponent (double x);
double dd_fraction (double x);

double d_constant (double x);
double d_linear (double x);
double d_quadric (double x);
double d_cubic (double x);
double d_biquadr (double x);
double d_exponent (double x);
double d_fraction (double x);

#include "functions_and_derivatives.h"

#include <cmath>

const char *enum_to_string (function_type type)
{
  switch (type)
    {
      case function_type::constant:
        return "f (x) = 1";
      case function_type::linear:
        return "f (x) = x";
      case function_type::quadric:
        return "f (x) = x^2";
      case function_type::cubic:
        return "f (x) = x^3";
      case function_type::biquadr:
        return "f (x) = x^4";
      case function_type::exponent:
        return "f (x) = exp (x)";
      case function_type::fraction:
        return "f (x) = 1 / (25x^2 + 1)";
      case function_type::COUNT:
        return "Invalid function type";
    }
  return "";
}

std::function<double (double)> get_func_by_type (function_type type)
{
  switch (type)
    {
      case function_type::constant:
        return f_constant;
      case function_type::linear:
        return f_linear;
      case function_type::quadric:
        return f_quadric;
      case function_type::cubic:
        return f_cubic;
      case function_type::biquadr:
        return f_biquadr;
      case function_type::exponent:
        return f_exponent;
      case function_type::fraction:
        return f_fraction;
      case function_type::COUNT:
        return {};
    }
  return {};
}

std::function<double (double)> get_second_deriv_by_type (function_type type)
{
  switch (type)
    {
      case function_type::constant:
        return dd_constant;
      case function_type::linear:
        return dd_linear;
      case function_type::quadric:
        return dd_quadric;
      case function_type::cubic:
        return dd_cubic;
      case function_type::biquadr:
        return dd_biquadr;
      case function_type::exponent:
        return dd_exponent;
      case function_type::fraction:
        return dd_fraction;
      case function_type::COUNT:
        return {};
    }
  return {};
}

std::function<double (double)> get_first_deriv_by_type (function_type type)
{
  switch (type)
    {
      case function_type::constant:
        return d_constant;
      case function_type::linear:
        return d_linear;
      case function_type::quadric:
        return d_quadric;
      case function_type::cubic:
        return d_cubic;
      case function_type::biquadr:
        return d_biquadr;
      case function_type::exponent:
        return d_exponent;
      case function_type::fraction:
        return d_fraction;
      case function_type::COUNT:
        return {};
    }
  return {};
}

std::pair<double, double> get_function_bounds_by_type (function_type type)
{
  switch (type)
    {
      case function_type::constant:
      case function_type::linear:
        return {-1e306, 1e306};
      case function_type::quadric:
        return {-1e153, 1e153};
      case function_type::cubic:
        return {-1e100, 1e100};
      case function_type::biquadr:
        return {-1e75, 1e75};
      case function_type::exponent:
        return {-500, 500};
      case function_type::fraction:
        return {-1e50, 1e50};
      case function_type::COUNT:
        break;
    }

  return {0, 0};
}

void set_next (function_type &cur_func)
{
  int next_func = (static_cast<int> (cur_func) + 1) % static_cast<int> (function_type::COUNT);
  cur_func = static_cast<function_type> (next_func);
}

precision_t::precision_t (common_params_t &params) : m_params (params) {}

precision_t::~precision_t () = default;

void precision_t::set_dirty () { dirty = true; }

void precision_t::reduce_coefficient ()
{
  m_coefficient--;
  set_dirty ();
}

void precision_t::raise_coefficient ()
{
  m_coefficient++;
  set_dirty ();
}

double precision_t::get_value ()
{
  if (dirty)
    {
      value = minmax (m_params.left_bound, m_params.right_bound, get_func_by_type (m_params.func_type), m_params.number_points).second * 0.1
              * m_coefficient;
      dirty = false;
    }
  return value;
}

std::pair<double, double> minmax (double from, double to, std::function<double (double)> func, int number_points)
{
  if (from > to)
    {
      return {-1, 1};
    }

  double step = (to - from) / number_points;
  double min = func (from), max = func (from);

  for (double cur = from + step; to - cur > 1e-6; cur += step)
    {
      double value = func (cur);
      if (value < min)
        min = value;
      if (value > max)
        max = value;
    }
  return {min, max};
}

bool math_equal (double x, double y) { return fabs (x - y) < eps * (fabs (x) > fabs (y) ? fabs (x) : fabs (y)); }

double f_constant (double /*x*/) { return 1; }
double f_linear (double x) { return x; }
double f_quadric (double x) { return x * x; }
double f_cubic (double x) { return x * x * x; }
double f_biquadr (double x) { return x * x * x * x; }
double f_exponent (double x) { return exp (x); }
double f_fraction (double x) { return 1.0 / (25 * x * x + 1.); }

 double dd_constant (double /*x*/) { return 0; }
 double dd_linear (double /*x*/) { return 0; }
 double dd_quadric (double /*x*/) { return 2; }
 double dd_cubic (double x) { return 6 * x; }
 double dd_biquadr (double x) { return 12 * x * x; }
 double dd_exponent (double x) { return exp (x); }
 double dd_fraction (double x) { return (3750. * x * x - 50.) / ((25. * x * x + 1.) * (25. * x * x + 1.) * (25. * x * x + 1.)); }

 double d_constant (double /*x*/) { return 0; }
 double d_linear (double /*x*/) { return 1; }
 double d_quadric (double x) { return 2 * x; }
 double d_cubic (double x) { return 3 * x * x; }
 double d_biquadr (double x) { return 4 * x * x * x; }
 double d_exponent (double x) { return exp (x); }
 double d_fraction (double x) { return -50 * x / ((25 * x * x + 1) * (25 * x * x + 1)); }

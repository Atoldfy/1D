#pragma once

#include "functions_and_derivatives.h"
#include <vector>

#include <cmath>

constexpr double newton_limits_max = 40;
constexpr double newton_limits_min = 2;

constexpr double spline_limits_max = 10000000000000;
constexpr double spline_limits_min = 3;

class calculator_t
{
  friend class residual_t;

public:
  calculator_t (common_params_t init_params, precision_t &precision);
  virtual ~calculator_t () = default;

  std::function<double (double)> value_getter ();
  virtual void update (common_params_t new_params);

  virtual int get_color ();
  virtual int get_line_style ();

protected:
  common_params_t m_cur_params;
  precision_t &m_precision;

  virtual void resize_all ();
  virtual void resolve ();

  virtual double get_value (double x);
};


class newton_t : public calculator_t
{
private:
  std::vector<double> x_v;
  std::vector<double> y_v;

   void resize_all () override;
   void resolve () override;

public:
  newton_t (common_params_t init_params, precision_t &precision);
  ~newton_t () = default;

  int get_color () override;
  int get_line_style () override;
  double get_value (double x) override;
};

class spline_t : public calculator_t
{
private:
  std::vector<double> v_x;
  std::vector<double> v_y;

  std::vector<double> v_eps;
  std::vector<double> v_rside;
  std::vector<double> v_left;
  std::vector<double> v_diagonal;
  std::vector<double> v_right;
  std::vector<double> v_ans_1;
  std::vector<double> v_ans_2;
  std::vector<double> v_ans_3;
  std::vector<double> help_v;

   void resize_all () override;
   void resolve () override;

public:
  spline_t (common_params_t init_params, precision_t &precision);
  ~spline_t () = default;

  int get_color () override;
  int get_line_style () override;
  double get_value (double x) override;
};

class residual_t : public calculator_t
{
  calculator_t *m_calculator;
public:
  residual_t (common_params_t init_params, precision_t &precision, calculator_t *calculator) : calculator_t (init_params, precision)
  {
    m_calculator = calculator;
  }
  ~residual_t () = default;

private:
  void update (common_params_t new_params) override
  {
    if (m_cur_params.func_type == new_params.func_type && m_cur_params.number_points == new_params.number_points
        && math_equal (m_cur_params.left_bound, new_params.left_bound) && !math_equal (m_cur_params.right_bound, new_params.right_bound))
      return;

    m_cur_params = new_params;
    if (m_calculator)
      m_calculator->m_cur_params = new_params;

    resize_all ();
    resolve ();
  }
  void resize_all () override { m_calculator->resize_all (); }
  void resolve () override { m_calculator->resolve (); }

protected:
  double get_value (double x) override { return fabs (m_calculator->get_value (x) - get_func_by_type (m_cur_params.func_type) (x)); }
  int get_color () override { return m_calculator->get_color () + 6; }
  int get_line_style () override { return 3; }
};

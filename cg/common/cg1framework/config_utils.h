

#ifndef INCLUDED_CONFIG_UTILS
#define INCLUDED_CONFIG_UTILS

#pragma once

#include <stdexcept>
#include <array>
#include <vector>
#include <fstream>
#include <iostream>

#include "cfg_config.h"
#include "cfg_container.h"
#include "vector.h"
#include "matrix.h"

template <typename Type>
inline Type getAttribute(const CgcvConfig::Testcase& testcase, const std::string& name, Type default_value)
{
  try
  {
    return testcase.getAttribute<Type>(name);
  }
  catch (const CgcvConfig::ConfigError&)
  {
  }

  return default_value;
}

template <typename Type>
inline Type getAttribute(const CgcvConfig::Container& container, const std::string& name, Type default_value)
{
  try
  {
    return container.getAttribute<Type>(name);
  }
  catch (const CgcvConfig::ConfigError&)
  {
  }

  return default_value;
}

template <typename T, unsigned int N>
math::vector<T, N> make_vector(const T (&v)[N]);

template <typename T>
inline math::vector<T, 2> make_vector(const T (&v)[2])
{
  return math::vector<T, 2>(v[0], v[1]);
}

template <typename T>
inline math::vector<T, 3> make_vector(const T (&v)[3])
{
  return math::vector<T, 3>(v[0], v[1], v[2]);
}

template <typename T, unsigned int N>
math::vector<T, N> make_vector(const std::array<T, N>& v);

template <typename T>
inline math::vector<T, 2> make_vector(const std::array<T, 2>& v)
{
  return math::vector<T, 2>(v[0], v[1]);
}

template <typename T>
inline math::vector<T, 3> make_vector(const std::array<T, 3>& v)
{
  return math::vector<T, 3>(v[0], v[1], v[2]);
}

template <typename T>
inline math::matrix<T, 2, 2> make_matrix(const std::array<std::array<T, 2>, 2>& v)
{
  return math::matrix<T, 3, 4>(v[0][0], v[0][1], v[1][0], v[1][1]);
}

template <typename T>
inline math::matrix<T, 3, 3> make_matrix(const std::array<std::array<T, 3>, 3>& v)
{
  return math::matrix<T, 3, 4>(v[0][0], v[0][1], v[0][2], v[1][0], v[1][1], v[1][2], v[2][0], v[2][1], v[2][2]);
}

template <typename T>
inline math::matrix<T, 3, 4> make_matrix(const std::array<std::array<T, 4>, 3>& v)
{
  return math::matrix<T, 3, 4>(v[0][0], v[0][1], v[0][2], v[0][3], v[1][0], v[1][1], v[1][2], v[1][3], v[2][0], v[2][1], v[2][2], v[2][3]);
}

template <typename T>
inline math::matrix<T, 4, 4> make_matrix(const std::array<std::array<T, 4>, 4>& v)
{
  return math::matrix<T, 4, 4>(v[0][0], v[0][1], v[0][2], v[0][3], v[1][0], v[1][1], v[1][2], v[1][3], v[2][0], v[2][1], v[2][2], v[2][3], v[3][0], v[3][1], v[3][2], v[3][3]);
}

template <typename T, unsigned int N>
inline math::vector<T, N> getVector(const CgcvConfig::Container& container, const std::string& name, const math::vector<T, N>& default_value)
{
  try
  {
    std::array<T, N> v;
    container.getVector(v, name);
    return make_vector(v);
  }
  catch (const CgcvConfig::ConfigError&)
  {
  }

  return default_value;
}

template <typename T, unsigned int M, unsigned int N>
inline math::matrix<T, M, N> getMatrix(const CgcvConfig::Container& container, const std::string& name, const math::matrix<T, M, N>& default_value)
{
  try
  {
    std::array<std::array<T, N>, M> m;
    container.getMatrix(m, name);
    return make_matrix(m);
  }
  catch (const CgcvConfig::ConfigError&)
  {
  }

  return default_value;
}

#endif // INCLUDED_CONFIG_UTILS

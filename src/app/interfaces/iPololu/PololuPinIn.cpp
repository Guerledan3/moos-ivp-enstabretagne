/************************************************************/
/*    FILE: PololuPinIn.cpp
/*    ORGN: ENSTA Bretagne Robotics - moos-ivp-enstabretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#include "PololuPinIn.h"

PololuPinIn::PololuPinIn(int pin_number) : PololuPin(pin_number)
{
  m_value = 0.;
  m_coeff = 1.;
  m_threshold = 0.;
  m_test_threshold = false;
}

double PololuPinIn::getValue()
{
  return m_value;
}

double PololuPinIn::getCoeff()
{
  return m_coeff;
}

double PololuPinIn::getThreshold()
{
  return m_threshold;
}

std::string PololuPinIn::getUnit()
{
  return m_unit;
}

std::string PololuPinIn::getWarningMessage()
{
  return m_warning_message;
}

bool PololuPinIn::testThresholdRequired()
{
  return m_test_threshold;
}

void PololuPinIn::setValue(double value)
{
  m_value = value * m_coeff;
}

void PololuPinIn::setCoeff(double coeff)
{
  m_coeff = coeff;
}

void PololuPinIn::setThreshold(double threshold)
{
  m_threshold = threshold;
}

void PololuPinIn::setUnit(std::string unit)
{
  m_unit = unit;
}

void PololuPinIn::setWarningMessage(std::string message)
{
  m_warning_message = message;
}

void PololuPinIn::requireTestThreshold(bool test)
{
  m_test_threshold = test;
}

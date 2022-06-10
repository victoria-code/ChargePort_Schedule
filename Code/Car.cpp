#include "Car.h"

Car::Car(std::string uname = "", std::string Cname = "", double BCap = 0.0, double BNow = 0.0)
{
    usrname = uname;
    CarName = Cname;
    BatteryCap = BCap;
    BatteryNow = BNow;
}
Car::~Car()
{
    if (this->Ask != nullptr)
        delete this->Ask;
    if (this->Reply != nullptr)
        delete this->Reply;
}

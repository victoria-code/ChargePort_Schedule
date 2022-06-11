#include "Car.h"

Car::Car(std::string uname, std::string Cname, double BCap, double BNow)
{
    usrname = uname;
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

#include "Car.h"

Car::Car(std::string uname = "", int CID = 0, std::string Cname = "", double BCap = 0.0, double BNow = 0.0)
{
    usrname = uname;
    CarID = CID;
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

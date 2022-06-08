#include "Car.h"
Car::~Car()
{
    if (this->Ask != nullptr)
        delete this->Ask;
    if (this->Reply != nullptr)
        delete this->Reply;
}
void Car::setBatteryCap(double cap)
{
    this->BatteryCap = cap;
}
void Car::setAsk(CarAsk *ask)
{
    this->Ask = ask;
}
void Car::setReply(CarReply *reply)
{
    this->Reply = reply;
}
void Car::setChargeInfo(ChargeInfo *info)
{
    this->info = info;
}

// getters
double Car::getBatteryCap()
{
    return this->BatteryCap;
}
CarAsk *Car::getCarAsk()
{
    return this->Ask;
}
CarReply *Car::getCarReply()
{
    return this->Reply;
}
ChargeInfo *Car::getChargeInfo()
{
    return this->info;
}
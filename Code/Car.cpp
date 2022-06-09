#include "Car.h"

Car::~Car()
{
    if (this->Ask != nullptr)
        delete this->Ask;
    if (this->Reply != nullptr)
        delete this->Reply;
}

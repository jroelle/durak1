#pragma once
#include "Hand.h"

class Actor
{
public:
	virtual ~Actor() = default;

private:
	Hand _hand;
};
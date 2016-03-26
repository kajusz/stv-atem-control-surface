#pragma once

typedef unsigned int atemId; // this is quicker to write

#include "qatemconnection.h"

#include <memory>
typedef std::shared_ptr<QAtemConnection> atemPtr;

#include <vector>
typedef std::vector<std::shared_ptr<QAtemConnection>> atemVec;

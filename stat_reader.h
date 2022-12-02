#pragma once

#include <string>
#include <iostream>

#include "transport_catalogue.h"

namespace catalogue{
namespace detail{
void GetInfo(Transport& transport, std::string_view bus, std::ostream& out = std::cout);
}
}

#include "xexception.h"

xException::xException(const char Message[], int Error)
{
    sprintf(this->Message, "%s", Message);
    this->Error = Error;
}

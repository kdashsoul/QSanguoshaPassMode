#ifndef PASSGENERAL_H
#define PASSGENERAL_H

#include <general.h>

class PassGeneral: public General
{
public:
    PassGeneral(Package *package,const General *general);
};

#endif // PASSGENERAL_H

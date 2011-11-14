#include "pass-general.h"
#include "general.h"
#include "engine.h"

PassGeneral::PassGeneral(Package *package,const General *general)
    :General(package, general->objectName() + "_p", general->getKingdom(),  general->getMaxHp(), general->isMale(), general->isHidden())
{

}

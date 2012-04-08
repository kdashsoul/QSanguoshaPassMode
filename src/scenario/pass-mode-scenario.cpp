#include "pass-mode-scenario.h"

#include <QFile>
#include <QFileDialog>
#include <QByteArray>
#include <QMessageBox>
#include <QTime>

SaveDataStruct::SaveDataStruct()
    :read_success(false)
{
}

SkillAttrStruct::SkillAttrStruct()
    :limit_times(0)
{
}
int SkillAttrStruct::getValue(const int level) const{
    if(values.length() >= level)
        return values.at(level - 1) ;
    else
        return 0 ;
}

int SkillAttrStruct::getLimitTimes() const{
    if(! ServerInfo.GameMode.contains("_pass_"))
        return 0 ;
    return limit_times ;
}

const QString PassMode::version = "ver2.0";
const QString PassMode::savePath = "savedata/pass_mode.sav";
const int PassMode::maxStage = 2 ;

PassModeRule::PassModeRule(Scenario *scenario)
    :ScenarioRule(scenario)
{
    events << GameStart << DrawNCards << Predamaged << CardUsed;
    stage = scenario->objectName().remove("_pass_").toInt() ;
}

void PassModeRule::assign(QStringList &generals, QStringList &roles) const{

    for(int i=0;i<players.length();i++){
        QMap<QString,QString> sp =players.at(i);
        QString name = sp["general"];
        QString role = "rebel";
        if(i == 0){
            name = "sujiang" ;
            role = "lord";
        }
        generals << name;
        roles << role;
    }
}

QStringList PassModeRule::existedGenerals() const{
    QStringList names;
    for(int i=0;i<players.length();i++){
        QMap<QString,QString> sp =players.at(i);
        QString name = sp["general"];
        if(i == 0){
            name = "sujiang";
        }
        names << name;
    }
    return names;
}

void PassModeRule::loadStage(QString path){
    QFile file(path);
    if(file.open(QIODevice::ReadOnly)){
        players.clear();

        QTextStream stream(&file);
        while(!stream.atEnd()){
            QString aline = stream.readLine();
            if(aline.isEmpty()) continue;
            addNPC(aline);
        }
        file.close();
    }
}

void PassModeRule::addNPC(QString feature){
    QMap<QString, QString> player;
    QStringList features;
    if(feature.contains("|"))features= feature.split("|");
    else features = feature.split(" ");
    foreach(QString str, features){
        QStringList keys = str.split(":");
        if(keys.size() < 2) continue;
        if(keys.first().size() < 1) continue;
        player.insert(keys.at(0),keys.at(1));
    }
    players << player;
}

bool PassModeRule::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    Room *room = player->getRoom() ;
    switch(event){
    case GameStart:{
        if(room->getTag("Stage").isNull() && player->isLord()){
            SaveDataStruct *save_data = PassMode::getSaveData();
            room->setTag("Stage", stage);
            QString general_name ;
            if(stage == 1){
                PassMode::removeSaveData();
                room->setTag("Times", 1);
                room->setTag("LoadTimes", 0);
                general_name = room->askForGeneralPass(player,"standard");
            }else{
                general_name = save_data->general_name;
            }
            room->transfigure(player, general_name, true, true);
            const General *general = Sanguosha->getGeneral(general_name) ;
            room->setPlayerProperty(player, "kingdom", general->getKingdom());
            if(stage == 1 ){
                player->gainMark("@exp",50);
            }else{
                PassMode::loadData(room,save_data);
            }
            room->askForSkillLearn(player);

            int n = 0;
            int hero_draw = 6 ;
            int enemy_draw = 4 ;
            foreach(ServerPlayer *player, room->getPlayers()){
                n = player->isLord() ? hero_draw : enemy_draw;
                player->drawCards(player->hasAbility("startdraw") ? n + 1 : n);
            }
        }
        break ;
    }case DrawNCards:{
        if(player->hasAbility("turndraw")){
            data = data.toInt() + 1 ;
        }
        if(player->getKingdom() == "evil")
            data = data.toInt() - 1 ;
        else if(player->getKingdom() == "god")
            data = data.toInt() + 1 ;
        break;
    }
    case Predamaged:{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Lightning") && damage.damage == 3){
            damage.damage--;
            data = QVariant::fromValue(damage);
        }
        break;
    }
    case CardUsed:{
        break;
    }
    default:
        break;
    }
    return false;
}

PassMode::PassMode(QObject *parent)
    :GameRule(parent)
{
    setObjectName("pass_mode_rule");
}

static int Restart = 1;

bool PassMode::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    Room *room = player->getRoom();
    ServerPlayer *lord = room->getLord();
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    switch(event){
    case GameStart:{
//        if(setjmp(env) == Restart){
//            initNextStageStart(player);
//            return false;
//        }

        return false ;
    }
    case TurnStart :{
        if(player->isLord()){
            int turns = room->getTag("Turns").toInt() ;
            room->setTag("Turns",++turns);
        }
        break;
    }
    case Death:{
        if(player->isLord()){
            if(! room->getAlivePlayers().empty())
                room->gameOver("rebel");
        }else{
            if(lord->hasAbility("killdraw")){
                lord->drawCards(1);
            }

            DamageStar damage = data.value<DamageStar>();
            if((!damage || damage->from != lord))
                lord->drawCards(3);

            int exp = PassMode::getExpMap().value(player->getKingdom(), 0);
            exp = qrand()%(exp + 1) + exp/2;
            if(lord->hasAbility("exp")){
                exp += exp / 2 ;
            }

            int orgin_exp = lord->getMark("@exp") ;
            lord->gainMark("@exp",exp);
            LogMessage log;
            log.type = "#GainExp";
            log.from = lord;
            log.to << player;
            log.arg = QString::number(exp);
            log.arg2 = QString::number(orgin_exp+exp);
            room->sendLog(log);

            if(room->getAlivePlayers().length() == 1){
                if(!goToNextStage(room))
                    break;

                setNextStageInfo(room);

                longjmp(env, Restart);
                return true;
            }
        }
    }

    default:
        break;
    }
    return GameRule::trigger(event, player, data);
}

QStringList PassMode::getNeedSaveRoomTagName(){
    static QStringList tagNames;
    if(tagNames.isEmpty())
        tagNames << "Stage" << "Times" << "Turns" << "LoadTimes" ;

    return tagNames;
}

QStringList PassMode::getNeedSavePlayerMarkName(){
    static QStringList markNames;
    if(markNames.isEmpty())
        markNames << "@exp" ;

    return markNames;
}

int PassMode::getStage() const{
    return ServerInfo.GameMode.remove("_pass_").toInt() ;
}

void PassMode::stageStartDraw(Room *room) const{
    int n = 0;
    int hero_draw = 6 ;
    int enemy_draw = 4 ;
    foreach(ServerPlayer *player, room->getPlayers()){
        n = player->isLord() ? hero_draw : enemy_draw;
        player->drawCards(player->hasAbility("startdraw") ? n + 1 : n);
    }
}

bool PassMode::loadData(Room *room, SaveDataStruct *save_data){
    ServerPlayer *lord = room->getLord();
    if(! save_data)
        save_data = getSaveData();
    if(!save_data->read_success)
        return false;

    foreach (QString tag_name, save_data->roomTags.keys()) {
        room->setTag(tag_name,save_data->roomTags.value(tag_name));
    }
    int i = 0 ;
    foreach (QString mark_name, getNeedSavePlayerMarkName()) {
        room->setPlayerMark(lord,mark_name,save_data->playerMarks.at(i).toInt());
        i ++ ;
    }

    foreach(QString skill, save_data->skills)
        room->acquireSkill(lord, skill, true , false);
    foreach(QString enhance, save_data->skills_enhance)
        room->setPlayerSkillEnHance(lord,enhance);
    foreach(QString ability, save_data->abilities)
        room->setPlayerAbility(lord , ability);

    LogMessage log;
    log.type = "#LoadNextStage";
    log.from = room->getLord();
    log.arg = room->getTag("Stage").toString();
    log.arg2 = room->getTag("Times").toString();
    room->sendLog(log);
    return true;
}

bool PassMode::removeSaveData(){
    QFile file(savePath);
    if(file.exists()){
        file.remove();
    }
    return true;
}

void PassMode::initNextStageStart(ServerPlayer *player) const{
    Room *room = player->getRoom();
    room->setTag("SwapPile", 0);
    room->setPlayerProperty(player, "phase", "not_active");

    room->setCurrent(room->getLord());
    stageStartDraw(room);
    room->getLord()->play();
}

void PassMode::initGameStart(ServerPlayer *player) const{
    Room *room = player->getRoom();
    foreach(ServerPlayer *p, room->getPlayers()){
        if(p->isLord()){
            if(getStage() == 1){
                room->setTag("Stage", 1);
                room->setTag("Times", 1);
                room->setTag("LoadTimes", 0);

                QString original = p->getGeneralName() ;
                QString name = room->askForGeneralPass(p,"standard");
                QString trans = QString("%1:%2").arg(original).arg(name);
                p->invoke("transfigure",trans);
                room->setPlayerProperty(p,"general",name);
            }
//            room->transfigure(p, name, true, true);
//            room->setPlayerProperty(p, "maxhp", p->getMaxHP() + 1);
//            room->setPlayerProperty(p, "hp", p->getMaxHP());
//            const General *general = Sanguosha->getGeneral(name);
//            if(p->getKingdom() != general->getKingdom())
//                room->setPlayerProperty(p, "kingdom", general->getKingdom());

            p->gainMark("@exp",50);
            room->askForSkillLearn(p);
        }else{
//            QStringList enemys = PassMode::getStageList().at(0).split(QRegExp("\\s+"));
//            QString enemy_name = enemys.at(p->getSeat()-2) ;
//            const General *general = Sanguosha->getGeneral(enemy_name + "_e") ;
//            if(! general)
//                general = Sanguosha->getGeneral(enemy_name);
//            room->transfigure(p, general->objectName(), true, true);
//            if(p->getKingdom() != general->getKingdom())
//                room->setPlayerProperty(p, "kingdom", general->getKingdom());
        }
//        stageStartDraw(room, p);
    }
}


bool PassMode::goToNextStage(Room *room) const{
    int stage = room->getTag("Stage").toInt();
    if(stage >= getStageList().length()){
        room->gameOver("lord");
        return false;
    }

    askForSaveData(room);
    return true;
}

SaveDataStruct* PassMode::catchSaveInfo(Room *room){
    ServerPlayer *lord = room->getLord();
    QStringList lord_skills;
    QSet<QString> skill_names = lord->getAcquiredSkills();
    foreach(QString skill_name, skill_names){
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if(skill->inherits("WeaponSkill") || skill->inherits("ArmorSkill")
                || skill_name == "axe" || skill_name == "spear")
            continue;
        if(skill->isVisible())
            lord_skills << skill_name;
    }
    QStringList lord_skill_enhance ;
    QSet<QString> skill_enhance = lord->getSkillEnhance();
    foreach(QString enhance_name, skill_enhance){
        lord_skill_enhance << enhance_name;
    }
    QStringList lord_abilities ;
    QHash<QString,int> abilities = lord->getAbilities();
    foreach(QString ability, abilities.keys()){
        lord_abilities << ability;
    }

    SaveDataStruct *save_cache = new SaveDataStruct;
    foreach(const QString tag_name, getNeedSaveRoomTagName()){
        save_cache->roomTags.insert(tag_name,QVariant::fromValue(room->getTag(tag_name))) ;
    }
    foreach(const QString mark_name, getNeedSavePlayerMarkName()){
        save_cache->playerMarks << QString::number(lord->getMark(mark_name)) ;
    }

    save_cache->general_name      = lord->getGeneralName();
    save_cache->skills            = lord_skills;
    save_cache->skills_enhance    = lord_skill_enhance ;
    save_cache->abilities         = lord_abilities;

    return save_cache;
}

bool PassMode::askForSaveData(Room *room){
//    if(room->askForChoice(room->getLord(), "savefile", "save+notsave") == "notsave")
//        return false;

    SaveDataStar save = catchSaveInfo(room);

    QByteArray data;
    QString room_tags_str ;
    foreach (QString key_name, save->roomTags.keys()) {
        room_tags_str.append(QString("%1:%2").arg(key_name).arg(save->roomTags.value(key_name).toString())) ;
        room_tags_str.append("|");
    }
    data.append(room_tags_str.toUtf8().toBase64());
    data.append(QString("\n"));
    data.append(save->playerMarks.join("|").toUtf8().toBase64());
    data.append(QString("\n"));
    data.append(save->general_name.toUtf8().toBase64());
    data.append(QString("\n"));
    data.append(save->skills.join("|").toUtf8().toBase64());
    data.append(QString("\n"));
    data.append(save->skills_enhance.join("|").toUtf8().toBase64());
    data.append(QString("\n"));
    data.append(save->abilities.join("|").toUtf8().toBase64());
    data.append(QString("\n"));
    data.append(version.toUtf8().toBase64());

    QStringList paths = savePath.split("/");
    if(!QDir(paths[0]).exists()){
        QDir().mkdir(paths[0]) ;
    }
    QFile file(savePath);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return file.write(data) != -1;
    else
        return false;
}

void PassMode::setNextStageInfo(Room *room, bool save_loaded) const{
    int stage = room->getTag("Stage").toInt() ;
    room->setTag("Stage", ++stage);
    ServerPlayer *lord = room->getLord();

    room->askForSkillLearn(lord);

    if(!save_loaded){
        resetPlayer(lord);

        LogMessage log;
        log.type = "#NextStage";
        log.from = lord;
        log.arg = QString::number(stage);
        room->sendLog(log);
    }

    int i = 0;
    QStringList enemys = getStageList().at(stage - 1).split(QRegExp("\\s+"));
    foreach(ServerPlayer *player, room->getPlayers()){
        if(!player->isLord()){
            QString enemy_name = enemys.at(i) ;
            const General *general = Sanguosha->getGeneral(enemy_name + "_e") ;
            if(! general)
                general = Sanguosha->getGeneral(enemy_name);
            room->transfigure(player, general->objectName(), true, true);
            if(player->getKingdom() != general->getKingdom())
                room->setPlayerProperty(player, "kingdom", general->getKingdom());
            resetPlayer(player);
            i ++ ;
        }
    }
}

void PassMode::resetPlayer(ServerPlayer *player) const{
    Room *room = player->getRoom();
    if(player->isDead())
        room->revivePlayer(player);
    player->setAlive(false);
    player->throwAllCards();
    player->setAlive(true);
    player->clearFlags();
    player->clearHistory();
    if(!player->faceUp())
        player->turnOver();
    if(player->isChained())
        room->setPlayerProperty(player, "chained", false);
    if(player->isLord()){
        QMap<QString,int> tmp;
        foreach (QString mark_name, PassMode::getNeedSavePlayerMarkName()) {
            tmp.insert(mark_name,player->getMark(mark_name));
        }
        player->throwAllMarks();
        foreach (QString mark_name, PassMode::getNeedSavePlayerMarkName()) {
            room->setPlayerMark(player,mark_name,tmp.value(mark_name, 0));
        }
    }else{
        player->throwAllMarks();
    }

    room->setPlayerProperty(player, "hp", player->getMaxHP());
}


SaveDataStruct *PassMode::getSaveData(){
    SaveDataStruct *save = new SaveDataStruct;

    int line_num = 1;
    QFile file(savePath);
    if(file.open(QIODevice::ReadOnly)){
        QTextStream stream(&file);
        while(!stream.atEnd()){
            QString line = stream.readLine();
            line = QString::fromUtf8(QByteArray::fromBase64(line.toAscii()));

            switch(line_num){
            case 1:{
                foreach(QString kv,line.split("|")){
                    QStringList kv_list = kv.split(":") ;
                    if(kv_list.length() != 2)
                        continue ;
                    save->roomTags.insert(kv_list.at(0),kv_list.at(1));
                }
                break;
            }
            case 2:{
                save->playerMarks = line.split("|");
                break;
            }
            case 3:{
                save->general_name = line ;
                break;
            }
            case 4:{
                save->skills = line.split("|") ;
                break;
            }
            case 5:{
                save->skills_enhance = line.split("|") ;
                break;
            }
            case 6:{
                save->abilities = line.split("|") ;
                break;
            }
            default:
                break;
            }
            line_num++;
        }
        file.close();
        save->read_success = true;
    }
    return save;
}


QMap<QString, int> PassMode::getExpMap() {
    static QMap<QString, int> exp_map;
    if(exp_map.isEmpty()){
        QRegExp rx("(\\w+)\\s+(\\d+)");
        QFile file("etc/passmode/exps.txt");
        if(file.open(QIODevice::ReadOnly)){
            QTextStream stream(&file);
            while(!stream.atEnd()){
                QString line = stream.readLine();
                if(!rx.exactMatch(line))
                    continue;
                QStringList texts = rx.capturedTexts();
                QString name = texts.at(1);
                QString exp = texts.at(2);
                exp_map.insert(name,exp.toInt());
            }
            file.close();
        }
    }
    return exp_map;
}

QStringList PassMode::getStageList() {
    static QStringList stage_list;
    if(stage_list.isEmpty()){
        QRegExp rx("\\w+\\s+\\w+\\s+\\w+");
        QFile file("etc/passmode/stages.txt");
        if(file.open(QIODevice::ReadOnly)){
            QTextStream stream(&file);
            while(!stream.atEnd()){
                QString line = stream.readLine();
                if(!rx.exactMatch(line))
                    continue;
                stage_list << line ;
            }
            file.close();
        }
    }
    return stage_list;
}

QMap<QString, SkillAttrStruct*> PassMode::getSkillMap(){
    static QMap<QString, SkillAttrStruct*> skill_map;
    if(skill_map.isEmpty()){
        QRegExp rx("(\\w+)\\s+([\\d|:]+)");
        QFile file("etc/passmode/skills.txt");
        if(file.open(QIODevice::ReadOnly)){
            QTextStream stream(&file);
            while(!stream.atEnd()){
                QString line = stream.readLine();
                if(!rx.exactMatch(line))
                    continue;
                QStringList texts = rx.capturedTexts();
                QString skill_name = texts.at(1);
                QStringList skill_attr_str = texts.at(2).split(":");
                QString skill_value = skill_attr_str.at(0) ;
                SkillAttrStruct *skill_attr = new SkillAttrStruct;;
                if(skill_attr_str.length() > 1){
                    skill_attr->limit_times = skill_attr_str.at(1).toInt() ;
                }
                foreach(QString value , skill_value.split("|")){
                    skill_attr->values << value.toInt() ;
                }
                skill_map.insert(skill_name, skill_attr) ;
            }
            file.close();
        }
    }
    return skill_map;
}


QMap<QString, QStringList> PassMode::getGeneralMap(){
    static QMap<QString, QStringList> general_map;
    QRegExp rx("(\\w+)\\s+([\\w|]+)");
    QFile file("etc/passmode/generals.txt");
    if(file.open(QIODevice::ReadOnly)){
        QTextStream stream(&file);
        while(!stream.atEnd()){
            QString line = stream.readLine();
            if(!rx.exactMatch(line))
                continue;
            QStringList texts = rx.capturedTexts();
            QString general_name = texts.at(1);
            QString skills = texts.at(2);
            general_map.insert(general_name,skills.split("|")) ;
        }
        file.close();
    }
    return general_map;
}

void PassModeScenario::onTagSet(Room *room, const QString &key) const{
    // dummy
}

PassModeScenario::PassModeScenario(const QString &name)
    :Scenario(QString("_pass_%1").arg(name))
{
    rule = new PassModeRule(this);
    setupStage(name);
}

void PassModeScenario::setupStage(QString name) const{
    name.prepend("etc/passmode/");
    name.append(".txt");

    PassModeRule* pass_rule = qobject_cast<PassModeRule*>(this->getRule());
    pass_rule->loadStage(name);
}

#define ADD_PASS_SCENARIO(name) static ScenarioAdder PassModeScenario##name##ScenarioAdder(QString("PassMode_") + #name, new PassModeScenario(#name));

ADD_PASS_SCENARIO(01)
ADD_PASS_SCENARIO(02)

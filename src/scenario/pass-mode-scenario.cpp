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

PassModeRule::PassModeRule(Scenario *scenario)
    :ScenarioRule(scenario)
{
    events << GameStart << TurnStart << DrawNCards << Predamaged << CardUsed << Death ;
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

void PassModeRule::initGameStart(Room *room) const {
    foreach (ServerPlayer *player, room->getAllPlayers()) {
        QMap<QString, QString> props = getPlayerProps(player) ;

        QString str = props["maxhp"];
        if(str == NULL)
            str=QString::number(player->getGeneralMaxHP());
        if(! player->isLord()){
            room->setPlayerProperty(player,"maxhp",str.toInt());
        }
        str = props["hpadj"];
        if(str != NULL)
            room->setPlayerProperty(player,"maxhp",player->getMaxHP()+str.toInt());
        str = QString::number(player->getMaxHP());

        QString str2 = props["hp"] ;
        if(str2 != NULL)
            str = str2;
        room->setPlayerProperty(player,"hp",str.toInt());

        str = props.value("equip","");
        QStringList equips = str.split(",");
        foreach(QString equip,equips){
            bool ok;
            equip.toInt(&ok);
            if(!ok)room->installEquip(player,equip);
            else room->moveCardTo(Sanguosha->getCard(equip.toInt()),player,Player::Equip);
        }
        str = props.value("judge","");
        if(str != NULL){
            QStringList judges = str.split(",");
            foreach(QString judge,judges){
                room->moveCardTo(Sanguosha->getCard(judge.toInt()),player,Player::Judging);
            }
        }
        str = props.value("hand","");
        if(str !=NULL){
            QStringList hands = str.split(",");
            foreach(QString hand,hands){
                room->obtainCard(player,hand.toInt());
            }
        }

        QVariant v;
        foreach(const TriggerSkill *skill, player->getTriggerSkills()){
            if(!skill->inherits("SPConvertSkill"))
                room->getThread()->addTriggerSkill(skill);
            else continue;
            if(skill->getTriggerEvents().contains(GameStart))
                skill->trigger(GameStart, player, v);
        }

        QString skills = props.value("acquireSkills","");
        if(skills != NULL){
            foreach(QString skill_name, skills.split(","))
                room->acquireSkill(player, skill_name);
        }

        if(props["chained"] != NULL){
            player->setChained(true);
            room->broadcastProperty(player, "chained");
        }
        if(props["turned"] == "true"){
            if(player->faceUp())
                player->turnOver();
        }

        if(props["kingdom"] != NULL){
            room->setPlayerProperty(player, "kingdom", props["kingdom"]);
        }

        if(props["marks"] != NULL){
            QStringList marks = props["marks"].split(",");
            foreach(QString qs,marks){
                QStringList keys = qs.split("*");
                str = keys.at(1);
                room->setPlayerMark(player, keys.at(0), str.toInt());
            }
        }

        int n, hero_draw = 6 , enemy_draw = 4 ;
        QString draw_conf = props["draw"] ;
        if(draw_conf == NULL){
            n = player->isLord() ? hero_draw : enemy_draw;
        }else{
            n = draw_conf.toInt() ;
        }
        player->drawCards(player->hasAbility("startdraw") ? n + 1 : n);
        room->setTag("DrawnCard",true) ;
        if(props["starter"] != NULL){
            room->setCurrent(player);
            QVariant data = QVariant::fromValue(player);
            room->setTag("Starter", data);
        }
    }
}

QString PassModeRule::getPlayerProp(ServerPlayer *player,QString prop_name, QString default_value) const{
    return players.at(player->getSeat() - 1).value(prop_name,default_value) ;
}

QMap<QString, QString> PassModeRule::getPlayerProps(ServerPlayer *player) const{
    return players.at(player->getSeat() - 1);
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
                room->setTag("Score", 0);
                general_name = room->askForGeneralPass(player,"standard");
            }else{
                general_name = save_data->general_name;
            }

            room->transfigurePass(player, general_name, 1);

            if(stage == 1 ){
                player->gainMark("@exp",50);
            }else{
                PassMode::loadData(room,save_data);
            }
            room->setTag("Turns",0);

            room->askForSkillLearn(player);
            initGameStart(player->getRoom());
        }
        break ;
    }case TurnStart:{
        ServerPlayer* starter = room->getTag("Starter").value<PlayerStar>() ;
        if(starter == NULL){
            starter = room->getLord();
        }
        if(player == starter){
            int turns = room->getTag("Turns").toInt() ;
            room->setTag("Turns", ++turns );
        }
        break;
    }case DrawNCards:{
        if(player->hasAbility("turndraw")){
            data = data.toInt() + 1 ;
        }
        data = data.toInt() + getPlayerProp(player,"draw_plus").toInt() ;
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
    case Death:{
        ServerPlayer *lord = room->getLord() ;

        if(player->getRole() == "rebel"){
            DamageStar damage = data.value<DamageStar>();
            if((!damage || damage->from != lord)){
                lord->drawCards(3);
            }else{
                if(lord->hasAbility("killdraw")){
                    lord->drawCards(1);
                }
            }
        }
    }
    default:
        break;
    }
    return false;
}



const QString PassMode::version = "2.0";
const QString PassMode::savePath = "savedata/pass_mode.sav";
const int PassMode::maxStage = 2 ;

PassMode::PassMode()
{
}

QStringList PassMode::getNeedSaveRoomTagName(){
    static QStringList tagNames;
    if(tagNames.isEmpty())
        tagNames << "Stage" << "Times" << "Turns" << "UseTurns" << "LoadTimes" ;

    return tagNames;
}

QStringList PassMode::getNeedSavePlayerMarkName(){
    static QStringList markNames;
    if(markNames.isEmpty())
        markNames << "@exp" << "@score" ;

    return markNames;
}

int PassMode::getStage() const{
    return ServerInfo.GameMode.remove("_pass_").toInt() ;
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

bool PassMode::saveData(Room *room, SaveDataStruct *save_data){
    if(! save_data)
        save_data = catchSaveInfo(room);

    QByteArray data;
    QString room_tags_str ;
    foreach (QString key_name, save_data->roomTags.keys()) {
        room_tags_str.append(QString("%1:%2").arg(key_name).arg(save_data->roomTags.value(key_name).toString())) ;
        room_tags_str.append("|");
    }
    data.append(room_tags_str.toUtf8().toBase64());
    data.append(QString("\n"));
    data.append(save_data->playerMarks.join("|").toUtf8().toBase64());
    data.append(QString("\n"));
    data.append(save_data->general_name.toUtf8().toBase64());
    data.append(QString("\n"));
    data.append(save_data->skills.join("|").toUtf8().toBase64());
    data.append(QString("\n"));
    data.append(save_data->skills_enhance.join("|").toUtf8().toBase64());
    data.append(QString("\n"));
    data.append(save_data->abilities.join("|").toUtf8().toBase64());
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

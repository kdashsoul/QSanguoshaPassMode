#include "pass-mode-scenario.h"

#include <QFile>
#include <QFileDialog>
#include <QByteArray>
#include <QMessageBox>
#include <QTime>

SaveDataStruct::SaveDataStruct()
    :size(0), stage(0),
    exp(0), lord_maxhp(0),
    times(1),
    read_success(false)
{
}

int SaveDataStruct::default_size = 7;

bool SaveDataStruct::canRead() const{
    return default_size == this->size;
}

bool SaveDataStruct::checkDataFormat() const{
    bool format_right = this->stage >= 0
                       && this->times > 0
                       && this->lord != NULL
                       && this->lord_maxhp > 0;

    return format_right;
}

const QString PassMode::default_hero = "caocao" ;
const QString PassMode::version = "ver1.6.2.1";
const QString PassMode::savePath = "savedata/pass_mode.sav";
PassMode::PassMode(QObject *parent)
    :GameRule(parent)
{
    setObjectName("pass_mode_rule");

    enemy_list  << "shizu+gongshou+jianshi" << "jianshi+qibing+jianshi"
                << "huwei+gongshou+jianshi" << "jianshi+kuangdaoke+shizu"
                << "huwei+caocao+jianshi" << "kuangdaoke+luxun+shizu"
                << "qibing+machao+qibing" << "jianshi+zhaoyun+kuangdaoke"
                << "jianshi+zhouyu+jianshi" << "leishi+guojia+leishi"
                << "huwei+zhangliao+kuangdaoke" << "shizu+shenlumeng+shizu"
                << "huwei+sunshangxiang+lumeng" << "zhugeliang+huangyueying+kuangdaoke"
                << "kuangdaoke+xuchu+xiahoudun" <<"luxun+simayi+leishi"
                << "guojia+caocao+zhenji" << "zhouyu+sunquan+huanggai"
                << "guanyu+liubei+zhangfei" << "lubu+shenguanyu+diaochan";

    exp_map.insert("evil", 4);
    exp_map.insert("wei", 8);
    exp_map.insert("shu", 8);
    exp_map.insert("wu", 8);
    exp_map.insert("qun", 8);
    exp_map.insert("god", 16);

    skill_map.insert("mashu", 15);
    skill_map.insert("kezhi_p", 20);
    skill_map.insert("nuhou_p", 20);
    skill_map.insert("duanyan_p", 25);
    skill_map.insert("fenjin_p", 30);
//    skill_map.insert("niepan", 30);
    skill_map.insert("quanheng_p", 40);
    skill_map.insert("xiuluo", 40);
    skill_map.insert("tipo_p" ,40);
    skill_map.insert("qiangong_p",50);
    skill_map.insert("xiongzi_p", 80);

//    hidden_reward["xiongzi_p"] = "._rewardyingzi|feiying_qingnangshu";

    spec_general["dongzhuo_p"] = QVariant::fromValue(4);

}

static int Restart = 1;

bool PassMode::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    Room *room = player->getRoom();
    ServerPlayer *lord = room->getLord();
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    switch(event){
    case GameStart:{
            if(setjmp(env) == Restart){
                initNextStageStart(player);
                return false;
            }
            if(room->getTag("FirstRound").toBool())
                return false;

            if(room->getTag("Stage").isNull() && askForLoadData(room)){
                setNextStageInfo(room, room->getTag("Stage").toInt()-1, true);
                stageStartDraw(room);
            }else
                initGameStart(player);

            room->setTag("FirstRound", true);
            return false;
        }

    case Death:{
            if(player->isLord()){
                if(! room->getAlivePlayers().empty())
                    room->gameOver("rebel");
            }else{
                if(lord->hasSkill("fenjin_p")){
                    lord->drawCards(1);
                }

                DamageStar damage = data.value<DamageStar>();
                if((!damage || damage->from != lord))
                    lord->drawCards(3);

                int exp = exp_map.value(player->getKingdom(), 0);
                exp += exp_map.value(player->getKingdom(), 0)*(room->getTag("Times").toInt()-1)/2;
                exp = qrand()%(exp + 1) + exp/2;

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
                    int stage = room->getTag("Stage").toInt();
                    if(!goToNextStage(player, stage))
                        break;

                    setNextStageInfo(room, stage);

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

void PassMode::stageStartDraw(Room *room, ServerPlayer *player) const{
    int n = 0;
    int hero_draw = 6 ;
    int enemy_draw = room->getTag("Stage").toInt() > 10 ? 4 : 3 ;
    if(player){
        n = player->isLord() ? hero_draw : enemy_draw;
        player->drawCards(player->hasSkill("fenjin_p") ? ++n : n);
    }else{
        foreach(ServerPlayer *p, room->getPlayers()){
            stageStartDraw(room,p);
        }
    }
}

bool PassMode::askForLoadData(Room *room) const{
    ServerPlayer *lord = room->getLord();
    SaveDataStar save = askForReadData();
    if(!save->canRead() || !save->read_success)
        return false;

    QString choice = room->askForChoice(lord, "savefile", "read+deletesave+cancel");
    if(choice == "cancel")
        return false;
    if(choice == "deletesave"){
        QFile::remove(savePath);
        return false;
    }

    QString exception;
    SaveDataStruct::WrongType exception_type = checkDataValid(save);
    if(exception_type == SaveDataStruct::Ex_Exp){
        exception = "wrong_exp";
    }
    else if(exception_type == SaveDataStruct::Ex_HP){
        exception = "wrong_hp";
    }
    else if(exception_type == SaveDataStruct::Ex_Skills){
        exception = "wrong_skills";
    }
    if(exception != NULL){
        room->askForChoice(lord, "savefile", exception);
        return false;
    }

    save->times = (save->stage >= enemy_list.length()) ? (save->times+1) : save->times;
    save->stage = (save->stage >= enemy_list.length()) ? 0 : save->stage;
    room->setTag("Stage", save->stage+1);
    room->setTag("Times", save->times);

    if(resetPlayerSkills(save)){
        LogMessage log;
        log.type = "#ResetPlayer";
        room->sendLog(log);
    }
    room->transfigure(lord, save->lord, true);
    room->setPlayerProperty(lord, "maxhp", save->lord_maxhp);
    room->setPlayerProperty(lord, "hp", save->lord_maxhp);
    const General *general = Sanguosha->getGeneral(save->lord);
    if(lord->getKingdom() != general->getKingdom())
        room->setPlayerProperty(lord, "kingdom", general->getKingdom());

    lord->gainMark("@exp", save->exp);
    QStringList skills = save->skills.split("+");
    foreach(QString skill, skills)
            room->acquireSkill(lord, skill,true,false);

    if(save->reward_list != NULL)
        room->setTag("Reward", save->reward_list.split("+"));
    setLoadedStageInfo(room);
    return true;
}

void PassMode::setLoadedStageInfo(Room *room) const{
    LogMessage log;
    log.type = "#LoadNextStage";
    log.from = room->getLord();
    log.arg = room->getTag("Stage").toString();
    log.arg2 = room->getTag("Times").toString();
    room->sendLog(log);
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
            room->setTag("Stage", 1);
            room->setTag("Times", 1);

            QString name = room->askForGeneralPass(p,"");

            room->transfigure(p, name, true, true);
            room->setPlayerProperty(p, "maxhp", p->getMaxHP() + 1);
            room->setPlayerProperty(p, "hp", p->getMaxHP());
            const General *general = Sanguosha->getGeneral(name);
            if(p->getKingdom() != general->getKingdom())
                room->setPlayerProperty(p, "kingdom", general->getKingdom());
            // debug
            p->enHanceSkill("rende",1);
            askForLearnSkill(p);
        }else{
            QStringList enemys = enemy_list.at(0).split("+");
            QString enemy_name = enemys.at(p->getSeat()-2) ;
            const General *general = Sanguosha->getGeneral(enemy_name + "_e") ;
            if(! general)
                general = Sanguosha->getGeneral(enemy_name + "_p") ;
            if(! general)
                general = Sanguosha->getGeneral(enemy_name);
            room->transfigure(p, general->objectName(), true, true);
            if(p->getKingdom() != general->getKingdom())
                room->setPlayerProperty(p, "kingdom", general->getKingdom());
        }

        stageStartDraw(room, p);
    }
}

void PassMode::setTimesDifficult(Room *room) const{
    int times = room->getTag("Times").toInt();
    int n = 0, time = times;
    while(time > 1){
        time = (time/2 + time%2);
        n++;
    }

    foreach(ServerPlayer* player, room->getPlayers()){
        if(!player->isLord()){
            room->setPlayerProperty(player, "maxhp", player->getMaxHP() + n);
            room->setPlayerProperty(player, "hp", player->getMaxHP());
        }
    }
}

bool PassMode::askForLearnSkill(ServerPlayer *lord) const{
    Room *room = lord->getRoom();
    QString choice;
    while(choice != "."){
        QString skill_info;
        int min_exp = 999;
        getLearnSkillInfo(lord, skill_info, min_exp);

        choice = room->askForSkillChoice(lord, skill_info);

        QString skill_name = choice ;
        int exp = lord->getMark("@exp");
        int need_exp = skill_map.value(skill_name);
        if(exp >= need_exp){
            exp -= need_exp;
            room->setPlayerMark(lord, "@exp", exp);
            room->acquireSkill(lord, skill_name);

        }

        if(exp < min_exp)
            break;
    }

    return true;
}

void PassMode::getLearnSkillInfo(ServerPlayer *lord, QString &skills, int &min_exp) const{

    foreach (QString key, skill_map.keys()) {
        int value = skill_map.value(key) ;
        if(!lord->hasSkill(key))
            skills.append(key).append(":").append(QString::number(value)).append("+");

        if(value < min_exp)
            min_exp = value;
    }

    if(skills.length() > 0)
        skills.chop(1);

}

bool PassMode::goToNextStage(ServerPlayer *player, int stage) const{
    Room *room = player->getRoom();
    ServerPlayer *lord = room->getLord();

    player->throwAllCards();
    if(!player->faceUp())
        player->turnOver();
    if(player->isChained())
        room->setPlayerProperty(player, "chained", false);
    if(stage >= enemy_list.length()){
        SaveDataStar save = askForReadData();
        if(save->canRead() && save->read_success){
            save->stage = 0;
            save->times = room->getTag("Times").toInt()+1;
            save->exp = lord->getMark("@exp");
            save->reward_list = room->getTag("Reward").toStringList().join("+");
          //  askForSaveData(save);
        }

        room->gameOver("lord");
        return false;
    }

    askForSaveData(room, stage);
    return true;
}

SaveDataStruct *PassMode::catchSaveInfo(Room *room, int stage) const{
    ServerPlayer *lord = room->getLord();
    QStringList lord_skills;
    QSet<QString> skill_names = lord->getAcquiredSkills();
    foreach(QString skill_name, skill_names){
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if(skill->inherits("WeaponSkill")
            || skill->inherits("ArmorSkill")
            || skill_name == "axe"
            || skill_name == "spear")
            continue;
        if(skill->isVisible())
            lord_skills << skill_name;
    }

    SaveDataStruct *save_cache = new SaveDataStruct;
    save_cache->stage       = stage >= 0 ? stage : room->getTag("Stage").toInt();
    save_cache->lord        = lord->getGeneralName();
    save_cache->lord_maxhp  = lord->getMaxHP();
    save_cache->exp         = lord->getMark("@exp");
    save_cache->skills      = lord_skills.join("+");
    save_cache->times       = room->getTag("Times").toInt();
    save_cache->reward_list = room->getTag("Reward").toStringList().join("+");
    save_cache->size        = 7;

    return save_cache;
}

bool PassMode::askForSaveData(Room *room, int told_stage) const{
    if(room->askForChoice(room->getLord(), "savefile", "save+notsave") == "notsave")
        return false;

    SaveDataStar save = catchSaveInfo(room, told_stage);
    return askForSaveData(save);
}

bool PassMode::askForSaveData(SaveDataStruct *save) const{
    if(!save->checkDataFormat())
        return false;
		
    QString stage = QString::number(save->stage);
    QString marks = QString::number(save->exp) ;
    QString lord = save->lord + " " + QString::number(save->lord_maxhp);

    QByteArray data;
    data.append(stage.toUtf8().toBase64());
    data.append(QString("\n"));
    data.append(lord.toUtf8().toBase64());
    data.append(QString("\n"));
    data.append(save->skills.toUtf8().toBase64());
    data.append(QString("\n"));
    data.append(marks.toUtf8().toBase64());
    data.append(QString("\n"));
    data.append(QString::number(save->times).toUtf8().toBase64());
    data.append(QString("\n"));
    data.append(save->reward_list.toUtf8().toBase64());
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

void PassMode::setNextStageInfo(Room *room, int stage, bool save_loaded) const{
    room->setTag("Stage", stage+1);
    ServerPlayer *lord = room->getLord();

    askForLearnSkill(lord);

    if(!save_loaded){
        room->setPlayerProperty(lord, "hp", lord->getMaxHP());
        lord->setAlive(false);
        lord->throwAllCards();
        lord->setAlive(true);
        lord->clearFlags();
        lord->clearHistory();
        if(!lord->faceUp())
            lord->turnOver();
        if(lord->isChained())
            room->setPlayerProperty(lord, "chained", false);

        room->setPlayerProperty(lord, "hp", lord->getMaxHP());
        int exp = lord->getMark("@exp");
        lord->throwAllMarks();
        lord->gainMark("@exp", exp);

        LogMessage log;
        log.type = "#NextStage";
        log.from = lord;
        log.arg = room->getTag("Stage").toString();
        room->sendLog(log);
    }

    int i = 0;
    QStringList enemys = enemy_list.at(stage).split("+");
    foreach(ServerPlayer *player, room->getPlayers()){
        if(!player->isLord()){
            QString enemy_name = enemys.at(i) ;
            const General *general = Sanguosha->getGeneral(enemy_name + "_e") ;
            if(! general)
                general = Sanguosha->getGeneral(enemy_name + "_p") ;
            if(! general)
                general = Sanguosha->getGeneral(enemy_name);
            room->transfigure(player, general->objectName(), true, true);
            if(player->getKingdom() != general->getKingdom())
                room->setPlayerProperty(player, "kingdom", general->getKingdom());
            if(!player->faceUp())
                player->turnOver();
            if(player->isChained())
                room->setPlayerProperty(player, "chained", false);
            player->throwAllMarks();
            i ++ ;
        }

        if(player->isDead())
            room->revivePlayer(player);
    }

    setTimesDifficult(room);
}

SaveDataStruct *PassMode::askForReadData() const{
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
                    save->stage = line.toInt();
                    break;
                }
            case 2:{
                    QRegExp rx("(\\w+)\\s+(\\d+)");
                    if(!rx.exactMatch(line)){
                        save->read_success = false;
                        return save;
                    }

                    QStringList texts = rx.capturedTexts();
                    QString lord_name = texts.at(1);
                    int lord_maxhp = texts.at(2).toInt();

                    save->lord = lord_name;
                    save->lord_maxhp = lord_maxhp;
                    break;
                }
            case 3:{
                    save->skills = line;
                    break;
                }
            case 4:{
                    QRegExp rx("(\\d+)");
                    if(!rx.exactMatch(line)){
                        save->read_success = false;
                        return save;
                    }

                    QStringList texts = rx.capturedTexts();
                    int exp = texts.at(1).toInt();
                    save->exp = exp;
                    break;
                }
            case 5:{
                    save->times = line.toInt();
                    break;
                }
            case 6:{
                    save->reward_list = line;
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
    else
        save->read_success = false;

    save->size = line_num-1;

    return save;
}

SaveDataStruct::WrongType PassMode::checkDataValid(SaveDataStruct *save) const{
    QStringList skill_list = save->skills.split("+");
    const General *lord_general = Sanguosha->getGeneral(save->lord);
    QList<const Skill *> lord_skill = lord_general->findChildren<const Skill *>();
    int check_hp = save->lord_maxhp-1;
    if(skill_list.contains("tipo_p"))
        check_hp--;

    if(check_hp != lord_general->getMaxHp()){
        if(spec_general[save->lord].toInt() != check_hp)
            return SaveDataStruct::Ex_HP;
    }

    if(save->stage == 0 && save->exp > 0 && save->times == 1)
        return SaveDataStruct::Ex_Exp;

    int check_exp = 0;
    for(int level = 0; level < save->stage; level++){
        QString stage_enemy = enemy_list.at(level);
        foreach(QString enemy, stage_enemy.split("+")){
            const General *enemy_one = Sanguosha->getGeneral(enemy + "_e") ;
            if(! enemy_one)
                enemy_one = Sanguosha->getGeneral(enemy + "_p") ;
            if(! enemy_one)
                enemy_one = Sanguosha->getGeneral(enemy);
            int each_exp = exp_map.value(enemy_one->getKingdom());
            each_exp += each_exp/2;
            check_exp += each_exp;
        }
    }
    foreach(QString skill, skill_list){
        int skill_exp = skill_map.value(skill);
        check_exp -= skill_exp;
    }
    if(save->exp > check_exp)
        return SaveDataStruct::Ex_Exp;

    foreach(QString skill, skill_list){
        if(skill == NULL)
            continue;

        bool go_check = false;
        foreach(QString learn_skill, skill_map.keys()){
            if(skill == learn_skill){
                go_check = true;
                break;
            }
        }

        if(go_check)
            continue;
        else{
            foreach(const Skill *skill_object, lord_skill){
                if(skill == skill_object->objectName()){
                    go_check = true;
                    break;
                }
            }
        }

        if(!go_check)
            return SaveDataStruct::Ex_Skills;
    }
    return SaveDataStruct::Struct_Right;
}

bool PassMode::resetPlayerSkills(SaveDataStruct *savedata) const{
    if(savedata->times != 2 || savedata->stage != 0)
        return false;

    savedata->exp = 50;
    return true;
}



bool PassModeScenario::exposeRoles() const{
    return true;
}

void PassModeScenario::assign(QStringList &generals, QStringList &roles) const{
    generals << lord << loyalists << rebels << renegades;

    foreach(QString general, generals){
        if(general == lord)
            roles << "lord";
        else if(loyalists.contains(general))
            roles << "loyalist";
        else if(rebels.contains(general))
            roles << "rebel";
        else
            roles << "renegade";
    }
}

int PassModeScenario::getPlayerCount() const{
    return 4;
}

void PassModeScenario::getRoles(char *roles) const{
    strcpy(roles, "ZFFF");
}

void PassModeScenario::onTagSet(Room *room, const QString &key) const{
    // dummy
}

bool PassModeScenario::generalSelection() const{
    return false;
}


PassModeRule::PassModeRule(Scenario *scenario)
    :ScenarioRule(scenario)
{
    events << GameOverJudge << DrawNCards << Predamaged << CardUsed;

}

bool PassModeRule::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    Room *room = player->getRoom();
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    switch(event){
    case DrawNCards:{
        if(player->getKingdom() == "evil")
            data = data.toInt() - 1 ;
        else if(player->getKingdom() == "evil_god")
            data = data.toInt() + 1 ;

        int times = room->getTag("Times").toInt();
        if(!player->isLord()){
                int n = (int)(2.0/times - 2/times + 0.5);
                data = data.toInt() + (n == 0 ? times/2 : n);
            }
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
    case GameOverJudge:{
            return true ;
            break;
        }
    default:
        break;
    }

    return false;
}

PassModeScenario::PassModeScenario()
        :Scenario("pass_mode")
{
    rule = new PassModeRule(this);

    lord = "lubu";
    rebels << "guanyu" << "liubei" << "zhangfei";

}

ADD_SCENARIO(PassMode)

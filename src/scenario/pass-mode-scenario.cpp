#include "pass-mode-skills.cpp"

#include <QFile>
#include <QFileDialog>
#include <QByteArray>
#include <QMessageBox>

SaveDataStruct::SaveDataStruct()
    :size(0), stage(0), exp(0), nirvana(0), times(1),
    read_success(false)
{}

int SaveDataStruct::default_size = 5;

bool SaveDataStruct::canRead() const{
    return default_size == this->size;
}

PassMode::PassMode(QObject *parent)
    :GameRule(parent)
{
    setObjectName("pass_mode_rule");

    enemy_list  << "shizu+gongshou+yaodao" << "jianwei+qibing+jianwei"
                << "huwei+gongshou+huwei" << "jianwei+yaodao+shizu"
                << "huwei+caocao_p+jianwei" << "gongshou+luxun_p+shizu"
                << "qibing+machao_p+qibing" << "jianwei+zhaoyun_p+huwei"
                << "jianwei+zhouyu_p+jianwei" << "yaodao+guojia_p+yaodao"
                << "huwei+zhangliao_p+qibing" << "shizu+ganning_p+shizu"
                << "huwei+sunshangxiang_p+lumeng_p" << "zhugeliang_p+huangyueying_p+gongshou"
                << "qibing+xuchu_p+xiahoudun_p" <<"luxun_p+huangyueying_p+huwei"
                << "guojia_p+caocao_p+zhenji_p" << "zhouyu_p+sunquan_p+huanggai_p"
                << "zhaoyun_p+liubei_p+zhangfei_p" << "lubu_p+diaochan_p+simayi_p";

    exp_map.insert("evil", 4);
    exp_map.insert("hero", 8);

    skill_map.insert("mashu", 15);
    skill_map.insert("kezhi", 15);
    skill_map.insert("fenjin", 20);
    skill_map.insert("nuhou", 20);
    skill_map.insert("duanyan", 25);
    skill_map.insert("xiuluo", 25);
    skill_map.insert("quanheng", 30);
    skill_map.insert("niepan", 30);
    skill_map.insert("tipo" ,40);
    skill_map.insert("feiying", 55);
    skill_map.insert("xiongzi", 60);

    skill_map_hidden.insert("kuangji", 65);
    skill_map_hidden.insert("mengjin", 70);
    skill_map_hidden.insert("zhiheng", 80);
    skill_map_hidden.insert("wansha", 110);
    skill_map_hidden.insert("jijiu", 120);

    skill_raise["kuangji"] = "nuhou";
    skill_raise["mengjin"] = "feiying";
    skill_raise["zhiheng"] = "quanheng";
    skill_raise["wansha"] = "duanyan";
    skill_raise["jijiu"] = "tipo";
}

static int Restart = 1;

bool PassMode::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    Room *room = player->getRoom();

    switch(event){
    case GameStart:{
            if(room->getTag("Stage").toInt() == 0 && askForLoadData(room)){
                setNextStageInfo(room, room->getTag("Stage").toInt()-1);
                setLoadedStageInfo(room);
            }
            else if(setjmp(env) == Restart)
                initNextStageStart(player);
            else
                initGameStart(player);

            return false;
        }

    case Death:{
            if(player->isLord()){
                if(! room->getAlivePlayers().empty())
                    room->gameOver("rebel");
            }else{
                ServerPlayer *lord = room->getLord();
                if(lord->hasSkill("fenjin")){
                    lord->drawCards(1);
                }

                DamageStar damage = data.value<DamageStar>();
                if((!damage || damage->from != lord))
                    lord->drawCards(3);

                int exp = exp_map.value(player->getKingdom(), 10);
                if(lord->getKingdom() == "hero"){
                    int orgin_exp = lord->getMark("@exp") ;
                    lord->gainMark("@exp",exp);
                    LogMessage log;
                    log.type = "#GainExp";
                    log.from = lord;
                    log.to << player;
                    log.arg = QString::number(exp);
                    log.arg2 = QString::number(orgin_exp+exp);
                    room->sendLog(log);
                }else{
                    LogMessage log;
                    log.type = "#CantGainExp";
                    log.from = lord;
                    room->sendLog(log);
                }

                if(room->getAlivePlayers().length() == 1){
                    int stage = room->getTag("Stage").toInt();
                    if(!goToNextStage(player, stage))
                        break;

                    askForSaveData(room, stage);
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

bool PassMode::askForLoadData(Room *room) const{
    ServerPlayer *lord = room->getLord();
    SaveDataStar save = askForReadData();
    if(!save->canRead() || !save->read_success)
        return false;

    QString choice = room->askForChoice(lord, "savefile", "read+deletesave+notread");
    if(choice == "notread")
        return false;
    if(choice == "deletesave"){
        QString filename = "savedata/pass_mode.sav";
        QFile::remove(filename);
        return false;
    }

    save->times = (save->stage >= enemy_list.length()) ? (save->times+1) : save->times;
    save->stage = (save->stage >= enemy_list.length()) ? 0 : save->stage;
    room->setTag("Stage", save->stage+1);
    room->setTag("Times", save->times);

    QStringList skills = save->skills.split("+");
    foreach(QString skill, skills)
        room->acquireSkill(lord, skill);

    room->transfigure(lord, save->lord, true);
    room->setPlayerProperty(lord, "maxhp", save->lord_maxhp);
    room->setPlayerProperty(lord, "hp", lord->getMaxHP());
    const General *general = Sanguosha->getGeneral(save->lord);
    if(lord->getKingdom() != general->getKingdom())
        room->setPlayerProperty(lord, "kingdom", general->getKingdom());

    lord->gainMark("@exp", save->exp);
    room->setPlayerMark(lord, "@nirvana", save->nirvana);

    return true;
}

void PassMode::setLoadedStageInfo(Room *room) const{
    LogMessage log;
    log.type = "#LoadNextStage";
    log.from = room->getLord();
    log.arg = room->getTag("Stage").toString();
    log.arg2 = room->getTag("Times").toString();
    room->sendLog(log);

    int n = 6;
    if(room->getLord()->hasSkill("fenjin"))
        n++;
    room->getLord()->drawCards(n);
}

void PassMode::initNextStageStart(ServerPlayer *player) const{
    Room *room = player->getRoom();
    room->setTag("SwapPile", 0);
    room->setPlayerProperty(player, "phase", "not_active");
    room->setCurrent(room->getLord());
    foreach(ServerPlayer *p, room->getPlayers()){
        int n = p->isLord() ? 6 : 4;
        if(p->hasSkill("fenjin"))
            n ++ ;
        p->drawCards(n);
    }
    room->getLord()->play();
}

void PassMode::initGameStart(ServerPlayer *player) const{
    Room *room = player->getRoom();
    if(player->isLord()){
        room->setTag("Stage", 1);
        room->setTag("Times", 1);

        const Package *passpack = Sanguosha->findChild<const Package *>("pass_mode");
        QList<const General *> generals = passpack->findChildren<const General *>();

        QStringList names;
        foreach(const General *general, generals){
            if(general->getKingdom() == "hero")
                names << general->objectName();
        }

        QString name = room->askForGeneral(player, names);

        room->transfigure(player, name, true, true);
        room->setPlayerProperty(player, "maxhp", player->getMaxHP() + 1);
        room->setPlayerProperty(player, "hp", player->getMaxHP());
        const General *general = Sanguosha->getGeneral(name);
        if(player->getKingdom() != general->getKingdom())
            room->setPlayerProperty(player, "kingdom", general->getKingdom());
    }
    int n = player->isLord() ? 6 : 4;
    if(player->hasSkill("fenjin"))
        n ++ ;
    player->drawCards(n);
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
    while(choice != "cancel"){
        QString skill_names;
        int min_exp = 999;
        bool hidden_learn = askForLearnHiddenSkill(lord, skill_names, min_exp);

        choice = room->askForChoice(lord, "study", skill_names);
        QString skill_name = choice.split("_").at(0);
        int exp = lord->getMark("@exp");
        int need_exp = hidden_learn ? skill_map_hidden.value(skill_name) : skill_map.value(skill_name);
        if(exp >= need_exp){
            exp -= need_exp;
            room->setPlayerMark(lord, "@exp", exp);
            room->acquireSkill(lord, skill_name);
            if(skill_raise[skill_name] != NULL){
                room->detachSkillFromPlayer(lord, skill_raise[skill_name]);
                if(skill_raise[skill_name] == "tipo")
                    room->setPlayerProperty(lord, "maxhp", lord->getMaxHP() - 1);
            }

            if(skill_name == "tipo"){
                room->setPlayerProperty(lord, "maxhp", lord->getMaxHP() + 1);
            }else if(skill_name == "niepan"){
                lord->gainMark("@nirvana");
            }
        }

        if(exp < min_exp)
            break;
    }

    return true;
}

bool PassMode::askForLearnHiddenSkill(ServerPlayer *lord, QString &skills, int &min_exp) const{
    bool has_learnt = false;
    foreach(QString key, skill_map_hidden.keys()){
        if(lord->hasSkill(key)){
            has_learnt = true;
            break;
        }
    }
    if(!has_learnt){
        has_learnt = true;
        foreach(QString key, skill_map.keys()){
            if(!lord->hasSkill(key)){
                has_learnt = false;
                break;
            }
        }
    }

    if(!has_learnt){
        foreach (QString key, skill_map.keys()) {
            if(!lord->hasSkill(key))
                skills.append(key).append("_s+");

            if(skill_map.value(key) > min_exp)
                min_exp = skill_map.value(key);
        }
    }
    else{
        foreach (QString key, skill_map_hidden.keys()) {
            if(!lord->hasSkill(key))
                skills.append(key).append("_s+");

            if(skill_map_hidden.value(key) > min_exp)
                min_exp = skill_map_hidden.value(key);
        }
    }
    skills.append("cancel");

    return has_learnt;
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
        /*SaveDataStar save = askForReadData();
        if(save->canRead() && save->read_success){
            save->stage = 0;
            save->times = room->getTag("Times").toInt()+1;
            save->exp = lord->getMark("@exp");
            askForSaveData(save);
        }*/

        room->gameOver("lord");
        return false;
    }
    stage ++ ;
    room->setTag("Stage", stage);

    LogMessage log;
    log.type = "#NextStage";
    log.from = lord;
    log.arg = QString::number(stage);
    room->sendLog(log);

    askForLearnSkill(lord);

    room->setPlayerProperty(lord, "hp", lord->getMaxHP());
    lord->setAlive(false);
    room->broadcastInvoke("killPlayer", lord->objectName());
    lord->throwAllCards();
    room->broadcastInvoke("revivePlayer", lord->objectName());
    lord->setAlive(true);
    lord->clearFlags();
    lord->clearHistory();

    room->setPlayerProperty(lord, "hp", lord->getMaxHP());
    int exp = lord->getMark("@exp");
    int nirvana = lord->getMark("@nirvana");
    lord->throwAllMarks();
    lord->gainMark("@exp", exp);
    if(nirvana > 0)
        lord->gainMark("@nirvana");

    return true;
}

bool PassMode::askForSaveData(Room *room, int told_stage) const{
    ServerPlayer *lord = room->getLord();
    if(room->askForChoice(lord, "savefile", "save+notsave") == "notsave")
        return false;

    QStringList lord_skills;
    QList<const Skill *> skills = lord->getVisibleSkillList();
    foreach(const Skill *skill, skills){
        if(skill->inherits("WeaponSkill") || skill->inherits("ArmorSkill"))
            continue;

        lord_skills << skill->objectName();
    }

    SaveDataStruct save;
    save.stage      = told_stage;
    save.lord       = lord->getGeneralName();
    save.lord_maxhp = lord->getMaxHP();
    save.exp        = lord->getMark("@exp");
    save.nirvana    = lord->getMark("@nirvana");
    save.skills     = lord_skills.join("+");
    save.times      = room->getTag("Times").toInt();
    save.size       = 5;

    return askForSaveData(&save);
}

bool PassMode::askForSaveData(SaveDataStruct *save) const{
    QString stage = QString::number(save->stage);
    QString marks = QString::number(save->exp) + " " + QString::number(save->nirvana);
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

    QString filename = "savedata/pass_mode.sav";
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return file.write(data) != -1;
    else
        return false;
}

void PassMode::setNextStageInfo(Room *room, int stage) const{
    int i = 0;
    QStringList enemys = enemy_list.at(stage).split("+") ;
    foreach(ServerPlayer *player, room->getPlayers()){
        if(!player->isLord()){
            room->transfigure(player, enemys.at(i), true, true);
            const General *general = Sanguosha->getGeneral(enemys.at(i));
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
    QFile file("savedata/pass_mode.sav");
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
                    QRegExp rx("(\\d+)\\s+(\\d+)");
                    if(!rx.exactMatch(line)){
                        save->read_success = false;
                        return save;
                    }

                    QStringList texts = rx.capturedTexts();
                    int exp = texts.at(1).toInt();
                    int nirvana = texts.at(2).toInt();
                    save->exp = exp;
                    save->nirvana = nirvana;
                    break;
                }
            case 5:{
                    save->times = line.toInt();
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


class PassModeRule: public ScenarioRule{
public:
    PassModeRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameOverJudge << DrawNCards;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        switch(event){
        case DrawNCards:{
            if(player->getKingdom() == "evil")
                data = data.toInt() - 1 ;

            int times = room->getTag("Times").toInt();
            if(!player->isLord()){
                    int n = (int)(2.0/times - 2/times + 0.5);
                    data = data.toInt() + (n == 0 ? times/2 : n);
                }
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
};

PassModeScenario::PassModeScenario()
        :Scenario("pass_mode")
{
    rule = new PassModeRule(this);

    addGeneralAndSkills();
}

ADD_SCENARIO(PassMode)

#include "pass-mode-scenario.h"
#include "engine.h"
#include "standard-skillcards.h"
#include "clientplayer.h"
#include "client.h"
#include "carditem.h"

class Duanyan:public ZeroCardViewAsSkill{
public:
    Duanyan():ZeroCardViewAsSkill("duanyan"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("DuanyanCard");
    }

    virtual const Card *viewAs() const{
        return new DuanyanCard;
    }
};

DuanyanCard::DuanyanCard(){
    once = true;
}

bool DuanyanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self ;
}

void DuanyanCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *player = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = player->getRoom();

    QString choice = room->askForChoice(player, "duanyan", "slash+jink+peach_analeptic+other");

    int card_id = target->getRandomHandCardId();
    const Card *card = Sanguosha->getCard(card_id);
    LogMessage log;
    log.type = "#ChooseType";
    log.from = player;
    log.to << target;
    log.arg = choice;
    room->sendLog(log);

    room->showCard(target, card_id);
    room->getThread()->delay();

    bool correct ;
    bool isSlash = card->inherits("Slash") ;
    bool isJink = card->inherits("Jink") ;
    bool isPna = card->inherits("Peach") || card->inherits("Analeptic") ;

    if(choice == "slash"){
        correct = isSlash ;
    }else if(choice == "jink"){
        correct = isJink ;
    }else if(choice == "peach_analeptic"){
        correct = isPna ;
    }else{
        correct = !(isSlash || isJink || isPna);
    }

    if(correct){
        room->throwCard(card_id);
    }else{
        if(!room->askForDiscard(player, objectName(), 1, true , true)){
            player->drawCards(1);
            DamageStruct damage;
            damage.card = NULL;
            damage.from = target;
            damage.to = player;
            room->damage(damage);
        }
    }
}


class Xiongzi:public TriggerSkill{
public:
    Xiongzi():TriggerSkill("xiongzi"){
        events << PhaseChange ;
        frequency = Frequent;
    }


    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getPhase() == Player::Start;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        player->drawCards(1);
        return false ;
    }
};


class Qiangong:public TriggerSkill{
public:
    Qiangong():TriggerSkill("qiangong"){
        events << CardEffected ;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        Room *room = player->getRoom();
        if((effect.card->inherits("Dismantlement") || effect.card->inherits("Snatch")) && player->askForSkillInvoke(objectName())){
            LogMessage log;
            log.from = player;
            if(effect.card->inherits("Dismantlement") && player->getEquips().empty()){
                room->askForDiscard(player, objectName(), 1 , false , true) ;
                log.type = "#Qiangong1";
                player->getRoom()->sendLog(log);
                return true;
            }else{
                const Card *card = room->askForCard(player,".NT","@qiangong");
                if(!card){
                    if(player->isKongcheng()){
                        card = player->getEquips().at(0);
                    }else{
                        card = player->getRandomHandCard() ;
                    }
                }
                room->moveCardTo(card, effect.from, Player::Hand);
                log.to << effect.from;
                log.type = "#Qiangong2";
                player->getRoom()->sendLog(log);
                return true;
            }
        }
        return false ;
    }
};


class Quanheng:public ViewAsSkill{
public:
    Quanheng():ViewAsSkill("quanheng"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        int n = qMin(4, Self->getLostHp());
        if(selected.length() != n)
            return !to_select->isEquipped() ;
        return false ;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.empty())
            return NULL;

        QuanhengCard *quanheng_card = new QuanhengCard;
        quanheng_card->addSubcards(cards);

        return quanheng_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QuanhengCard") && player->isWounded() ;
    }
};


QuanhengCard::QuanhengCard(){
    target_fixed = true;
    once = true;
}

void QuanhengCard::use(Room *room, ServerPlayer *player, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    player->drawCards(this->getSubcards().length());
}


class Shiqi:public TriggerSkill{
public:
    Shiqi():TriggerSkill("shiqi"){
        events << PhaseChange << FinishJudge;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *shizu, QVariant &data) const{
        if(event == PhaseChange && shizu->getPhase() == Player::Draw){
            Room *room = shizu->getRoom();
            if(shizu->askForSkillInvoke("shiqi")){
                shizu->setFlags("shiqi");
                room->playSkillEffect(objectName());

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = shizu;

                room->judge(judge);
            }
            shizu->setFlags("-shiqi");
        }else if(event == FinishJudge){
            if(shizu->hasFlag("shiqi")){
                JudgeStar judge = data.value<JudgeStar>();
                if(judge->card->isRed()){
                    shizu->obtainCard(judge->card);
                    return true;
                }
            }
        }
        return false;
    }
};

class Qianggong: public SlashBuffSkill{
public:
    Qianggong():SlashBuffSkill("qianggong"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *gongshou = effect.from;
        Room *room = gongshou->getRoom();

        if(gongshou->getHp() <= 1 || gongshou->getAttackRange() > 3){
            if(gongshou->askForSkillInvoke("liegong", QVariant::fromValue(effect))){
                room->playSkillEffect("liegong");
                room->slashResult(effect, NULL);
                return true;
            }
        }
        return false;
    }
};

class Pojia: public TriggerSkill{
public:
    Pojia():TriggerSkill("pojia"){
        events << SlashEffect << SlashMissed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(event == SlashEffect){
            if(effect.slash->isBlack()){
                effect.to->addMark("qinggang");
            }
        }else if(event == SlashMissed){
            if(!effect.slash->isRed())
                return false;
            if(effect.to->hasSkill("kongcheng") && effect.to->isKongcheng())
                return false;
            Room *room = player->getRoom();
            const Card *card = room->askForCard(player, "slash", "blade-slash");
            if(card){
                // if player is drank, unset his flag
                if(player->hasFlag("drank"))
                    room->setPlayerFlag(player, "-drank");

                CardUseStruct use;
                use.card = card;
                use.from = player;
                use.to << effect.to;
                room->useCard(use, false);
            }
        }
        return false;
    }
};

class ZhanshangPass: public TriggerSkill{
public:
    ZhanshangPass():TriggerSkill("zhanshang_pass"){
        events << CardEffected;
        frequency = Frequent ;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.card->inherits("AOE")){
            Room *room = player->getRoom();
            player->drawCards(1);
            LogMessage log;
            log.type = "#ZhanshangPass";
            log.from = player;
            room->sendLog(log);
        }

        return false;
    }
};

class Qishu: public DistanceSkill{
public:
    Qishu():DistanceSkill("qishu"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        if(from->hasSkill(objectName())){
            correct --;
            if(from->getOffensiveHorse() != NULL)
                correct -= from->getOffensiveHorse()->getCorrect() ;
        }
        if(to->hasSkill(objectName())){
            correct ++;
            if(to->getDefensiveHorse() != NULL)
                correct -= to->getDefensiveHorse()->getCorrect() ;
        }

        return correct;
    }
};

class Chenwen: public TriggerSkill{
public:
    Chenwen():TriggerSkill("chenwen"){
        events << SlashEffected;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getArmor() == NULL;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if(effect.slash->getSuit() == Card::Club){
            player->getRoom()->playSkillEffect(objectName());

            LogMessage log;
            log.type = "#SkillNullify";
            log.from = player;
            log.arg = objectName();
            log.arg2 = effect.slash->objectName();

            player->getRoom()->sendLog(log);

            return true;
        }

        return false;
    }
};

class Zhongzhuang: public TriggerSkill{
public:
    Zhongzhuang():TriggerSkill("zhongzhuang"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.damage > 1){
            LogMessage log;
            log.type = "#Zhongzhuang";
            log.from = player;
            log.arg = QString::number(damage.damage);
            player->getRoom()->sendLog(log);

            damage.damage = 1;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};


class Yaoshu: public OneCardViewAsSkill{
public:
    Yaoshu():OneCardViewAsSkill("yaoshu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YaoshuCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        YaoshuCard *yaoshu_card = new YaoshuCard;
        yaoshu_card->addSubcard(card_item->getCard()->getId());

        return yaoshu_card;
    }
};

YaoshuCard::YaoshuCard(){
    once = true;
}

bool YaoshuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void YaoshuCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *yaodao = effect.from;
    ServerPlayer *target = effect.to;

    Room *room = yaodao->getRoom();
    room->setEmotion(target, "bad");

    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(spade|club):([2-9])");
    judge.good = false;
    judge.reason = "leiji";
    judge.who = target;

    room->judge(judge);

    if(judge.isBad()){
        DamageStruct damage;
        damage.card = NULL;
        damage.damage = 1;
        damage.from = yaodao;
        damage.to = target;
        damage.nature = DamageStruct::Thunder;

        room->damage(damage);
    }else
        room->setEmotion(yaodao, "bad");
}



class Jitian: public TriggerSkill{
public:
    Jitian():TriggerSkill("jitian"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Thunder){
            LogMessage log;
            log.type = "#Jitian";
            log.from = player;
            player->getRoom()->sendLog(log);
            player->drawCards(damage.damage);
            damage.damage = 0;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};


class LianzhanPass: public TriggerSkill{
public:
    LianzhanPass():TriggerSkill("lianzhan_pass"){
        events << SlashMissed << Predamage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = player->getRoom();
        if(event == SlashMissed){
            if(effect.to->hasSkill("kongcheng") && effect.to->isKongcheng())
                return false;
            const Card *card = room->askForCard(player, "slash", "blade-slash");
            if(card){
                // if player is drank, unset his flag
                if(player->hasFlag("drank"))
                    room->setPlayerFlag(player, "-drank");

                room->setPlayerFlag(player, "lianzhan");
                CardUseStruct use;
                use.card = card;
                use.from = player;
                use.to << effect.to;
                room->useCard(use, false);
                room->setPlayerFlag(player, "-lianzhan");
            }
        }else if(event == Predamage){
            DamageStruct damage = data.value<DamageStruct>();
            const Card *reason = damage.card;
            if(reason && reason->inherits("Slash") && player->hasFlag("lianzhan")){
                LogMessage log;
                log.type = "#LianzhanPass";
                log.from = player;
                log.to << damage.to;
                log.arg = QString::number(damage.damage);
                log.arg2 = QString::number(damage.damage + 1);
                player->getRoom()->sendLog(log);

                damage.damage ++;
                data = QVariant::fromValue(damage);
            }
        }
        return false;
    }
};

class DouzhiPass: public TriggerSkill{
public:
    DouzhiPass():TriggerSkill("douzhi_pass"){
        events << PhaseChange;
        frequency = Frequent ;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        int n = 0;
        if(player->getPhase() == Player::Draw){
            if(player->isKongcheng() && player->askForSkillInvoke(objectName())){
                n = player->getHp() ;
            }
        }else if(player->getPhase() == Player::Finish){
            if(player->getHandcardNum() < player->getHp() && player->askForSkillInvoke(objectName())){
                n = player->getHp() - player->getHandcardNum() ;
            }
        }
        if(n > 0){
            player->drawCards(n);
            LogMessage log;
            log.type = "#DouzhiPass";
            log.from = player;
            log.arg = QString::number(n);
            room->sendLog(log);
        }
        return false;
    }
};


class RendePassViewAsSkill:public ViewAsSkill{
public:
    RendePassViewAsSkill():ViewAsSkill("rende_pass"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return true ;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        RendePassCard *rende_pass_card = new RendePassCard;
        rende_pass_card->addSubcards(cards);
        return rende_pass_card;
    }
};

class RendePass: public PhaseChangeSkill{
public:
    RendePass():PhaseChangeSkill("rende_pass"){
        view_as_skill = new RendePassViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::NotActive
                && target->hasUsed("RendePassCard");
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        target->getRoom()->setPlayerMark(target, "rende_pass", 0);
        return false;
    }
};


RendePassCard::RendePassCard(){
    will_throw = false;
}

void RendePassCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(targets.isEmpty())
        return;

    ServerPlayer *target = targets.first();
    room->moveCardTo(this, target, Player::Hand, false);

    int old_value = source->getMark("rende_pass");
    int new_value = old_value + subcards.length();
    room->setPlayerMark(source, "rende_pass", new_value);
    room->playSkillEffect("rende");

    if(old_value < 1 && new_value >= 1){
        RecoverStruct recover;
        recover.card = this;
        recover.who = source;
        room->recover(source, recover);
    }
    source->gainMark("@renyi",subcards.length());
}


class JijiangPass: public ZeroCardViewAsSkill{
public:
    JijiangPass():ZeroCardViewAsSkill("jijiang_pass"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return (player->canSlashWithoutCrossbow() || player->hasWeapon("crossbow")) && !player->hasUsed("JijiangPassCard");
    }

    virtual const Card *viewAs() const{
        return new JijiangPassCard;
    }
};


JijiangPassCard::JijiangPassCard(){
    mute = true;
}

bool JijiangPassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;
    if(to_select == Self)
        return false;
    return true;
}

void JijiangPassCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->playSkillEffect("jijiang");
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("jijiang_pass");
    CardUseStruct use;
    use.card = slash;
    use.from = source;
    use.to = targets;

    room->useCard(use);
}

class JijiangCost: public TriggerSkill{
public:
    JijiangCost():TriggerSkill("#jijiang_cost"){
        events << SlashHit;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *liubei, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = liubei->getRoom();
        if(effect.slash->getSkillName() == "jijiang_pass"){
            if(liubei->getMark("@renyi") > 0 && room->askForChoice(liubei , "jijiangcost" , "losemark+losehp") == "losemark"){
                liubei->loseMark("@renyi");
            }else{
                room->loseHp(liubei);
                LogMessage log;
                log.type = "#JijiangLoseHp";
                log.from = liubei;
                room->sendLog(log);
            }
        }
        return false;
    }
};

class ZhaoliePass: public PhaseChangeSkill{
public:
    ZhaoliePass():PhaseChangeSkill("zhaolie_pass"){
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::NotActive
                && target->getMark("@renyi") > target->getHp()  ;
    }

    virtual bool onPhaseChange(ServerPlayer *liubei) const{
        if(!liubei->askForSkillInvoke(objectName()))
            return false;

        int n = liubei->getMark("@renyi");
        liubei->loseMark("@renyi",liubei->getMark("@renyi"));

        Room *room = liubei->getRoom();

        LogMessage log;
        log.type = "#ZhaolieCanInvoke";
        log.from = liubei;
        log.arg = QString::number(n);
        room->sendLog(log);

        RecoverStruct recover;
        recover.who = liubei;
        room->recover(liubei, recover);
        room->getThread()->trigger(TurnStart, liubei);

        return false;
    }
};



class WenjiuPass:public TriggerSkill{
public:
    WenjiuPass():TriggerSkill("wenjiu_pass"){
        events << PhaseChange ;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *guanyu, QVariant &data) const{
        Room *room = guanyu->getRoom() ;
        if(event == PhaseChange){
            if(guanyu->getPhase() == Player::Start){
                if(guanyu->askForSkillInvoke("wenjiu_pass")){
                    room->setPlayerFlag(guanyu, "wenjiu");
                    guanyu->setMark("@wenjiu",2);
                    guanyu->drawCards(1);
                }
                if(guanyu->getMark("@wenjiu") > 0){
                    guanyu->setFlags("wenjiu");
                }
            }else if(guanyu->getPhase() == Player::Finish){
                if(guanyu->getMark("@wenjiu") > 0){
                    guanyu->loseMark("@wenjiu");
                }
                if(guanyu->hasFlag("wenjiu")){
                    room->setPlayerFlag(guanyu, "-wenjiu");
                }
            }
        }
        return false;
    }
};

class WenjiuBuff: public TriggerSkill{
public:
    WenjiuBuff():TriggerSkill("#wenjiu"){
        events << Predamage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasFlag("wenjiu") && target->isAlive();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *guanyu, QVariant &data) const{
        Room *room = guanyu->getRoom() ;
        DamageStruct damage = data.value<DamageStruct>();

        const Card *reason = damage.card;
        if(reason == NULL)
            return false;

        if(reason->inherits("Slash") || reason->inherits("Duel")){
            LogMessage log;
            log.type = "#WenjiuBuff";
            log.from = guanyu;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            guanyu->getRoom()->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
            room->setPlayerFlag(guanyu, "-wenjiu");
        }

        return false;
    }
};


class TuodaoPass: public OneCardViewAsSkill{
public:
    TuodaoPass():OneCardViewAsSkill("tuodao_pass"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("TuodaoPassCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return (to_select->getCard()->isBlack() && to_select->getFilteredCard()->inherits("Weapon")) ||
                (to_select->getCard()->isRed() && to_select->getFilteredCard()->inherits("Horse"));
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        TuodaoPassCard *tuodao_pass_card = new TuodaoPassCard;
        tuodao_pass_card->addSubcard(card_item->getCard()->getId());

        return tuodao_pass_card;
    }
};

TuodaoPassCard::TuodaoPassCard(){
    once = true;
}

void TuodaoPassCard::use(Room *room, ServerPlayer *guanyu, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    ServerPlayer *to = guanyu;
    ServerPlayer *from = targets.at(0);

    Duel *duel = new Duel(Card::NoSuit, 0);
    duel->setSkillName("tuodao");
    duel->setCancelable(false);

    CardUseStruct use;
    use.from = from;
    use.to << to;
    use.card = duel;
    room->useCard(use);
}


class BaonuPass: public TriggerSkill{
public:
    BaonuPass():TriggerSkill("baonu_pass"){
        events << SlashHit;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getLostHp() >= 3 ;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *zhangfei, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        zhangfei->drawCards(1);
        LogMessage log;
        log.type = "#BaonuPass";
        log.from = zhangfei;
        log.to << effect.to;
        zhangfei->getRoom()->sendLog(log);
        return false;
    }
};

class DuanhePass:public OneCardViewAsSkill{
public:
    DuanhePass():OneCardViewAsSkill("duanhe_pass"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("Weapon");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new DuanhePassCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

DuanhePassCard::DuanhePassCard(){
}

bool DuanhePassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    int card_id = getSubcards().first();
    const Weapon *weapon = qobject_cast<const Weapon *>(Sanguosha->getCard(card_id));
    return to_select != Self && (to_select->getHp() + to_select->getHandcardNum()) >= weapon->getRange() ;
}

void DuanhePassCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->loseHp(effect.from);
    if(!effect.to->isNude()){
        int card_id = room->askForCardChosen(effect.from, effect.to, "he", "duanhe_pass");
        room->throwCard(card_id);
    }
    effect.to->turnOver();
    DamageStruct damage;
    damage.from = effect.from;
    damage.to = effect.to;
    room->damage(damage);
}

class TiejiPass:public TriggerSkill{
public:
    TiejiPass():TriggerSkill("tieji_pass"){
        events << SlashProceed << Predamage;
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == SlashProceed){
            if(data.canConvert<SlashEffectStruct>()){
                SlashEffectStruct effect = data.value<SlashEffectStruct>();

                if(player->isAlive()){
                    ServerPlayer *machao = effect.from;

                    Room *room = machao->getRoom();
                    if(effect.from->askForSkillInvoke("tieji", QVariant::fromValue(effect))){
                        room->playSkillEffect("tieji");

                        JudgeStruct judge;
                        judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                        judge.good = true;
                        judge.reason = objectName();
                        judge.who = machao;

                        room->judge(judge);
                        if(judge.isGood()){
                            if(judge.card->getSuit() == Card::Diamond){
                                effect.to->setFlags("tieji_damage");
                            }else if(judge.card->getSuit() == Card::Heart){
                                machao->obtainCard(judge.card);
                            }
                            room->slashResult(effect, NULL);
                            return true;
                        }
                    }
                    return false;
                }
            }
        }else if(event == Predamage){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.to->hasFlag("tieji_damage")){
                damage.damage ++ ;
                data = QVariant::fromValue(damage);
                damage.to->setFlags("-tieji_damage");
            }
        }
        return false;
    }
};


class MashuPass: public DistanceSkill{
public:
    MashuPass():DistanceSkill("mashu_pass"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        if(from->hasSkill(objectName())){
            correct -- ;
        }
        if(to->hasSkill(objectName())){
            if(to->getOffensiveHorse() != NULL)
                correct ++ ;
            if(to->getDefensiveHorse() != NULL)
                correct ++ ;
        }
        return correct;
    }
};

class LongweiPass: public TriggerSkill{
public:
    LongweiPass():TriggerSkill("longwei_pass"){
        events << CardLost;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *zhaoyun, QVariant &data) const{
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->to_place == Player::Equip && Sanguosha->getCard(move->card_id)->inherits("Weapon")){
            Room *room = zhaoyun->getRoom();
            if(room->askForSkillInvoke(zhaoyun, objectName())){
                QList<ServerPlayer *> targets;
                foreach(ServerPlayer *target, room->getOtherPlayers(zhaoyun)){
                    if(zhaoyun->canSlash(target,false) && (target->getHp() > zhaoyun->getHp() || target->getHandcardNum() > zhaoyun->getHandcardNum()))
                        targets << target;
                }

                ServerPlayer *target = room->askForPlayerChosen(zhaoyun, targets, "longwei-slash");
                if(!target)
                    return false;

                Slash *slash = new Slash(Card::NoSuit, 0);
                slash->setSkillName(objectName());

                CardUseStruct card_use;
                card_use.card = slash;
                card_use.from = zhaoyun;
                card_use.to << target;
                room->useCard(card_use, false);
            }
        }
        return false;
    }
};


class Longhou: public TriggerSkill{
public:
    Longhou():TriggerSkill("longhou"){
        events << SlashHit;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *zhaoyun, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = zhaoyun->getRoom();
        if(effect.to->getWeapon() != NULL && effect.to != zhaoyun && !zhaoyun->isNude() && zhaoyun->askForSkillInvoke(objectName(), QVariant::fromValue(effect.to))){
            room->moveCardTo(effect.to->getWeapon(),zhaoyun,Player::Hand);
            if(!room->askForDiscard(zhaoyun, "longhou", 2 , true)){
                room->loseHp(zhaoyun);
            }
        }
        return false;
    }
};


class GuanxingPass:public PhaseChangeSkill{
public:
    GuanxingPass():PhaseChangeSkill("guanxing_pass"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *zhuge) const{
        if(zhuge->getPhase() == Player::Start &&
           zhuge->askForSkillInvoke(objectName()))
        {
            Room *room = zhuge->getRoom();
            room->playSkillEffect(objectName());
            int n = 3 + qMax(0, (zhuge->getLostHp() + 1) / 2) ;
            room->doGuanxing(zhuge, room->getNCards(n, false), false);
        }
        return false;
    }
};

class BeifaPass: public ViewAsSkill{
public:
    BeifaPass():ViewAsSkill("beifa_pass"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.isEmpty())
            return !to_select->isEquipped();
        else if(selected.length() == 1){
            const Card *card = selected.first()->getFilteredCard();
            return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == card->getSuit();
        }else
            return false;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("BeifaPassCard") ;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 2){
            Card *card = new BeifaPassCard;
            card->addSubcards(cards);
            return card;
        }else
            return NULL;
    }
};

BeifaPassCard::BeifaPassCard(){
    once = true;
    target_fixed = true;
}

void BeifaPassCard::use(Room *room, ServerPlayer *zhuge, const QList<ServerPlayer *> &) const{
    int card_id = this->getSubcards().first();
    room->showCard(zhuge, card_id);
    room->throwCard(this);
    const Card *card = Sanguosha->getCard(card_id);
    foreach(ServerPlayer *player, room->getOtherPlayers(zhuge)){
        bool success = false;
        if(player->isKongcheng()){
            success = true;
        }else{
            QString suit_str = card->getSuitString();
            QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
            QString prompt = QString("@beifa_pass:%1::%2").arg(zhuge->getGeneralName()).arg(suit_str);
            const Card *acard = room->askForCard(player, pattern, prompt);
            if(acard){
                room->showCard(player, acard->getEffectiveId());
            }else
                success = true;
        }
        if(success){
            if(player->isNude()){
                DamageStruct damage;
                damage.from = zhuge;
                damage.to = player;
                room->damage(damage);
            }else{
                room->throwCard(room->askForCardChosen(zhuge, player, "he", "beifa_pass"));
            }
        }else{
            room->loseHp(zhuge);
        }
    }
    room->throwCard(card_id);
}

class JizhiPass:public TriggerSkill{
public:
    JizhiPass():TriggerSkill("jizhi_pass"){
        frequency = Frequent;
        events << CardUsed << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yueying, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->inherits("TrickCard")){
            Room *room = yueying->getRoom();
            if(room->askForSkillInvoke(yueying, objectName())){
                room->playSkillEffect("jizhi");
                yueying->drawCards(1);
                yueying->gainMark("@zhimou");
            }
        }
        return false;
    }
};

class ShipoPass:public TriggerSkill{
public:
    ShipoPass():TriggerSkill("shipo_pass"){
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yueying, QVariant &data) const{
        return false;
    }
};

class JumouPass: public TriggerSkill{
public:
    JumouPass():TriggerSkill("jumou_pass"){
        events << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Nullification")){
            Room *room = player->getRoom();
            if(room->getCardPlace(use.card->getEffectiveId()) == Player::DiscardedPile){
                QList<ServerPlayer *> players = room->getAllPlayers();
                foreach(ServerPlayer *p, players){
                    if(p->hasSkill(objectName()) && p->getMark("@zhimou") >= p->getMaxHP() && room->askForSkillInvoke(p, objectName())){
                        p->obtainCard(use.card);
                        p->loseMark("@zhimou",p->getMark("@zhimou"));
                        room->playSkillEffect(objectName());
                        break;
                    }
                }
            }
        }

        return false;
    }
};

class LiegongPass: public OneCardViewAsSkill{
public:
    LiegongPass():OneCardViewAsSkill("liegong_pass"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        int card_id = Self->getMark("liegong_mark");
        const Card *card = Sanguosha->getCard(card_id);
        return to_select->getFilteredCard()->getSuit() == card->getSuit();
        return !to_select->isEquipped() ;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Self->getMark("liegong_mark") > 0 && ! player->hasUsed("LiegongPassCard") ;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LiegongPassCard *card = new LiegongPassCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

LiegongPassCard::LiegongPassCard(){
    mute = true;
}

bool LiegongPassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;
    if(to_select == Self)
        return false;
    return true;
}

void LiegongPassCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->playSkillEffect("liegong");
    room->throwCard(this);
    int card_id = getSubcards().first();
    const Card *card = Sanguosha->getCard(card_id);
    Slash *slash = new Slash(card->getSuit(), card->getNumber());
    slash->setSkillName("liegong_pass");
    CardUseStruct use;
    use.card = slash;
    use.from = source;
    use.to = targets;
    room->useCard(use,false);
}


class LiegongPassMark: public TriggerSkill{
public:
    LiegongPassMark():TriggerSkill("#liegong_pass_mark"){
        events << SlashEffect << PhaseChange;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *huangzhong, QVariant &data) const{
        Room *room = huangzhong->getRoom() ;
        if(event == SlashEffect){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            room->setPlayerMark(huangzhong,"liegong_mark",effect.slash->getEffectiveId());
            if(effect.slash->getSkillName() == "liegong_pass"){
                room->playSkillEffect("liegong");
                foreach(const Card *card, effect.to->getEquips()){
                    if(card->getSuit() == effect.slash->getSuit())
                        room->throwCard(card->getEffectiveId());
                }
                room->slashResult(effect, NULL);
                return true;
            }
        }else if(event == PhaseChange && huangzhong->getPhase() == Player::Finish){
            room->setPlayerMark(huangzhong,"liegong_mark",0);
        }
        return false;

    }
};


class GongshenPass: public TriggerSkill{
public:
    GongshenPass():TriggerSkill("gongshen_pass"){
        events << SlashHit;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *huangzhong, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(huangzhong->getMark("@gongshen") < 10){
            int n = 1 ;
            if(effect.slash->isRed())
                n ++;
            if(effect.slash->getNature() != DamageStruct::Normal)
                n ++ ;
            huangzhong->gainMark("@gongshen",n);
        }
        return false;
    }
};


class JianhunPass: public TriggerSkill{
public:
    JianhunPass():TriggerSkill("jianhun_pass"){
        events << CardUsed;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *huangzhong, QVariant &data) const{
        Room *room = huangzhong->getRoom();
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Slash") && huangzhong->getMark("@gongshen") >= huangzhong->getHp()){
            huangzhong->loseMark("@gongshen", qMin(huangzhong->getMark("@gongshen"),2));
            huangzhong->drawCards(1);
            LogMessage log;
            log.type = "#JianhunPass";
            log.from = huangzhong;
            room->sendLog(log);
        }
        return false;
    }
};

class JianxiongPass:public MasochismSkill{
public:
    JianxiongPass():MasochismSkill("jianxiong_pass"){
    }

    virtual void onDamaged(ServerPlayer *caocao, const DamageStruct &damage) const{
        Room *room = caocao->getRoom();
        const Card *card = damage.card;
        caocao->gainMark("@jianxiong",damage.damage);
        if(damage.card){
            QVariant data = QVariant::fromValue(card);
            if(room->askForSkillInvoke(caocao, "jianxiong_pass", data)){
                if(room->obtainable(card, caocao)){
                    room->playSkillEffect("jianxiong");
                        caocao->obtainCard(card);
                }
            }else{
                caocao->drawCards(1);
            }
        }
    }
};

class Xietian: public TriggerSkill{
public:
    Xietian():TriggerSkill("xietian"){
        frequency = Frequent;
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *caocao, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        const Card *reason = damage.card;
        if(reason == NULL)
            return false;
        Room *room = caocao->getRoom();
        if(reason->inherits("AOE") && room->askForSkillInvoke(caocao, objectName(), data)){
            LogMessage log;
            log.from = caocao;
            log.to << damage.to;
            if(!damage.to->isKongcheng()){
                const Card *card = damage.to->getRandomHandCard();
                room->moveCardTo(card, caocao, Player::Hand, false);
                log.type = "#Xietian1";
            }else{
                damage.damage ++;
                data = QVariant::fromValue(damage);
                log.type = "#Xietian2";
                log.arg = damage.card->getName() ;
                log.arg2 = damage.damage ;
            }
            caocao->getRoom()->sendLog(log);
        }
        return false;
    }
};

class BadaoPass:public OneCardViewAsSkill{
public:
    BadaoPass():OneCardViewAsSkill("badao_pass"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@jianxiong") >= 3 ;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card ;
        const Card *sub_card = card_item->getFilteredCard() ;
        if(sub_card->isRed()){
            card = new ArcheryAttack(sub_card->getSuit(), sub_card->getNumber());
        }else{
            card = new SavageAssault(sub_card->getSuit(), sub_card->getNumber());
        }
        card->addSubcard(sub_card);
        card->setSkillName(objectName());
        return card;
    }
};

class BadaoCost: public TriggerSkill{
public:
    BadaoCost():TriggerSkill("#badao_cost"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *caocao, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->getSkillName() == "badao_pass"){
            caocao->loseMark("@jianxiong",3);
        }
        return false ;
    }
};

class Langgu: public PhaseChangeSkill{
public:
    Langgu():PhaseChangeSkill("langgu"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *simayi) const{
        if(simayi->getPhase() == Player::Finish){
            Room *room = simayi->getRoom();
            int n = 0 ;
            foreach(ServerPlayer *player, room->getAlivePlayers()){
                n = qMax(n,player->getHandcardNum());
            }
            if(n - simayi->getHandcardNum() > 2 && room->askForSkillInvoke(simayi, objectName())){
                simayi->drawCards(2);
            }
        }
        return false;
    }
};


class FankuiPass:public MasochismSkill{
public:
    FankuiPass():MasochismSkill("fankui_pass"){

    }

    virtual void onDamaged(ServerPlayer *simayi, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = simayi->getRoom();
        QVariant data = QVariant::fromValue(from);
        if(from && from->isAlive() && room->askForSkillInvoke(simayi, "fankui", data)){
            int x = damage.damage, i;
            for(i=0; i<x; i++){
                if(!from->isNude()){
                    int card_id = room->askForCardChosen(simayi, from, "he", "fankui");
                    if(room->getCardPlace(card_id) == Player::Hand)
                        room->moveCardTo(Sanguosha->getCard(card_id), simayi, Player::Hand, false);
                    else
                        room->obtainCard(simayi, card_id);
                }else{
                    DamageStruct damage;
                    damage.from = simayi;
                    damage.to = from;
                    room->damage(damage);
                    break ;
                }
            }
            room->playSkillEffect("fankui");
        }
    }
};

class GangliePass:public MasochismSkill{
public:
    GangliePass():MasochismSkill("ganglie_pass"){

    }

    virtual void onDamaged(ServerPlayer *xiahou, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = xiahou->getRoom();
        QVariant source = QVariant::fromValue(from);

        if(from && from->isAlive() && room->askForSkillInvoke(xiahou, "ganglie", source)){
            room->playSkillEffect("ganglie");

            JudgeStruct judge;
            QString num_str = QString("A23456789").left(xiahou->getHp());
            judge.pattern = QRegExp(QString("(.*):(.*):([%1])").arg(num_str)) ;
            judge.good = false;
            judge.reason = objectName();
            judge.who = xiahou;
            int x = damage.damage, i;
            for(i=0; i<x; i++){
                room->judge(judge);
                if(judge.isGood()){
                    if(!room->askForDiscard(from, "ganglie", 2, true)){
                        DamageStruct damage;
                        damage.from = xiahou;
                        damage.to = from;
                        room->damage(damage);
                    }
                }
                if(judge.card->isRed())
                    xiahou->obtainCard(judge.card);
            }
        }
    }
};

class JueduanPass: public TriggerSkill{
public:
    JueduanPass():TriggerSkill("jueduan_pass"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *xiahou, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") && damage.to->isAlive() && !damage.to->isNude()){
            Room *room = xiahou->getRoom();
            if(room->askForSkillInvoke(xiahou, objectName(),QVariant::fromValue(damage.to))){
                int card_id = room->askForCardChosen(xiahou, damage.to, "he", objectName());
                room->throwCard(card_id);
            }
        }
        return false;
    }
};


class YijiPass:public TriggerSkill{
public:
    YijiPass():TriggerSkill("yiji_pass"){
        frequency = Frequent;
        events << Predamaged << Damaged << AskForPeachesDone;
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *guojia, QVariant &data) const{
        if(event == Predamaged){
            if(guojia->hasFlag("yiji_dying"))
                guojia->setFlags("-yiji_dying");
        }else if(event == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(guojia->isAlive()){
                Room *room = guojia->getRoom();

                if(!room->askForSkillInvoke(guojia, objectName()))
                    return false;

                room->playSkillEffect(objectName());
                int n = damage.damage * (guojia->hasFlag("yiji_dying") ? 3 : 2);
                guojia->drawCards(n);
                guojia->setFlags("-yiji_dying");
            }
        }else if(event == AskForPeachesDone){
            if(guojia->getHp() > 0)
                guojia->setFlags("yiji_dying");
        }
        return false;
    }
};

class Guimou:public TriggerSkill{
public:
    Guimou():TriggerSkill("guimou"){
        events << FinishJudge;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true ;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *guojia = room->findPlayerBySkillName(objectName());

        if(guojia == NULL)
            return false;

        JudgeStar judge = data.value<JudgeStar>();
        if(judge->card->isRed() && judge->card->getNumber() > guojia->getHp() + guojia->getHandcardNum() && guojia->askForSkillInvoke(objectName(), data)){
            LogMessage log;
            log.type = "#Guimou";
            log.from = player;
            log.to << guojia;
            room->sendLog(log);
            guojia->drawCards(1);
        }
        return false;
    }
};

class JinghongPass: public TriggerSkill{
public:
    JinghongPass():TriggerSkill("jinghong_pass"){
        frequency = Frequent;
        events << CardLost << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhenji, QVariant &data) const{
        if(event == CardLost){
            if(zhenji->getPhase() != Player::Discard)
                return false;
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->to_place == Player::DiscardedPile){
                zhenji->addMark("jinghong_pass");
            }
        }else if(event == PhaseChange){
            if(zhenji->getPhase() != Player::Finish)
                return false;
            if(zhenji->getMark("jinghong_pass") >= qMax(1,zhenji->getHp())){
                Room *room = zhenji->getRoom();
                if(zhenji->askForSkillInvoke(objectName())){
                    QList<int> cards = room->getNCards(zhenji->getMark("jinghong_pass"));

                    QList<int> black_cards;
                    foreach(int card_id, cards){
                        const Card *card = Sanguosha->getCard(card_id);
                        if(card->isBlack())
                            black_cards << card_id;
                    }

                    room->fillAG(cards, zhenji);

                    while(true){
                        int card_id;
                        if(zhenji->getState() == "robot" || zhenji->getState() == "offline" || zhenji->getState() == "trust"){
                            if(black_cards.empty())
                                break;
                             card_id = black_cards.at(qrand() % black_cards.length()) ;
                        }else{
                            card_id = room->askForAG(zhenji, black_cards, true, "jinghong");
                        }

                        if(card_id == -1)
                            break;

                        if(!black_cards.contains(card_id))
                            continue;

                        black_cards.removeOne(card_id);

                        zhenji->obtainCard(Sanguosha->getCard(card_id));
                        room->showCard(zhenji, card_id);
                        break;
                    }

                    zhenji->invoke("clearAG");
                }
            }
            zhenji->setMark("jinghong_pass", 0);
        }

        return false;
    }
};

class LuoyiPass:public ViewAsSkill{
public:
    LuoyiPass():ViewAsSkill("luoyi_pass"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 2){
            return false ;
        }else if(selected.length() == 1){
            return ! (selected.first()->isEquipped() || to_select->isEquipped()) ;
        }else{
            return true ;
        }
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;
        if(cards.length()== 1 && !cards.first()->getCard()->inherits("EquipCard"))
            return NULL;
        LuoyiPassCard *luoyi_pass_card = new LuoyiPassCard;
        luoyi_pass_card->addSubcards(cards);

        return luoyi_pass_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("LuoyiPassCard");
    }
};


LuoyiPassCard::LuoyiPassCard(){
    target_fixed = true;
    once = true;
}

void LuoyiPassCard::use(Room *room, ServerPlayer *xuchu, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    xuchu->setFlags("luoyi");
    room->playSkillEffect("luoyi");
}



class Guantong: public TriggerSkill{
public:
    Guantong():TriggerSkill("guantong"){
        events << DamageComplete;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.from == NULL)
            return false;

        if(!damage.from->hasSkill(objectName()))
            return false;

        if(!damage.card || (!damage.card->inherits("Slash") && !damage.card->inherits("Duel")))
            return false;

        ServerPlayer *xuchu = damage.from;
        Room *room = xuchu->getRoom();
        if(!xuchu->isKongcheng() && xuchu->askForSkillInvoke(objectName(), data)){
            if(room->askForDiscard(xuchu, objectName(), 1 , true)){
                room->playSkillEffect(objectName());
                xuchu->setFlags("guantong_used");
                damage.to = player->getNextAlive();
                damage.card = NULL;
                damage.damage = 1;
                damage.nature = DamageStruct::Normal ;
                room->damage(damage);
            }
        }else{
            xuchu->drawCards(1);
        }
        return false;
    }
};


class TuxiPassViewAsSkill: public ZeroCardViewAsSkill{
public:
    TuxiPassViewAsSkill():ZeroCardViewAsSkill("tuxi_pass"){
    }

    virtual const Card *viewAs() const{
        return new TuxiPassCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@tuxi_pass";
    }
};

class TuxiPass:public PhaseChangeSkill{
public:
    TuxiPass():PhaseChangeSkill("tuxi_pass"){
        view_as_skill = new TuxiPassViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *zhangliao) const{
        if(zhangliao->getPhase() == Player::Draw){
            Room *room = zhangliao->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(zhangliao);
            foreach(ServerPlayer *player, other_players){
                if(!player->isNude()){
                    can_invoke = true;
                    break;
                }
            }

            if(can_invoke && room->askForUseCard(zhangliao, "@@tuxi_pass", "@tuxi-card"))
                return true;
        }

        return false;
    }
};

TuxiPassCard::TuxiPassCard(){
}

bool TuxiPassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;

    if(to_select == Self)
        return false;

    return !to_select->isNude();
}

void TuxiPassCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    int card_id = room->askForCardChosen(effect.from, effect.to, "he", "tuxi");
    const Card *card = Sanguosha->getCard(card_id);
    room->moveCardTo(card, effect.from, Player::Hand, false);

    room->setEmotion(effect.to, "bad");
    room->setEmotion(effect.from, "good");
}

void TuxiPassCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
   room->playSkillEffect("tuxi");
   foreach(ServerPlayer *target, targets){
       CardEffectStruct effect;
       effect.card = this;
       effect.from = source;
       effect.to = target;
       effect.multiple = true;

       room->cardEffect(effect);
   }
   if(targets.length() < 2)
       source->drawCards(2-targets.length());
}


class XingshangPass: public TriggerSkill{
public:
    XingshangPass():TriggerSkill("xingshang_pass"){
        frequency = Frequent;
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        if(player->isNude())
            return false;

        Room *room = player->getRoom();
        ServerPlayer *caopi = room->findPlayerBySkillName(objectName());
        if(caopi && caopi->isAlive() && room->askForSkillInvoke(caopi, objectName())){
            int effect_type ;
            if(player->isCaoCao()){
                effect_type = 3;
            }else if(player->getGeneral()->isMale())
                effect_type = 1;
            else
               effect_type = 2;

            QString result = room->askForChoice(caopi,objectName(),"gethe+draw+cancel") ;
            if(result == "gethe"){
                room->playSkillEffect(objectName(), effect_type);
                caopi->obtainCard(player->getWeapon());
                caopi->obtainCard(player->getArmor());
                caopi->obtainCard(player->getDefensiveHorse());
                caopi->obtainCard(player->getOffensiveHorse());

                DummyCard *all_cards = player->wholeHandCards();
                if(all_cards){
                    room->moveCardTo(all_cards, caopi, Player::Hand, false);
                    delete all_cards;
                }
            }else if(result == "draw"){
                room->playSkillEffect(objectName(), effect_type);
                caopi->drawCards(qMax(player->getHandcardNum(),player->getEquips().length()));
            }
        }
        return false;
    }
};

FangzhuPassCard::FangzhuPassCard(){
    mute = true;
}
bool FangzhuPassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() ;
}

void FangzhuPassCard::onEffect(const CardEffectStruct &effect) const{
    int x = effect.to->getLostHp();
    effect.to->drawCards(x);

    Room *room = effect.to->getRoom();

    int index;
    if(effect.to->faceUp())
        index = effect.to->getGeneralName() == "caozhi" ? 3 : 1;
    else
        index = 2;
    room->playSkillEffect("fangzhu", index);

    effect.to->turnOver();
}

class FangzhuPassViewAsSkill: public ZeroCardViewAsSkill{
public:
    FangzhuPassViewAsSkill():ZeroCardViewAsSkill("fangzhu_pass"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@fangzhu_pass";
    }

    virtual const Card *viewAs() const{
        return new FangzhuPassCard;
    }
};

class FangzhuPass: public MasochismSkill{
public:
    FangzhuPass():MasochismSkill("fangzhu_pass"){
        view_as_skill = new FangzhuPassViewAsSkill;
    }

    virtual void onDamaged(ServerPlayer *caopi, const DamageStruct &damage) const{
        Room *room = caopi->getRoom();
        room->askForUseCard(caopi, "@@fangzhu_pass", "@fangzhu_pass");
    }
};

class FanzhiPass: public TriggerSkill{
public:
    FanzhiPass():TriggerSkill("fanzhi_pass"){
        events << TurnStart << Pindian;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *caopi, QVariant &data) const{
        if(event == TurnStart){
            if(caopi->faceUp() || caopi->isKongcheng())
                return false;
            Room *room = caopi->getRoom();
            QList<ServerPlayer *> targets;
            foreach(ServerPlayer *target, room->getOtherPlayers(caopi)){
                if(!target->isKongcheng())
                    targets << target;
            }
            if(!targets.isEmpty() && room->askForSkillInvoke(caopi, objectName(), data)){
                ServerPlayer *target = room->askForPlayerChosen(caopi,targets,objectName());
                if(target)
                    caopi->pindian(target, "fanzhi_pass");
            }
            return false;
        }else {
            PindianStar pindian = data.value<PindianStar>();
            if(pindian->reason == "fanzhi_pass" && pindian->isSuccess()){
                pindian->from->turnOver();
                pindian->to->turnOver();
            }
        }
        return false;
    }
};

class ZhihengPass:public ViewAsSkill{
public:
    ZhihengPass():ViewAsSkill("zhiheng_pass"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *) const{
        if(Self->hasUsed("ZhihengPassCard")){
            return selected.length() < Self->getMark("@zhiba") ;
        }else
            return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        ZhihengPassCard *zhiheng_pass_card = new ZhihengPassCard;
        zhiheng_pass_card->addSubcards(cards);

        return zhiheng_pass_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ZhihengPassCard") || player->getMark("@zhiba") > 0 ;
    }
};


ZhihengPassCard::ZhihengPassCard(){
    target_fixed = true;
}

void ZhihengPassCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->playSkillEffect("zhiheng");
    room->throwCard(this);
    bool all_same = true;
    int number = 0 ;
    foreach(int subcard, subcards){
        const Card *card = Sanguosha->getCard(subcard);
        if(number != 0 && number != card->getNumber()){
            all_same = false ;
            break ;
        }
        number = card->getNumber() ;
    }
    int n = subcards.length() ;
    if(source->usedTimes("ZhihengPassCard") > 1)
        source->loseMark("@zhiba",n);
    if(all_same){
        source->gainMark("@zhiba", n - 1);
        n = n * 2 - 1 ;
    }
    source->drawCards(n);
}

class FanjianPass: public OneCardViewAsSkill{
public:
    FanjianPass():OneCardViewAsSkill("fanjian_pass"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("FanjianPassCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        FanjianPassCard *fanjian_pass_card = new FanjianPassCard;
        fanjian_pass_card->addSubcard(card_item->getCard()->getId());

        return fanjian_pass_card;
    }
};

FanjianPassCard::FanjianPassCard(){
    will_throw = false ;
}

void FanjianPassCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhouyu = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = zhouyu->getRoom();
    room->playSkillEffect("fanjian");

    int card_id = getSubcards().first();
    const Card *card = Sanguosha->getCard(card_id);
    Card::Suit suit = room->askForSuit(target);

    LogMessage log;
    log.type = "#ChooseSuit";
    log.from = target;
    log.arg = Card::Suit2String(suit);
    room->sendLog(log);

    room->showCard(zhouyu, card_id);
    room->getThread()->delay();

    if(card->getSuit() != suit){
        DamageStruct damage;
        damage.card = NULL;
        damage.from = zhouyu;
        damage.to = target;

        room->damage(damage);

        if(target->isAlive()){
            target->obtainCard(card);
        }else{
            zhouyu->obtainCard(card);
        }
    }else{
        zhouyu->obtainCard(card);
    }
}


class QuwuPassViewAsSkill: public ViewAsSkill{
public:
    QuwuPassViewAsSkill():ViewAsSkill(""){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@quwu_pass";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 1)
            return false;

        if(!to_select->getCard()->isEquipped())
            return false;

        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        DummyCard *card = new DummyCard;
        card->setSkillName(objectName());
        card->addSubcards(cards);
        return card;
    }
};

class QuwuPass: public TriggerSkill{
public:
    QuwuPass():TriggerSkill("quwu_pass"){
        view_as_skill = new QuwuPassViewAsSkill;
        events << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && ! target->getEquips().empty() ;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        const Card *card = room->askForCard(player, "@quwu_pass", "@quwu_pass");
        if(card){
            room->throwCard(card->getEffectiveId());
            LogMessage log;
            log.type = "#QuwuPass";
            log.from = player;
            room->sendLog(log);

            damage.damage--;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class KurouPass: public ZeroCardViewAsSkill{
public:
    KurouPass():ZeroCardViewAsSkill("kurou_pass"){

    }

    virtual const Card *viewAs() const{
        return new KurouPassCard;
    }
};

KurouPassCard::KurouPassCard(){
    target_fixed = true;
}

void KurouPassCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->loseHp(source);
    int n = 2;
    if(source->usedTimes("KurouPassCard") == 1)
        n += 1;
    if(source->isAlive())
        room->drawCards(source, n);
}

class Zhaxiang:public ViewAsSkill{
public:
    Zhaxiang():ViewAsSkill("zhaxiang"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->isWounded() && !player->hasUsed("ZhaxiangCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 4 ;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 4){
            ZhaxiangCard *zhaxiang_card = new ZhaxiangCard();
            zhaxiang_card->addSubcards(cards);
            return zhaxiang_card;
        }else
            return NULL;
    }
};

ZhaxiangCard::ZhaxiangCard(){
    once = true;
    target_fixed = true;
}

void ZhaxiangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    RecoverStruct recover;
    recover.card = this;
    recover.who = source;
    source->getRoom()->recover(source, recover);
}


class KejiPass: public PhaseChangeSkill{
public:
    KejiPass():PhaseChangeSkill("keji_pass"){
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Discard;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        return player->getHandcardNum() < 13;
    }
};


class Baiyi:public TriggerSkill{
public:
    Baiyi():TriggerSkill("baiyi"){
        events << PhaseChange;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *lumeng, QVariant &data) const{
        if(lumeng->getPhase() == Player::Start){
            Room *room = lumeng->getRoom();
            if(lumeng->getEquips().empty() && room->askForSkillInvoke(lumeng, objectName())){
                lumeng->drawCards(1);
            }
        }
        return false;
    }
};

class DujiangPass:public TriggerSkill{
public:
    DujiangPass():TriggerSkill("dujiang_pass"){
        events << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *lumeng, QVariant &data) const{
        if(lumeng->getPhase() == Player::Finish){
            Room *room = lumeng->getRoom();
            int n = lumeng->getEquips().length();
            if(n > 0 && room->askForSkillInvoke(lumeng, objectName())){
                if(n == 1){
                    // lumeng->drawCards(1);
                }else if(n == 2){
                    lumeng->drawCards(2);
                }else if(n == 3){
                    QList<ServerPlayer *> players = room->getOtherPlayers(lumeng);
                    int count = 0 ;
                    foreach(ServerPlayer *p, players){
                        if(! p->isNude()){
                            int card_id = room->askForCardChosen(lumeng, p, "he", "dujiang");
                            room->throwCard(card_id);
                            count++;
                        }
                    }
                    if(count < 4)
                        lumeng->drawCards(4-count);
                }else if(n == 4){
                    QList<ServerPlayer *> players = room->getOtherPlayers(lumeng);
                    int count = 0 ;
                    foreach(ServerPlayer *p, players){
                        room->loseHp(p);
                        p->throwAllEquips();
                        count++;
                    }
                    RecoverStruct recover;
                    recover.who = lumeng;
                    recover.recover = lumeng->getLostHp();
                    room->recover(lumeng,recover);
                }
                lumeng->throwAllEquips();
            }
        }
        return false;
    }
};


class LianyingPass: public TriggerSkill{
public:
    LianyingPass():TriggerSkill("lianying_pass"){
        events << CardUsed << CardResponsed << CardLost;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *luxun, QVariant &data) const{
        Room *room = luxun->getRoom();
        if(event == CardUsed || event == CardResponsed){
            if(luxun->getPhase() != Player::NotActive)
                return false;
            CardStar card = NULL;
            if(event == CardUsed){
                CardUseStruct use = data.value<CardUseStruct>();
                card = use.card;
            }else if(event == CardResponsed)
                card = data.value<CardStar>();
            if(card == NULL || card->isVirtualCard())
                return false;

            if(luxun->askForSkillInvoke(objectName(), data))
                luxun->drawCards(1);
        }else if(event == CardLost){
            if(luxun->isKongcheng()){
                CardMoveStar move = data.value<CardMoveStar>();
                if(move->from_place == Player::Hand){
                    if(room->askForSkillInvoke(luxun, objectName())){
                        room->playSkillEffect("lianying");
                        luxun->drawCards(1);
                    }
                }
            }
        }
        return false;
    }
};


class JieyinPass: public OneCardViewAsSkill{
public:
    JieyinPass():OneCardViewAsSkill("jieyin_pass"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("JieyinPassCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        JieyinPassCard *jieyin_pass_card = new JieyinPassCard;
        jieyin_pass_card->addSubcard(card_item->getCard()->getId());

        return jieyin_pass_card;
    }
};

JieyinPassCard::JieyinPassCard(){
}

bool JieyinPassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return to_select->getGeneral()->isMale() && to_select->isWounded();
}

void JieyinPassCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;

    room->recover(effect.from, recover, true);
    room->recover(effect.to, recover, true);

    room->playSkillEffect("jieyin");
    if(!effect.to->isNude()){
        int card_id = room->askForCardChosen(effect.from, effect.to, "he", "jieyin_pass");
        const Card *card = Sanguosha->getCard(card_id);
        room->moveCardTo(card, effect.from, Player::Hand, false);
    }
}

class JinguoPass: public TriggerSkill{
public:
    JinguoPass():TriggerSkill("jinguo_pass"){
        events << CardLost;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *sunshangxiang, QVariant &data) const{
        CardMoveStar move = data.value<CardMoveStar>();
        const Card *card = Sanguosha->getCard(move->card_id);
        if(move->to_place == Player::Equip && (card->inherits("Armor") || card->inherits("DefensiveHorse")) ){
            Room *room = sunshangxiang->getRoom();
            if(room->askForSkillInvoke(sunshangxiang, objectName())){
                while(true){
                    int card_id = room->drawCard();
                    room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::Special, true);
                    room->getThread()->delay();
                    const Card *card = Sanguosha->getCard(card_id);
                    room->obtainCard(sunshangxiang, card_id);
                    if(!card->inherits("EquipCard"))
                        break;
                }
            }
        }
        return false;
    }
};

class Yuyue: public OneCardViewAsSkill{
public:
    Yuyue():OneCardViewAsSkill("yuyue"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YuyueCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        YuyueCard *yuyue_card = new YuyueCard;
        yuyue_card->addSubcard(card_item->getCard()->getId());

        return yuyue_card;
    }
};


YuyueCard::YuyueCard(){
}

bool YuyueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return ! to_select->getJudgingArea().empty() ;
}

void YuyueCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("yuyue");
    for(int i=0;i<effect.to->getJudgingArea().length();i++){
        CardUseStruct card_use;
        card_use.card = slash;
        card_use.from = effect.from;
        card_use.to << effect.to;
        room->useCard(card_use, false);
    }
}


class Shuangxing: public PhaseChangeSkill{
public:
    Shuangxing():PhaseChangeSkill("shuangxing"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *daqiao) const{
        if(daqiao->getPhase() == Player::Start || daqiao->getPhase() == Player::Finish){
            Room *room = daqiao->getRoom();
            int n = 0 ;
            foreach(ServerPlayer *player, room->getAlivePlayers()){
                n = qMax(n,player->getJudgingArea().length());
            }
            if(n > 0 && room->askForSkillInvoke(daqiao, objectName())){
                daqiao->drawCards(n);
            }
        }

        return false;
    }
};


class TongjiPass: public TriggerSkill{
public:
    TongjiPass():TriggerSkill("tongji_pass"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *ganning, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = damage.to->getRoom();
        if(damage.card && damage.card->inherits("Slash") && damage.to->isKongcheng() && !ganning->isKongcheng() && ganning->askForSkillInvoke(objectName(), QVariant::fromValue(damage.to)) ){
            if(room->askForDiscard(ganning, "tongji", 1 , true)){
                LogMessage log;
                log.type = "#TongjiPass";
                log.from = ganning;
                log.to << damage.to;
                log.arg = QString::number(damage.damage);
                log.arg2 = QString::number(damage.damage + 1);
                room->sendLog(log);

                damage.damage ++;
                data = QVariant::fromValue(damage);
            }
        }

        return false;
    }
};

class JieluePass: public TriggerSkill{
public:
    JieluePass():TriggerSkill("jielue_pass"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *ganning, QVariant &data) const{
        const Card *card = NULL;
        CardUseStruct use = data.value<CardUseStruct>();
        card = use.card;
        if(card && card->inherits("Dismantlement") && card->getSuit() == Card::Spade){
            ServerPlayer *to = use.to.first();
            Room *room = ganning->getRoom();
            if(!to->getEquips().empty() && ganning->askForSkillInvoke(objectName(), QVariant::fromValue(to))){
                int card_id = room->askForCardChosen(ganning, to, "e", objectName());
                const Card *card = Sanguosha->getCard(card_id);
                room->moveCardTo(card, ganning, Player::Hand, false);
            }
        }
        return false;
    }
};




class ZhanshenPass:public OneCardViewAsSkill{
public:
    ZhanshenPass():OneCardViewAsSkill("zhanshen_pass"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "slash";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        return (card->inherits("TrickCard") || card->inherits("EquipCard")) && !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Card *slash = new Slash(card->getSuit(), card->getNumber());
        slash->addSubcard(card->getId());
        slash->setSkillName(objectName());
        return slash;
    }
};


class NuozhanPass : public TriggerSkill{
public:
    NuozhanPass():TriggerSkill("nuozhan_pass"){
        events << CardResponsed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
           return false;

        CardStar card_star = data.value<CardStar>();
        if(!card_star->inherits("Jink"))
            return false;

        if(!player->isKongcheng() && player->askForSkillInvoke("nuozhan_pass"))
            player->getRoom()->askForUseCard(player, "slash", "yitian-slash");

        return false;
    }
};


class LijianPass: public OneCardViewAsSkill{
public:
    LijianPass():OneCardViewAsSkill("lijian_pass"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("LijianPassCard");
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LijianPassCard *lijian_pass_card = new LijianPassCard;
        lijian_pass_card->addSubcard(card_item->getCard()->getId());

        return lijian_pass_card;
    }
};

LijianPassCard::LijianPassCard(){
    once = true;
}

bool LijianPassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.isEmpty() && ((to_select->hasSkill("kongcheng") && to_select->isKongcheng()) || Self == to_select)){
        return false;
    }
    return true;
}

bool LijianPassCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void LijianPassCard::use(Room *room, ServerPlayer *, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *to = targets.at(0);
    ServerPlayer *from = targets.at(1);

    to->obtainCard(this);

    Duel *duel = new Duel(Card::NoSuit, 0);
    duel->setSkillName("lijian");
    duel->setCancelable(false);

    CardUseStruct use;
    use.from = from;
    use.to << to;
    use.card = duel;
    room->useCard(use);
}

class QingnangPass: public OneCardViewAsSkill{
public:
    QingnangPass():OneCardViewAsSkill("qingnang_pass"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QingnangPassCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        QingnangPassCard *qingnang_pass_card = new QingnangPassCard;
        qingnang_pass_card->addSubcard(card_item->getCard()->getId());

        return qingnang_pass_card;
    }
};


QingnangPassCard::QingnangPassCard(){
    once = true;
    target_fixed = true;
}

void QingnangPassCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    room->playSkillEffect("qingnang");
    if(source->isWounded()){
        RecoverStruct recover;
        recover.card = this;
        recover.who = source;
        source->getRoom()->recover(source, recover);
    }else{
        source->gainMark("qingnang_buff");
    }
}

class QingnangBuff: public PhaseChangeSkill{
public:
    QingnangBuff():PhaseChangeSkill("#qingnang"){
    }
    virtual bool onPhaseChange(ServerPlayer *huatuo) const{
        if(huatuo->getPhase() == Player::Start && huatuo->getMark("qingnang_buff") > 0){
            huatuo->loseMark("qingnang_buff");
            if(huatuo->isWounded()){
                RecoverStruct recover;
                recover.who = huatuo;
                huatuo->getRoom()->recover(huatuo, recover);
                LogMessage log;
                log.type = "#QingnangBuff";
                log.from = huatuo;
                huatuo->getRoom()->sendLog(log);
            }else{
                huatuo->drawCards(2);
            }
        }
        return false;
    }
};

class YangshenPass: public TriggerSkill{
public:
    YangshenPass():TriggerSkill("yangshen_pass"){
        frequency = Compulsory;
        events << HpRecover ;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *huatuo, QVariant &data) const{
        Room *room = huatuo->getRoom();
        RecoverStruct recover = data.value<RecoverStruct>();
        if(recover.card && (recover.card->inherits("Peach"))){
            LogMessage log;
            log.type = "#YangshenPass";
            log.from = huatuo;
            room->sendLog(log);
            RecoverStruct recover;
            recover.who = huatuo;
            room->recover(huatuo, recover);
        }
        return false;
    }
};


class MafeiPass:public MasochismSkill{
public:
    MafeiPass():MasochismSkill("mafei_pass"){

    }

    virtual void onDamaged(ServerPlayer *huatuo, const DamageStruct &damage) const{
        Room *room = huatuo->getRoom();
        ServerPlayer *from = damage.from;
        if(huatuo->getPhase() == Player::NotActive && from && from != huatuo && from->isAlive() && from->getMark("mafei_pass") == 0){
            LogMessage log;
            log.type = "#MafeiPass";
            log.from = huatuo;
            log.to.append(from);
            room->sendLog(log);
            from->addMark("mafei_pass");
        }
    }
};

class MafeiBuff: public PhaseChangeSkill{
public:
    MafeiBuff():PhaseChangeSkill("#mafei"){
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Finish && player->getMark("mafei_pass") > 0){
            if(player->getHandcardNum() > 0){
                if (player->getHandcardNum() == 1){
                    player->throwAllHandCards();
                }else{
                    room->askForDiscard(player, "mafei", 1);
                }
                player->setMark("mafei_pass", 0);
            }
        }
        return false;
    }
};



class GuhuoPass: public OneCardViewAsSkill{
public:
    GuhuoPass():OneCardViewAsSkill("guhuo_pass"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        int card_id = player->getMark("guhuo_mark");
        if(card_id > 0){
            const Card *card = Sanguosha->getCard(card_id);
            return card->isAvailable(player);
        }else
            return false;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        int card_id = Self->getMark("guhuo_mark");
        const Card *card = Sanguosha->getCard(card_id);
        return to_select->getFilteredCard()->getSuit() == card->getSuit();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        int card_id = Self->getMark("guhuo_mark");
        if(card_id == 0)
            return NULL;

        const Card *card = Sanguosha->getCard(card_id);
        const Card *orign = card_item->getFilteredCard();

        Card *new_card = Sanguosha->cloneCard(card->objectName(), orign->getSuit(), orign->getNumber());
        new_card->addSubcard(card_item->getCard());
        new_card->setSkillName(objectName());
        return new_card;
    }
};

class GuhuoPassMark: public TriggerSkill{
public:
    GuhuoPassMark():TriggerSkill("#guhuo_pass_mark"){
        events << CardUsed << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yuji, QVariant &data) const{
        Room *room = yuji->getRoom();
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card){
                if(!use.card->isVirtualCard() && (use.card->getTypeId() == Card::Basic || use.card->isNDTrick())){
                    room->setPlayerMark(yuji,"guhuo_mark",use.card->getId());
                }
                if(use.card->getSkillName() == "guhuo_pass")
                    room->setPlayerMark(yuji,"guhuo_mark",0);
            }
        }else if(event == PhaseChange && yuji->getPhase() == Player::Finish){
            room->setPlayerMark(yuji,"guhuo_mark",0);
        }
        return false ;
    }
};

class BuguaPass: public PhaseChangeSkill{
public:
    BuguaPass():PhaseChangeSkill("bugua_pass"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && !target->isKongcheng()
                && (target->getPhase() == Player::Judge || target->getPhase() == Player::Finish);
    }

    virtual bool onPhaseChange(ServerPlayer *yuji) const{
        if(yuji->hasFlag("bugua_used") || !yuji->askForSkillInvoke(objectName()))
            return false;
        if(yuji->getPhase() == Player::Start)
            yuji->setFlags("bugua_used");
        else if(yuji->getPhase() == Player::Finish)
            yuji->setFlags("-bugua_used");
        Room *room = yuji->getRoom();
        yuji->drawCards(1);

        const Card *card = room->askForCard(yuji,".NTH","@bugua_pass-card");
        if(!card)
            card = yuji->getRandomHandCard() ;

        room->moveCardTo(card, NULL, Player::DrawPile);

//        LogMessage log;
//        log.type = "#BuguaPass";
//        log.from = yuji;
//        room->sendLog(log);
        return false;
    }
};

class HuanshuPass: public TriggerSkill{
public:
    HuanshuPass():TriggerSkill("huanshu_pass"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *yuji, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.to->isAlive() && !damage.to->isKongcheng() && damage.to != yuji){
            Room *room = yuji->getRoom();
            if(room->askForSkillInvoke(yuji, objectName(), data)){
                QList<int> handcards_id;
                foreach(const Card *card, damage.to->getHandcards())
                    handcards_id << card->getEffectiveId();
                room->fillAG(handcards_id, yuji);
                int card_id = room->askForAG(yuji, handcards_id, true, objectName());
                room->broadcastInvoke("clearAG");
                if(card_id == -1 || yuji->isKongcheng())
                    return false;

                room->moveCardTo(Sanguosha->getCard(card_id), yuji, Player::Hand);
                const Card *card = room->askForCard(yuji,".NTH","@huanshu_pass-card");
                if(!card)
                    card = yuji->getRandomHandCard() ;
                room->moveCardTo(card, damage.to, Player::Hand);
            }
        }

        return false;
    }
};

class HuangtianPass : public TriggerSkill{
public:
    HuangtianPass():TriggerSkill("huangtian_pass"){
        events << CardFinished << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhangjiao, QVariant &data) const{
        if(zhangjiao->getPhase() != Player::NotActive)
            return false;
        Room *room = zhangjiao->getRoom() ;
        CardStar card = NULL;
        if(event == CardFinished){
            CardUseStruct card_use = data.value<CardUseStruct>();
            card = card_use.card;
        }else if(event == CardResponsed){
            card = data.value<CardStar>();
        }
        if(card && (card->isBlack() || card->inherits("GuidaoCard")) && room->askForSkillInvoke(zhangjiao, objectName(), data)){
            ServerPlayer *target = room->askForPlayerChosen(zhangjiao,room->getAlivePlayers(),objectName());
            if(target){
                bool chained = ! target->isChained();
                target->setChained(chained);
                room->broadcastProperty(target, "chained");
            }
        }
        return false;
    }
};

class LeijiPass: public TriggerSkill{
public:
    LeijiPass():TriggerSkill("leiji_pass"){
        events << CardFinished;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *zhangjiao, QVariant &data) const{
        if(zhangjiao->getPhase() == Player::NotActive)
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        Room *room = zhangjiao->getRoom();
        if(use.card && use.card->getSuit() == Card::Spade && (use.card->inherits("TrickCard") || use.card->inherits("EquipCard")) && room->askForSkillInvoke(zhangjiao, objectName(), data)){
            ServerPlayer *target = room->askForPlayerChosen(zhangjiao,room->getOtherPlayers(zhangjiao),objectName());
            if(target){
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(spade):(.*)");
                judge.good = false;
                judge.reason = "leiji";
                judge.who = target;
                room->judge(judge);

                DamageStruct damage;
                damage.card = NULL;
                damage.from = zhangjiao;
                damage.nature = DamageStruct::Thunder;
                if(judge.isBad()){
                    damage.to = target;
                    room->damage(damage);
                }else if(judge.card->getSuit() == Card::Club){
                    damage.to = zhangjiao;
                    room->damage(damage);
                }
            }
        }
        return false;
    }
};

class DajiPass : public TriggerSkill{
public:
    DajiPass():TriggerSkill("daji_pass"){
        frequency = Frequent;
        events << Damage << Damaged;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *zhangjiao, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == damage.Thunder){
            zhangjiao->drawCards(damage.damage);
        }
        return false;
    }
};

class WushenPass: public TriggerSkill{
public:
    WushenPass():TriggerSkill("wushen_pass"){
        events << SlashEffect << SlashProceed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *shenguanyu, QVariant &data) const{
        Room *room = shenguanyu->getRoom();

        if(event == SlashEffect){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();

            switch(effect.slash->getSuit()){
            case Card::Heart:{
                if(shenguanyu->isWounded() && shenguanyu->askForSkillInvoke(objectName(), QVariant::fromValue(effect.to))){
                    room->playSkillEffect("wushen");
                    RecoverStruct recover;
                    recover.who = shenguanyu;
                    room->recover(shenguanyu, recover);
                }
                break;
            }
            case Card::Diamond:{
                if(shenguanyu->askForSkillInvoke(objectName(), QVariant::fromValue(effect.to))){
                    room->playSkillEffect("wushen");
                    shenguanyu->drawCards(1);
                }
                break;
            }
            case Card::Club:{
                if(effect.to && effect.to->isAlive() && !effect.to->isKongcheng() && shenguanyu->askForSkillInvoke(objectName(), QVariant::fromValue(effect.to))){
                    room->playSkillEffect("wushen");
                    room->askForDiscard(effect.to, objectName(), 1, false);
                }
                break;
            }
            case Card::Spade:{
                break;
            }
            default:
                break;
            }
        }else if(event == SlashProceed){
            if(data.canConvert<SlashEffectStruct>()){
                SlashEffectStruct effect = data.value<SlashEffectStruct>();

                if(shenguanyu->isAlive() && effect.slash->getSuit() == Card::Spade && shenguanyu->askForSkillInvoke(objectName(), QVariant::fromValue(effect.to))){
                    room->playSkillEffect("wushen");
                    room->slashResult(effect, NULL);
                    return true;
                }
            }
        }
        return false;
    }
};

class WuhunPass: public TriggerSkill{
public:
    WuhunPass():TriggerSkill("wuhun_pass"){
        events << Damaged << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(room->getAlivePlayers().length() == 1)
            return false;
        if(player->hasSkill(objectName())){
            if(event == Damaged){
                DamageStruct damage = data.value<DamageStruct>();
                ServerPlayer *from = damage.from;
                if(from && from != player && ! from->isKongcheng()){
                    room->showAllCards(from);
                    QList<const Card *> black_cards;
                    foreach(const Card *card, from->getHandcards()){
                        if(card->isBlack())
                            black_cards << card ;
                    }
                    if(!black_cards.isEmpty()){
                        int index = qrand() % black_cards.length();
                        room->throwCard(black_cards.at(index)->getEffectiveId());

                        LogMessage log;
                        log.type = "#WuhunPassThrow";
                        log.from = player;
                        log.to << from;

                        if(black_cards.length() >= from->getHp()){
                            DamageStruct damage;
                            damage.from = from;
                            damage.to = from;
                            room->damage(damage);
                            log.type = "#WuhunPassThrowDamage";
                        }

                        room->sendLog(log);
                    }
                }
            }
        }else{
            if(event == Death){
                ServerPlayer *shenguanyu = room->findPlayerBySkillName(objectName());
                if(shenguanyu != NULL){
                    LogMessage log;
                    log.type = "#WuhunPassDraw";
                    log.from = player;
                    log.to << shenguanyu;
                    log.arg = QString::number(player->getMaxHP());
                    room->sendLog(log);
                    shenguanyu->drawCards(player->getMaxHP());
                }
            }
        }
        return false;
    }
};

class QishenPass: public DistanceSkill{
public:
    QishenPass():DistanceSkill("qishen_pass"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        if(from->hasSkill(objectName()))
            correct --;
        if(to->hasSkill(objectName()))
            correct ++;
        return correct;
    }
};

class SheliePass:public PhaseChangeSkill{
public:
    SheliePass():PhaseChangeSkill("shelie_pass"){
        frequency = Frequent;
        view_as_skill = new TuxiPassViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *shenlumeng) const{
        if(shenlumeng->getPhase() == Player::Draw){
            Room *room = shenlumeng->getRoom();

            if(shenlumeng->askForSkillInvoke(objectName())){
                shenlumeng->drawCards(2);
                room->playSkillEffect("shelie");
                int n = 0 ;
                while(true){
                    int card_id = room->drawCard();
                    room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::Special, true);
                    room->getThread()->delay();
                    const Card *card = Sanguosha->getCard(card_id);
                    if(card->getNumber() > n){
                        room->obtainCard(shenlumeng, card_id);
                        n = card->getNumber();
                    }else{
                        room->throwCard(card_id);
                        break;
                    }
                }
                return true;
            }
        }
        return false;
    }
};

PassModeItemCard::PassModeItemCard(){
    target_fixed = true;

    items["dunjiatianshu"]  = Warlock;
    items["qingnangshu"]    = Recover;
    items["rewardqibing"]   = Knight;
    items["qiangjian"]      = Strong;
    items["rewardyingzi"]   = Handsome;
    items["huifushu"]       = OnePeach;
}

void PassModeItemCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    QStringList choice = room->getTag("Reward").toStringList();
    choice.append("cancel");
    QString choose_item = room->askForChoice(source, "useitem", choice.join("+"));
    if(choose_item == "cancel")
        return;

    getItemEffect(room, source, choose_item);
    if(room->getTag("Reward").isNull())
        room->detachSkillFromPlayer(source, "useitem");
}

void PassModeItemCard::getItemEffect(Room *room, ServerPlayer *source, const QString &item_str) const{
    if(!items.keys().contains(item_str)){
        LogMessage log;
        log.type = "#ItemUnlock";
        room->sendLog(log);
    }
    else{
        LogMessage log;
        log.type = "#ItemUsed";
        log.from = source;
        log.arg = item_str;
        room->sendLog(log);

        switch(items[item_str]){
        case Warlock:{
                RecoverStruct recover;
                recover.who = source;
                recover.recover = source->getLostHp();
                room->recover(source, recover);
                source->throwAllHandCards();
                source->drawCards(source->getMaxHP());
                break;
            }
        case Recover:{
                ServerPlayer *target = room->askForPlayerChosen(source, room->getOtherPlayers(source), "useitem");
                if(target){
                    RecoverStruct recover;
                    recover.who = source;
                    room->recover(target, recover);
                    foreach(const Card *card, target->getHandcards())
                        room->moveCardTo(card, source, Player::Hand, false);
                }
                break;
            }
        case Knight:{
                int n = 0;
                foreach(ServerPlayer *p, room->getAlivePlayers()){
                    if(p->getDefensiveHorse()){
                        room->throwCard(p->getDefensiveHorse()->getEffectiveId());
                        n++;
                    }
                    if(p->getOffensiveHorse()){
                        room->throwCard(p->getOffensiveHorse()->getEffectiveId());
                        n++;
                    }
                }
                source->drawCards(n);
                break;
            }
        case Strong:{
                RecoverStruct recover;
                recover.who = source;
                recover.recover = 2;
                room->recover(source, recover);
                break;
            }
        case OnePeach:{
                RecoverStruct recover;
                recover.who = source;
                room->recover(source, recover);
                break;
            }
        case Handsome:{
                source->drawCards(2);
                ServerPlayer *target = room->askForPlayerChosen(source, room->getOtherPlayers(source), "useitem");
                if(target)
                    target->turnOver();
                break;
            }
        }
        QStringList reward_update = room->getTag("Reward").toStringList();
        reward_update.removeOne(item_str);
        if(reward_update.isEmpty())
            room->removeTag("Reward");
        else
            room->setTag("Reward", reward_update);
    }
}

class ItemUse: public ZeroCardViewAsSkill{
public:
    ItemUse(): ZeroCardViewAsSkill("useitem"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->isLord();
    }

    virtual const Card *viewAs() const{
        return new PassModeItemCard;
    }
};

void PassModeScenario::addGeneralAndSkills(){

    skills  << new Shiqi << new Qianggong << new Pojia << new ZhanshangPass << new Qishu << new Chenwen << new Zhongzhuang << new Yaoshu << new Jitian
                 << new LianzhanPass << new DouzhiPass
            << new RendePass << new JijiangPass << new JijiangCost << new ZhaoliePass << new WenjiuPass << new WenjiuBuff << new TuodaoPass << new BaonuPass
                 << new DuanhePass << new TiejiPass << new MashuPass << new GuanxingPass << new BeifaPass << new JizhiPass << new JumouPass << new ShipoPass
                 << new LongweiPass << new Longhou << new LiegongPass << new LiegongPassMark << new GongshenPass << new JianhunPass
            << new JianxiongPass << new Xietian << new BadaoPass << new BadaoCost << new FankuiPass << new Langgu << new GangliePass  << new JueduanPass
                << new YijiPass << new Guimou << new JinghongPass << new LuoyiPass << new Guantong  << new TuxiPass << new XingshangPass << new FangzhuPass
                << new FanzhiPass
            << new ZhihengPass << new FanjianPass << new QuwuPass << new KurouPass << new Zhaxiang << new KejiPass << new Baiyi << new DujiangPass
                << new LianyingPass << new JieyinPass << new JinguoPass << new TongjiPass << new JieluePass << new Yuyue << new Shuangxing
            << new ZhanshenPass << new NuozhanPass << new LijianPass << new QingnangPass << new QingnangBuff << new YangshenPass << new MafeiPass << new MafeiBuff
                << new GuhuoPass << new GuhuoPassMark << new BuguaPass << new HuanshuPass << new HuangtianPass << new LeijiPass << new DajiPass
            << new WushenPass << new WuhunPass << new QishenPass << new SheliePass
            << new Skill("nuhou") << new Skill("tipo") << new Skill("kezhi") << new Skill("fenjin") << new Quanheng << new Duanyan << new Xiongzi
                << new Qiangong
            << new ItemUse;

    related_skills.insertMulti("jijiang_pass", "#jijiang_cost");
    related_skills.insertMulti("wenjiu_pass", "#wenjiu");
    related_skills.insertMulti("liegong_pass", "#liegong_pass_mark");
    related_skills.insertMulti("qingnang_pass", "#qingnang");
    related_skills.insertMulti("mafei_pass", "#mafei");
    related_skills.insertMulti("badao_pass", "#badao_cost");
    related_skills.insertMulti("guhuo_pass", "#guhuo_pass_mark");

    General *shizu = new General(this,"shizu","evil",3, true, true);
    shizu->addSkill("shiqi");

    General *gongshou = new General(this,"gongshou","evil",3, false, true);
    gongshou->addSkill("zhengfeng");
    gongshou->addSkill("qianggong");

    General *jianwei = new General(this,"jianwei","evil",3, false, true);
    jianwei->addSkill("pojia");
    jianwei->addSkill("zhanshang_pass");

    General *qibing = new General(this,"qibing","evil",3, true, true);
    qibing->addSkill("qishu");

    General *huwei = new General(this,"huwei","evil",3, true, true);
    huwei->addSkill("chenwen");
    huwei->addSkill("zhongzhuang");

    General *yaodao = new General(this,"yaodao","evil",3, true, true);
    yaodao->addSkill("yaoshu");
    yaodao->addSkill("jitian");

    General *kuangdaoke = new General(this,"kuangdaoke","evil",3, true, true);
    kuangdaoke->addSkill("lianzhan_pass");
    kuangdaoke->addSkill("douzhi_pass");

    General *shenguanyu_p = new General(this, "shenguanyu_p", "evil_god", 5, true, true);
    shenguanyu_p->addSkill("wushen_pass");
    shenguanyu_p->addSkill("wuhun_pass");
    shenguanyu_p->addSkill("qishen_pass");

    General *shenlumeng_p = new General(this, "shenlumeng_p", "evil_god", 4, true, true);
    shenlumeng_p->addSkill("shelie_pass");
    shenlumeng_p->addSkill("gongxin");

    General *liubei_p = new General(this,"liubei_p","hero",4, true, true);
    liubei_p->addSkill("rende_pass");
    liubei_p->addSkill("jijiang_pass");
    liubei_p->addSkill("#jijiang_cost");
    liubei_p->addSkill("zhaolie_pass");

    General *guanyu_p = new General(this,"guanyu_p","hero",4, true, true);
    guanyu_p->addSkill("wusheng");
    guanyu_p->addSkill("wenjiu_pass");
    guanyu_p->addSkill("#wenjiu");
    guanyu_p->addSkill("tuodao_pass");

    General *zhangfei_p = new General(this,"zhangfei_p","hero",4, true, true);
    zhangfei_p->addSkill("paoxiao");
    zhangfei_p->addSkill("baonu_pass");
    zhangfei_p->addSkill("duanhe_pass");

    General *zhaoyun_p = new General(this,"zhaoyun_p","hero",4, true, true);
    zhaoyun_p->addSkill("longdan");
    zhaoyun_p->addSkill("longwei_pass");
    zhaoyun_p->addSkill("longhou");

    General *machao_p = new General(this,"machao_p","hero",4, true, true);
    machao_p->addSkill("tieji_pass");
    machao_p->addSkill("mashu_pass");

    General *zhugeliang_p = new General(this,"zhugeliang_p","hero",3, true, true);
    zhugeliang_p->addSkill("guanxing_pass");
    zhugeliang_p->addSkill("kongcheng");
    zhugeliang_p->addSkill("beifa_pass");

    General *huangyueying_p = new General(this,"huangyueying_p","hero",3, false, true);
    huangyueying_p->addSkill("jizhi_pass");
    huangyueying_p->addSkill("qicai");
    huangyueying_p->addSkill("shipo_pass");
    huangyueying_p->addSkill("jumou_pass");

    General *huangzhong_p = new General(this,"huangzhong_p","hero",4, true, true);
    huangzhong_p->addSkill("liegong_pass");
    huangzhong_p->addSkill("#liegong_pass_mark");
    huangzhong_p->addSkill("gongshen_pass");
    huangzhong_p->addSkill("jianhun_pass");

    General *caocao_p = new General(this,"caocao_p","hero",4, true, true);
    caocao_p->addSkill("jianxiong_pass");
    caocao_p->addSkill("xietian");
    caocao_p->addSkill("badao_pass");
    caocao_p->addSkill("#badao_cost");

    General *simayi_p = new General(this,"simayi_p","hero",3, true, true);
    simayi_p->addSkill("guicai");
    simayi_p->addSkill("fankui_pass");
    simayi_p->addSkill("langgu");

    General *xiahoudun_p = new General(this,"xiahoudun_p","hero",4, true, true);
    xiahoudun_p->addSkill("ganglie_pass");
    xiahoudun_p->addSkill("jueduan_pass");

    General *guojia_p = new General(this,"guojia_p","hero",3, true, true);
    guojia_p->addSkill("yiji_pass");
    guojia_p->addSkill("guimou");

    General *zhenji_p = new General(this,"zhenji_p","hero",3, false, true);
    zhenji_p->addSkill("luoshen");
    zhenji_p->addSkill("qingguo");
    zhenji_p->addSkill("jinghong_pass");

    General *xuchu_p = new General(this,"xuchu_p","hero",4, true, true);
    xuchu_p->addSkill("luoyi_pass");
    xuchu_p->addSkill("guantong");
    xuchu_p->addSkill("#luoyi");

    General *zhangliao_p = new General(this,"zhangliao_p","hero",4, true, true);
    zhangliao_p->addSkill("tuxi_pass");

    General *caopi_p = new General(this,"caopi_p","hero",3, true, true);
    caopi_p->addSkill("xingshang_pass");
    caopi_p->addSkill("fangzhu_pass");
    caopi_p->addSkill("fanzhi_pass");

    General *sunquan_p = new General(this,"sunquan_p","hero",4, true, true);
    sunquan_p->addSkill("zhiheng_pass");

    General *zhouyu_p = new General(this, "zhouyu_p", "hero", 3, true, true);
    zhouyu_p->addSkill("yingzi");
    zhouyu_p->addSkill("fanjian_pass");
    zhouyu_p->addSkill("quwu_pass");

    General *lumeng_p = new General(this, "lumeng_p", "hero", 4 ,true, true);
    lumeng_p->addSkill("keji_pass");
    lumeng_p->addSkill("baiyi");
    lumeng_p->addSkill("dujiang_pass");

    General *luxun_p = new General(this, "luxun_p", "hero", 3, true, true);
    luxun_p->addSkill("qianxun");
    luxun_p->addSkill("lianying_pass");

    General *ganning_p = new General(this, "ganning_p", "hero", 4 ,true, true);
    ganning_p->addSkill("qixi");
    ganning_p->addSkill("tongji_pass");
    ganning_p->addSkill("jielue_pass");

    General *huanggai_p = new General(this, "huanggai_p", "hero", 4 ,true, true);
    huanggai_p->addSkill("kurou_pass");
    huanggai_p->addSkill("zhaxiang");

    General *daqiao_p = new General(this, "daqiao_p", "hero", 3, false, true);
    daqiao_p->addSkill("guose");
    daqiao_p->addSkill("yuyue");
    daqiao_p->addSkill("shuangxing");

    General *sunshangxiang_p = new General(this, "sunshangxiang_p", "hero", 3, false, true);
    sunshangxiang_p->addSkill("jieyin_pass");
    sunshangxiang_p->addSkill("xiaoji");
    sunshangxiang_p->addSkill("jinguo_pass");

    General *lubu_p = new General(this, "lubu_p", "hero", 4, true, true);
    lubu_p->addSkill("wushuang");
    lubu_p->addSkill("zhanshen_pass");
    lubu_p->addSkill("nuozhan_pass");

    General *huatuo_p = new General(this, "huatuo_p", "hero", 3, true, true);
    huatuo_p->addSkill("qingnang_pass");
    huatuo_p->addSkill("#qingnang");
    huatuo_p->addSkill("jijiu");
    huatuo_p->addSkill("yangshen_pass");
    huatuo_p->addSkill("mafei_pass");
    huatuo_p->addSkill("#mafei");

    General *diaochan_p = new General(this, "diaochan_p", "hero", 3, false, true);
    diaochan_p->addSkill("lijian_pass");
    diaochan_p->addSkill("biyue");

    General *yuji_p = new General(this, "yuji_p", "hero", 3, true, true);
    yuji_p->addSkill("guhuo_pass");
    yuji_p->addSkill("#guhuo_pass_mark");
    yuji_p->addSkill("bugua_pass");
    yuji_p->addSkill("huanshu_pass");

    General *zhangjiao_p = new General(this, "zhangjiao_p", "hero", 3, true, true);
    zhangjiao_p->addSkill("huangtian_pass");
    zhangjiao_p->addSkill("leiji_pass");
    zhangjiao_p->addSkill("daji_pass");
    zhangjiao_p->addSkill("guidao");

    addMetaObject<DuanyanCard>();
    addMetaObject<QuanhengCard>();
    addMetaObject<YaoshuCard>();
    addMetaObject<RendePassCard>();
    addMetaObject<JijiangPassCard>();
    addMetaObject<LiegongPassCard>();
    addMetaObject<TuodaoPassCard>();
    addMetaObject<DuanhePassCard>();
    addMetaObject<BeifaPassCard>();
    addMetaObject<LuoyiPassCard>();
    addMetaObject<TuxiPassCard>();
    addMetaObject<FangzhuPassCard>();
    addMetaObject<ZhihengPassCard>();
    addMetaObject<FanjianPassCard>();
    addMetaObject<KurouPassCard>();
    addMetaObject<ZhaxiangCard>();
    addMetaObject<JieyinPassCard>();
    addMetaObject<YuyueCard>();
    addMetaObject<LijianPassCard>();
    addMetaObject<QingnangPassCard>();

    addMetaObject<PassModeItemCard>();
}

class NothrowHandcardsPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return ! player->hasEquip(card) ;
    }
    virtual bool willThrow() const{
        return false;
    }
};
class NothrowPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return true;
    }
    virtual bool willThrow() const{
        return false;
    }
};

HeroPackage::HeroPackage()
    :Package("hero")
{
    patterns[".NTH"] = new NothrowHandcardsPattern;
    patterns[".NT"] = new NothrowPattern;
}
ADD_PACKAGE(Hero)

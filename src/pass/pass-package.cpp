#include "pass-package.h"
#include "engine.h"
#include "standard-skillcards.h"
#include "clientplayer.h"
#include "client.h"
#include "carditem.h"
#include "aux-skills.h"
#include "wind.h"

class TipoPass:public PassiveSkill{
public:
    TipoPass():PassiveSkill("tipo_p"){
    }

    virtual void onAcquire(ServerPlayer *player) const{
        Room *room = player->getRoom() ;
        room->setPlayerProperty(player, "maxhp", player->getMaxHP() + 1);
        room->setPlayerProperty(player, "hp", player->getHp() + 1);
    }

    virtual void onDetach(ServerPlayer *player) const{
        Room *room = player->getRoom() ;
        room->setPlayerProperty(player, "maxhp", player->getMaxHP() - 1);
    }
};

class DuanyanPass:public ZeroCardViewAsSkill{
public:
    DuanyanPass():ZeroCardViewAsSkill("duanyan_p"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DuanyanPassCard");
    }

    virtual const Card *viewAs() const{
        return new DuanyanPassCard;
    }
};

DuanyanPassCard::DuanyanPassCard(){
    once = true;
}

bool DuanyanPassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self ;
}

void DuanyanPassCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *player = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = player->getRoom();

    QString choice = room->askForChoice(player, "duanyan_p", "slash+jink+peach_analeptic+other");

    int card_id = target->getRandomHandCardId();
    const Card *card = Sanguosha->getCard(card_id);
    LogMessage log;
    log.type = "#DuanYanPassChooseType";
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


class XiongziPass:public TriggerSkill{
public:
    XiongziPass():TriggerSkill("xiongzi_p"){
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


class QiangongPass:public TriggerSkill{
public:
    QiangongPass():TriggerSkill("qiangong_p"){
        events << CardEffected ;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        Room *room = player->getRoom();
        if((effect.card->inherits("Dismantlement") || effect.card->inherits("Snatch")) && player->askForSkillInvoke(objectName())){
            LogMessage log;
            log.from = player;
            log.arg = effect.card->objectName() ;
            if(effect.card->inherits("Dismantlement") && player->getEquips().empty()){
                room->askForDiscard(player, objectName(), 1 , false , true) ;
                log.type = "#QiangongPassThrow";
                player->getRoom()->sendLog(log);
                return true;
            }else{
                const Card *card = room->askForCard(player,".NT!","@qiangong_p-card");
                if(!card){
                    if(player->isKongcheng()){
                        card = player->getEquips().at(0);
                    }else{
                        card = player->getRandomHandCard() ;
                    }
                }
                room->moveCardTo(card, effect.from, Player::Hand);
                log.to << effect.from;
                log.type = "#QiangongPassGive";
                player->getRoom()->sendLog(log);
                return true;
            }
        }
        return false ;
    }
};


class QuanhengPass:public ViewAsSkill{
public:
    QuanhengPass():ViewAsSkill("quanheng_p"){

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

        QuanhengPassCard *card = new QuanhengPassCard;
        card->addSubcards(cards);

        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QuanhengPassCard") && player->isWounded() ;
    }
};


QuanhengPassCard::QuanhengPassCard(){
    target_fixed = true;
    once = true;
}

void QuanhengPassCard::use(Room *room, ServerPlayer *player, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    player->drawCards(this->getSubcards().length());
}


class ShiqiPass:public TriggerSkill{
public:
    ShiqiPass():TriggerSkill("shiqi_p"){
        events << PhaseChange << FinishJudge;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *shizu, QVariant &data) const{
        if(event == PhaseChange && shizu->getPhase() == Player::Draw){
            Room *room = shizu->getRoom();
            if(shizu->askForSkillInvoke(objectName())){
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = shizu;
                judge.time_consuming = true;

                room->judge(judge);
            }
        }else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(judge->reason == objectName() && judge->card->isRed()){
                shizu->obtainCard(judge->card);
                return true;
            }
        }
        return false;
    }
};


class QianggongPass: public SlashBuffSkill{
public:
    QianggongPass():SlashBuffSkill("qianggong_p"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *gongshou = effect.from;
        Room *room = gongshou->getRoom();

        if(gongshou->getHp() <= 1 || gongshou->getAttackRange() > 4){
            if(gongshou->askForSkillInvoke(objectName(), QVariant::fromValue(effect))){
                room->slashResult(effect, NULL);
                return true;
            }
        }
        return false;
    }
};

class PojiaPass: public TriggerSkill{
public:
    PojiaPass():TriggerSkill("pojia_p"){
        events << SlashEffect << SlashHit;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *jianshi, QVariant &data) const{
        Room *room = jianshi->getRoom();
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = jianshi;
        log.arg = objectName();
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(event == SlashEffect){
            if(effect.slash->isBlack()){
                room->sendLog(log);
                effect.to->addMark("qinggang");
            }
        }else if(event == SlashHit){
            if(effect.slash->isRed()){
                QList<const Card *> equips = effect.to->getEquips() ;
                if(!equips.empty()){
                    room->sendLog(log);
                    room->throwCard(equips.at(qrand() % equips.length())) ;
                }
            }
        }
        return false;
    }
};

class ZhanshangPass: public TriggerSkill{
public:
    ZhanshangPass():TriggerSkill("zhanshang_p"){
        events << CardEffected;
        frequency = Frequent ;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *jianshi, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.card->inherits("AOE")){
            Room *room = jianshi->getRoom();
            jianshi->drawCards(1);
            LogMessage log;
            log.type = "#TriggerDrawSkill";
            log.from = jianshi;
            log.arg = objectName();
            log.arg2 = QString::number(1);
            room->sendLog(log);
        }
        return false;
    }
};

class QishuPass: public DistanceSkill{
public:
    QishuPass():DistanceSkill("qishu_p"){
        frequency = Frequent;
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



class XunmaPass: public OneCardViewAsSkill{
public:
    XunmaPass():OneCardViewAsSkill("xunma_p"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Horse") ;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        XunmaPassCard *card = new XunmaPassCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

XunmaPassCard::XunmaPassCard(){
    target_fixed = true ;
}

void XunmaPassCard::use(Room *room, ServerPlayer *qibing, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    qibing->drawCards(2);
    if(qibing->isWounded()){
        RecoverStruct recover;
        recover.card = this;
        recover.who = qibing;
        room->recover(qibing,recover);
    }
}


class ChenwenPass: public TriggerSkill{
public:
    ChenwenPass():TriggerSkill("chenwen_p"){
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

class ZhongzhuangPass: public TriggerSkill{
public:
    ZhongzhuangPass():TriggerSkill("zhongzhuang_p"){
        events << Predamaged;
    }
    virtual int getPriority() const{
        return -1;
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.damage > 1){
            LogMessage log;
            log.type = "#ZhongzhuangPass";
            log.from = player;
            log.arg = QString::number(damage.damage - 1);
            player->getRoom()->sendLog(log);

            damage.damage = 1;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};


class DianjiPass: public OneCardViewAsSkill{
public:
    DianjiPass():OneCardViewAsSkill("dianji_p"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DianjiPassCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        DianjiPassCard *card = new DianjiPassCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

DianjiPassCard::DianjiPassCard(){
    once = true;
}

bool DianjiPassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void DianjiPassCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *leishi = effect.from;
    ServerPlayer *target = effect.to;

    Room *room = leishi->getRoom();
    room->setEmotion(target, "bad");

    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(spade|club):(.*)");
    judge.good = false;
    judge.reason = "leiji";
    judge.who = target;

    room->judge(judge);

    DamageStruct damage;
    damage.card = NULL;
    damage.damage = 1;
    damage.from = leishi;
    damage.nature = DamageStruct::Thunder;

    if(judge.isBad()){
        damage.to = target;
    }else{
        damage.to = leishi;
    }
    room->damage(damage);
}


class LeilingPass: public TriggerSkill{
public:
    LeilingPass():TriggerSkill("leiling_p"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Thunder){
            data = QVariant::fromValue(damage);
            LogMessage log;
            log.type = "#TriggerDamageDownSkill";
            log.from = player;
            log.arg = objectName();
            log.arg2 = QString::number(damage.damage) ;
            player->getRoom()->sendLog(log);
            player->drawCards(damage.damage);
            damage.damage = 0;
        }
        return false;
    }
};


class LianzhanPass: public TriggerSkill{
public:
    LianzhanPass():TriggerSkill("lianzhan_p"){
        events << SlashMissed << Predamage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = player->getRoom();
        static const QString flag = "lianzhan_p" ;
        if(event == SlashMissed){
            if(effect.to->hasSkill("kongcheng") && effect.to->isKongcheng())
                return false;
            const Card *card = room->askForCard(player, "slash", "blade-slash");
            if(card){
                // if player is drank, unset his flag
                if(player->hasFlag("drank"))
                    room->setPlayerFlag(player, "-drank");

                room->setPlayerFlag(player, flag);
                CardUseStruct use;
                use.card = card;
                use.from = player;
                use.to << effect.to;
                room->useCard(use, false);
                room->setPlayerFlag(player, "-" + flag);
            }
        }else if(event == Predamage){
            DamageStruct damage = data.value<DamageStruct>();
            const Card *reason = damage.card;
            if(reason && reason->inherits("Slash") && player->hasFlag(flag)){
                LogMessage log;
                log.type = "#TriggerDamageUpSkill";
                log.from = player;
                log.to << damage.to;
                log.arg = objectName();
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
    DouzhiPass():TriggerSkill("douzhi_p"){
        events << PhaseChange;
        frequency = Frequent ;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        int n = 0 ;
        if(player->getPhase() == Player::Finish){
            n = player->getMaxHP() - player->getHandcardNum() ;
        }
        if(n > 0 && room->askForSkillInvoke(player,objectName())){
            player->drawCards(n);
            LogMessage log;
            log.type = "#TriggerDrawSkill";
            log.from = player;
            log.arg = objectName();
            log.arg2 = QString::number(n);
            room->sendLog(log);
        }
        return false;
    }
};

class XiangxiaoPass:public MasochismSkill{
public:
    XiangxiaoPass():MasochismSkill("xiangxiao_p"){
    }

    virtual void onDamaged(ServerPlayer *wuniang, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = wuniang->getRoom();
        QVariant data = QVariant::fromValue(from);
        QList<ServerPlayer *> targets ;
        foreach (ServerPlayer *p, room->getAlivePlayers()) {
            if(p != wuniang && p->isWounded()){
                targets << p ;
            }
        }

        if(!targets.empty() && room->askForSkillInvoke(wuniang, objectName() , data)){
            ServerPlayer *frd = room->askForPlayerChosen(wuniang,targets,objectName());
            if(frd){
                RecoverStruct recover;
                recover.who = frd;
                room->recover(frd, recover);
            }
        }
    }
};



class ZhaoliePass: public TriggerSkill{
public:
    ZhaoliePass():TriggerSkill("zhaolie_p"){
        events << PhaseChange;
        frequency = Frequent ;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *liubei, QVariant &data) const{
        Room *room = liubei->getRoom();
        if(liubei->getPhase() == Player::Finish && liubei->isKongcheng() && room->askForSkillInvoke(liubei,objectName())){
            while(true){
                int card_id = room->drawCard();
                room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::Special, true);
                room->getThread()->delay();
                const Card *card = Sanguosha->getCard(card_id);
                room->obtainCard(liubei, card_id);
                if(card->inherits("BasicCard") || card->inherits("EquipCard"))
                    break;
            }
        }
        return false;
    }
};

class JunshenPass:public TriggerSkill{
public:
    JunshenPass():TriggerSkill("junshen_p"){
        events << PhaseChange << FinishJudge;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *guanyu, QVariant &data) const{
        if(event == PhaseChange && guanyu->getPhase() == Player::Start){
            Room *room = guanyu->getRoom();
            while(guanyu->askForSkillInvoke("junshen_p")){
                room->playSkillEffect(objectName());

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = guanyu;
                judge.time_consuming = true;

                room->judge(judge);
                if(judge.isBad())
                    break;
            }

        }else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(judge->reason == objectName()){
                if(judge->card->isRed()){
                    guanyu->obtainCard(judge->card);
                    return true;
                }
            }
        }
        return false;
    }
};


class BadaoPass:public MasochismSkill{
public:
    BadaoPass():MasochismSkill("badao_p"){
        frequency = Compulsory ;
    }

    virtual void onDamaged(ServerPlayer *caocao, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = caocao->getRoom();
        if(damage.damage < 2)
            return ;
        if(!from || !from->isAlive() || from->getPhase() != Player::Play || from == caocao)
            return ;
        LogMessage log;
        log.type = "#ForceEndSkill";
        log.from = caocao;
        log.to << from;
        log.arg = objectName() ;
        room->sendLog(log);
        room->setPlayerFlag(from,"force_end_play");
    }
};

class ShoujiePass: public TriggerSkill{
public:
    ShoujiePass():TriggerSkill("shoujie_p"){
        events << Dying;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *zhangliao, QVariant &data) const{
        DyingStruct dying_data = data.value<DyingStruct>();
        ServerPlayer *damager = dying_data.damage ? dying_data.damage->from : NULL;
        if(dying_data.who != zhangliao || zhangliao->isKongcheng())
            return false;
        if(! damager || damager == zhangliao || damager->isDead() || damager->isKongcheng())
            return false ;
        Room *room = zhangliao->getRoom();
        if(zhangliao->askForSkillInvoke(objectName(), data)){
            if(zhangliao->pindian(damager , "shoujie", NULL)){
                RecoverStruct rev;
                rev.recover = 1 - zhangliao->getHp();
                room->recover(zhangliao, rev);
            }
        }
        return false;
    }
};

class HonglangPass: public TriggerSkill{
public:
    HonglangPass():TriggerSkill("honglang_p"){
        events << CardGotDone << CardDrawnDone;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *sunquan, QVariant &) const{
        if(sunquan->getHandcardNum() == 2){
            sunquan->drawCards(1);
        }
        return false;
    }
};


static bool CompareByType(int card1, int card2){
    const Card *c1 = Sanguosha->getCard(card1);
    const Card *c2 = Sanguosha->getCard(card2);

    int a = static_cast<int>(c1->getTypeId());
    int b = static_cast<int>(c2->getTypeId());

    return a < b;
}

class GuoshiPass: public PhaseChangeSkill{
public:
    GuoshiPass():PhaseChangeSkill("guoshi_p"){

    }

    virtual bool onPhaseChange(ServerPlayer *lumeng) const{
        if(lumeng->getPhase() != Player::Draw || lumeng->isKongcheng())
            return false;

        Room *room = lumeng->getRoom();
        if(!lumeng->askForSkillInvoke(objectName()))
            return false;

        QList<int> card_ids = room->getNCards(qMin(lumeng->getHandcardNum() , 6));
        qSort(card_ids.begin(), card_ids.end(), CompareByType);
        room->fillAG(card_ids);

        while(!card_ids.isEmpty()){
            int card_id = room->askForAG(lumeng, card_ids, false, "guoshi_p");
            card_ids.removeOne(card_id);
            room->takeAG(lumeng, card_id);

            // throw the rest cards that matches the same type
            const Card *card = Sanguosha->getCard(card_id);
            Card::CardType type = card->getTypeId();
            QMutableListIterator<int> itor(card_ids);
            while(itor.hasNext()){
                const Card *c = Sanguosha->getCard(itor.next());
                if(c->getTypeId() == type){
                    itor.remove();
                    room->takeAG(NULL, c->getId());
                }
            }
        }
        room->broadcastInvoke("clearAG");
        return true;
    }
};







class WanghouPass: public OneCardViewAsSkill{
public:
    WanghouPass():OneCardViewAsSkill("wanghou_p"){
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return !player->isNude()
                && ! pattern.startsWith("@")
                && ! pattern.startsWith(".");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        int num = to_select->getFilteredCard()->getNumber() ;
        return (num == 5 || num == 9) && (to_select->getFilteredCard()->isRed()) ;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        if(ClientInstance->getStatus() == Client::Responsing){
            const Card *origin = card_item->getFilteredCard() ;
            Card *card = Sanguosha->cloneCard(ClientInstance->getPattern(), origin->getSuit(), origin->getNumber());
            card->addSubcard(card_item->getFilteredCard());
            card->setSkillName(objectName());
            return card;
        }

        CardStar c = Self->tag.value("wanghou").value<CardStar>();
        if(c){
            const Card *origin = card_item->getFilteredCard() ;
            Card *card = Sanguosha->cloneCard(c->objectName(), origin->getSuit(), origin->getNumber());
            card->addSubcard(card_item->getFilteredCard());
            card->setSkillName(objectName());
            return card;
        }else
            return NULL;
    }

    virtual QDialog *getDialog() const{
        return GuhuoDialog::GetInstance("wanghou");
    }
};

class QiangyunPass: public TriggerSkill{
public:
    QiangyunPass():TriggerSkill("qiangyun_p"){
        events << StartJudge;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        JudgeStar judge = data.value<JudgeStar>();

        if(judge->who == player && room->askForSkillInvoke(player,objectName(),data)){
            room->throwCard(judge->card);

            judge->card = Sanguosha->getCard(room->drawCard()) ;
            room->moveCardTo(judge->card, NULL, Player::Special);
            room->sendJudgeResult(judge);
            room->getThread()->delay();

            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = player;
            log.to << judge->who;
            log.card_str = judge->card->getEffectIdString();
            room->sendLog(log);
        }
        return false;
    }
};

class XiaoxiongPass: public TriggerSkill{
public:
    XiaoxiongPass():TriggerSkill("xiaoxiong_p"){
        events << CardLost;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->from_place == Player::Equip){
            Room *room = player->getRoom();
            if(player->isWounded() && room->askForSkillInvoke(player, objectName())){
                RecoverStruct recover;
                recover.who = player;
                room->recover(player, recover);
            }
        }
        return false;
    }
};

HujiangPassCard::HujiangPassCard(){
    once = true;
    will_throw = false;
    target_fixed = true ;
}

void HujiangPassCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->showCard(source, getEffectiveId());
    const Card *card = Sanguosha->getCard(getSubcards().first()) ;
    QString a = QString("hujiang_p_%1").arg(card->isRed() ? "red" : "black") ;
    room->setPlayerFlag(source, QString("hujiang_p_%1").arg(card->isRed() ? "red" : "black"));
}

class HujiangPass: public OneCardViewAsSkill{
public:
    HujiangPass():OneCardViewAsSkill("hujiang_p"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("HujiangPassCard") || player->hasFlag("hujiang_p_red");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return ! to_select->isEquipped() ;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        if(Self->hasFlag("hujiang_p_red")){
            const Card *card = card_item->getCard();
            Card *slash = new Slash(card->getSuit(), card->getNumber());
            slash->addSubcard(card->getId());
            slash->setSkillName(objectName());
            return slash;
        }else{
            HujiangPassCard *card = new HujiangPassCard;
            card->addSubcard(card_item->getFilteredCard());
            return card;
        }
    }
};

class HujiangPassBuff: public TriggerSkill{
public:
    HujiangPassBuff():TriggerSkill("#hujiang_p"){
        events << SlashHit << CardUsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == SlashHit && player->hasFlag("hujiang_p_black")){
            player->drawCards(1);
            room->setPlayerFlag(player,"-hujiang_p_black");
        }else if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->getSkillName() == "hujiang_p"){
                room->setPlayerFlag(player,"-hujiang_p_red");
            }
        }
        return false;
    }
};


class AoguPass: public TriggerSkill{
public:
    AoguPass():TriggerSkill("aogu_p"){
        events << Predamaged;
    }
    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.damage > 1 && player->getEquips().isEmpty()){
            damage.damage = 1;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class ZhongyiPass: public TriggerSkill{
public:
    ZhongyiPass():TriggerSkill("zhongyi_p"){
        frequency = Frequent;
        events << CardLost << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        // Room *room = player->getRoom();
        if(event == PhaseChange){
            if(player->getPhase() == Player::Finish){
                if(player->getMark("zhongyi_p") >= 2 && player->askForSkillInvoke(objectName(), data)){
                    QList<Player::Phase> phases = player->getPhases();
                    phases.prepend(Player::Play) ;
                    player->play(phases);
                }
            }else if(player->getPhase() == Player::RoundStart){
                player->setMark("zhongyi_p", 0);
            }
            return false;
        }
        if(player->getPhase() == Player::Discard){
            CardMoveStar move = data.value<CardMoveStar>();
            const Card *card = Sanguosha->getCard(move->card_id) ;
            if(move->to_place == Player::DiscardedPile){
                if(card->isRed()){
                    player->addMark("zhongyi_p");
                }
            }
        }
        return false;
    }
};


class CaiqingPass: public TriggerSkill{
public:
    CaiqingPass():TriggerSkill("caiqing_p"){
        events << CardLost << PhaseChange;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == PhaseChange){
            if(player->getPhase() == Player::Finish){
                if(player->isWounded() && player->getMark("caiqing_p") >= 2 && player->askForSkillInvoke(objectName(), data)){
                    RecoverStruct recover;
                    recover.who = player;
                    room->recover(player, recover);
                }
            }else if(player->getPhase() == Player::RoundStart){
                player->setMark("caiqing_p", 0);
            }
            return false;
        }
        if(player->getPhase() == Player::Discard){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->to_place == Player::DiscardedPile){
                player->addMark("caiqing_p");
            }
        }
        return false;
    }
};

class DuoyiPass: public TriggerSkill{
public:
    DuoyiPass():TriggerSkill("duoyi_p"){
        events << SlashMissed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        room->setPlayerFlag(player,"duoyi_p");
        return false;
    }
};


LiangjiangPassCard::LiangjiangPassCard(){
    target_fixed = true;
    once = true;
}

void LiangjiangPassCard::use(Room *room, ServerPlayer *player, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    player->drawCards(this->getSubcards().length());
}

class LiangjiangPass: public OneCardViewAsSkill{
public:
    LiangjiangPass():OneCardViewAsSkill("liangjiang_p"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("LiangjiangPassCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true ;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LiangjiangPassCard *card = new LiangjiangPassCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class FenyongPass: public TriggerSkill{
public:
    FenyongPass():TriggerSkill("fenyong_p"){
        view_as_skill = new DiscardViewAsSkill("@fenyong_p") ;
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = damage.to->getRoom();
        if(damage.card && damage.card->inherits("Slash")&& damage.to->isKongcheng() && !player->isKongcheng()){
                const Card *card = room->askForCard(player, "@fenyong_p", "@fenyong_p:" + damage.to->objectName(),data) ;
                if(card){
                    damage.damage ++;
                    data = QVariant::fromValue(damage);
                }
        }
        return false;
    }
};

class WuliePass:public OneCardViewAsSkill{
public:
    WuliePass():OneCardViewAsSkill("wulie_p"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "slash";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        if(card->getSuit() != Card::Heart || card->isEquipped())
            return false;

        if(card == Self->getWeapon() && card->objectName() == "crossbow")
            return Self->canSlashWithoutCrossbow();
        else
            return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Card *slash = new Slash(card->getSuit(), card->getNumber());
        slash->addSubcard(card->getId());
        slash->setSkillName(objectName());
        return slash;
    }
};


QuanmouPassCard::QuanmouPassCard(){
    once = true;
    will_throw = false;
}

void QuanmouPassCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);

    Room *room = effect.from->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "hej", "quanmou_p");
    const Card *card = Sanguosha->getCard(card_id);
    bool is_public = room->getCardPlace(card_id) != Player::Hand;
    room->moveCardTo(card, effect.from, Player::Hand, is_public ? true : false);

    QList<ServerPlayer *> targets = room->getOtherPlayers(effect.to);
    ServerPlayer *target = room->askForPlayerChosen(effect.from, targets, "quanmou_p");
    if(target != effect.from)
        room->moveCardTo(card, target, Player::Hand, false);
}

class QuanmouPass: public OneCardViewAsSkill{
public:
    QuanmouPass():OneCardViewAsSkill("quanmou_p"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QuanmouPassCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return ! to_select->isEquipped() && to_select->getFilteredCard()->inherits("TrickCard") ;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        QuanmouPassCard *card = new QuanmouPassCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class DuduPass: public TriggerSkill{
public:
    DuduPass():TriggerSkill("dudu_p"){
        view_as_skill = new DiscardViewAsSkill("@dudu_p","e") ;
        events << DamageProceed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && ! target->getEquips().empty() ;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        if(room->askForCard(player, "@dudu_p", "@dudu_p")){
            damage.damage--;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};


ZhejiePassCard::ZhejiePassCard(){
    once = true;
    will_throw = false;
}

bool ZhejiePassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return ! to_select->isKongcheng() && to_select->getHandcardNum() < Self->getHandcardNum() ;
}

void ZhejiePassCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->askForDiscard(effect.to,"zhejie_p",1);
    effect.to->obtainCard(this);
}

class ZhejiePass: public OneCardViewAsSkill{
public:
    ZhejiePass():OneCardViewAsSkill("zhejie_p"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ZhejiePassCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return ! to_select->isEquipped() ;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ZhejiePassCard *card = new ZhejiePassCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class DanmouPass: public TriggerSkill{
public:
    DanmouPass():TriggerSkill("danmou_p"){
        events << DrawNCards << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == DrawNCards){
            if(room->askForSkillInvoke(player,objectName())){
                data = data.toInt() + 2;
                room->setPlayerFlag(player,objectName());
            }
        }else if(event == PhaseChange && player->getPhase() == Player::Finish){
            if(!player->hasFlag(objectName()))
                return false ;
            int x = 2 ;
            int total = player->getEquips().length() + player->getHandcardNum();

            if(total <= x){
                player->throwAllHandCards();
                player->throwAllEquips();
                room->loseHp(player);
            }else{
                room->askForDiscard(player, objectName(), x, false, true);
            }
        }

        return false;
    }
};



class WujiPass: public TriggerSkill{
public:
    WujiPass():TriggerSkill("wuji_p"){
        events << CardUsed << CardFinished;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *sunshangxiang, QVariant &data) const{
        Room *room = sunshangxiang->getRoom();
        CardUseStruct use = data.value<CardUseStruct>();
        if(!use.card || !use.card->inherits("Slash"))
            return false;
        if(event == CardUsed){
            if(use.to.isEmpty())
                return false;
            const Weapon *weapon = use.to.first()->getWeapon() ;
            if(weapon){
                room->setPlayerFlag(sunshangxiang, QString("wuji_weapon:%1").arg(weapon->objectName()));
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = sunshangxiang;
                log.arg = objectName();
            }
        }else if(event == CardFinished){
            room->setPlayerFlag(sunshangxiang, QString("-wuji_weapon:*"));
        }
        return false;
    }
};


class FeijiangPass:public TriggerSkill{
public:
    FeijiangPass():TriggerSkill("feijiang_p"){
        events << CardEffect;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->inherits("Slash") && player->isKongcheng() && effect.to && !effect.to->isKongcheng()){
            if(player->askForSkillInvoke(objectName())){
                int card_id = effect.to->getRandomHandCardId();
                const Card *card = Sanguosha->getCard(card_id);
                player->obtainCard(card);
            }
        }
        return false ;
    }
};

class ZhunuePass : public TriggerSkill{
public:
    ZhunuePass():TriggerSkill("zhunue_p"){
        frequency = Frequent;
        events << Damage << Damaged;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() == Player::NotActive){
            player->drawCards(1);
        }
        return false;
    }
};

class DoushenPass: public TriggerSkill{
public:
    DoushenPass():TriggerSkill("doushen_p"){
        events << CardEffected;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.card->isNDTrick() && ! effect.card->inherits("Duel")){
            Room *room = player->getRoom();
            if(effect.to == effect.from)
                return false ;

            QString suit_str = effect.card->getSuitString();
            QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
            QString prompt = QString("@doushen_p:::%1").arg(suit_str);

            if(effect.to->hasSkill(objectName()) && effect.from){
                const Card *card = room->askForCard(effect.to,pattern,prompt,data) ;
                if(card){
                    Duel *duel = new Duel(effect.card->getSuit(),effect.card->getNumber());
                    duel->setSkillName(objectName());

                    CardUseStruct use;
                    use.from = effect.from;
                    use.to << effect.to;
                    use.card = duel;
                    room->useCard(use);
                    return true;
                }
            }
        }
        return false;
    }
};




class XietianPass: public TriggerSkill{
public:
    XietianPass():TriggerSkill("xietian_p"){
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
                log.type = "#XietianPass";
            }else{
                damage.damage ++;
                data = QVariant::fromValue(damage);
                log.type = "#TriggerDamageUpSkill";
                log.arg = objectName();
                log.arg2 = QString::number(damage.damage) ;
            }
            caocao->getRoom()->sendLog(log);
        }
        return false;
    }
};

class LangguPass: public PhaseChangeSkill{
public:
    LangguPass():PhaseChangeSkill("langgu_p"){
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
    FankuiPass():MasochismSkill("fankui_p"){

    }

    virtual void onDamaged(ServerPlayer *simayi, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = simayi->getRoom();
        QVariant data = QVariant::fromValue(from);
        if(from && from->isAlive() && room->askForSkillInvoke(simayi, objectName() , data)){
            int x = damage.damage, i;
            for(i=0; i<x; i++){
                if(!from->isNude()){
                    int card_id = room->askForCardChosen(simayi, from, "he", objectName());
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
            room->playSkillEffect(objectName());
        }
    }
};

class GangliePass:public MasochismSkill{
public:
    GangliePass():MasochismSkill("ganglie_p"){

    }

    virtual void onDamaged(ServerPlayer *xiahou, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = xiahou->getRoom();
        QVariant source = QVariant::fromValue(from);

        if(from && from->isAlive() && room->askForSkillInvoke(xiahou, objectName(), source)){
            room->playSkillEffect(objectName());

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
                    if(!room->askForDiscard(from, objectName(), 2, true)){
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
    JueduanPass():TriggerSkill("jueduan_p"){
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
    YijiPass():TriggerSkill("yiji_p"){
        frequency = Frequent;
        events << Predamaged << Damaged << AskForPeachesDone;
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *guojia, QVariant &data) const{
        static const QString flag = "yiji-p_dying" ;
        if(event == Predamaged){
            if(guojia->hasFlag(flag))
                guojia->setFlags("-"+flag);
        }else if(event == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(guojia->isAlive()){
                Room *room = guojia->getRoom();

                if(!room->askForSkillInvoke(guojia, objectName()))
                    return false;

                room->playSkillEffect(objectName());
                int n = damage.damage * (guojia->hasFlag(flag) ? 3 : 2);
                guojia->drawCards(n);
                guojia->setFlags("-"+flag);
            }
        }else if(event == AskForPeachesDone){
            if(guojia->getHp() > 0)
                guojia->setFlags(flag);
        }
        return false;
    }
};

class TianduPass:public TriggerSkill{
public:
    TianduPass():TriggerSkill("tiandu_p"){
        events << FinishJudge;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true ;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        JudgeStar judge = data.value<JudgeStar>();
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(p->hasSkill(objectName()) && judge->card->isRed() && judge->card->getNumber() > p->getHp() + p->getHandcardNum() && p->askForSkillInvoke(objectName(), data)){
                room->playSkillEffect(objectName());
                p->drawCards(1);
                LogMessage log;
                log.type = "#TriggerDrawSkill";
                log.from = p;
                log.arg = objectName();
                log.arg2 = QString::number(1);
                room->sendLog(log);
            }
        }
        return false;
    }
};

class JinghongPass: public TriggerSkill{
public:
    JinghongPass():TriggerSkill("jinghong_p"){
        frequency = Frequent;
        events << CardLost << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhenji, QVariant &data) const{
        static const QString mark = "jinghong_p" ;
        if(event == CardLost){
            if(zhenji->getPhase() != Player::Discard)
                return false;
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->to_place == Player::DiscardedPile){
                zhenji->addMark(mark);
            }
        }else if(event == PhaseChange){
            if(zhenji->getPhase() != Player::Finish)
                return false;
            if(zhenji->getMark(mark) >= qMax(1,zhenji->getHp())){
                Room *room = zhenji->getRoom();
                if(zhenji->askForSkillInvoke(objectName())){
                    QList<int> cards = room->getNCards(zhenji->getMark(mark));

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
                            card_id = room->askForAG(zhenji, black_cards, true, objectName());
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
            zhenji->loseAllMarks(mark);
        }

        return false;
    }
};

class LuoyiPass:public ViewAsSkill{
public:
    LuoyiPass():ViewAsSkill("luoyi_p"){

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
    room->playSkillEffect("luoyi_p");
}

class GuantongPass: public TriggerSkill{
public:
    GuantongPass():TriggerSkill("guantong_p"){
        events << DamageComplete;
        frequency = Frequent;
        default_choice = "draw" ;
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
        if(xuchu->askForSkillInvoke(objectName(), data)){
            QString choice = "draw" ;
            if(!xuchu->isKongcheng()){
                choice =  room->askForChoice(xuchu,objectName(),"draw+damage");
            }
            if(choice == "damage"){
                if(room->askForDiscard(xuchu, objectName(), 1 , true)){
                    room->playSkillEffect(objectName());
                    damage.to = player->getNextAlive();
                    damage.card = NULL;
                    damage.damage = 1;
                    damage.nature = DamageStruct::Normal ;
                    room->damage(damage);
                }
            }else{
                xuchu->drawCards(1);
            }
        }
        return false;
    }
};

class XingshangPass: public TriggerSkill{
public:
    XingshangPass():TriggerSkill("xingshang_p"){
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

            QString result = room->askForChoice(caopi,objectName(),"draw+gethe") ;
            if(result == "draw"){
                room->playSkillEffect(objectName(), effect_type);
                caopi->drawCards(qMax(player->getHandcardNum(),player->getEquips().length()));
            }else if(result == "gethe"){
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
            }
        }
        return false;
    }
};

FangzhuPassCard::FangzhuPassCard(){
}
bool FangzhuPassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() ;
}

void FangzhuPassCard::onEffect(const CardEffectStruct &effect) const{
    int x = effect.to->getLostHp();
    effect.to->drawCards(x);

    Room *room = effect.to->getRoom();

    room->playSkillEffect(objectName());

    effect.to->turnOver();
}

class FangzhuPassViewAsSkill: public ZeroCardViewAsSkill{
public:
    FangzhuPassViewAsSkill():ZeroCardViewAsSkill("fangzhu_p"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@fangzhu_p";
    }

    virtual const Card *viewAs() const{
        return new FangzhuPassCard;
    }
};

class FangzhuPass: public MasochismSkill{
public:
    FangzhuPass():MasochismSkill("fangzhu_p"){
        view_as_skill = new FangzhuPassViewAsSkill;
    }

    virtual void onDamaged(ServerPlayer *caopi, const DamageStruct &damage) const{
        Room *room = caopi->getRoom();
        room->askForUseCard(caopi, "@@fangzhu_p", "@fangzhu_p");
    }
};

class FanzhiPass: public TriggerSkill{
public:
    FanzhiPass():TriggerSkill("fanzhi_p"){
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
                    caopi->pindian(target, objectName());
            }
            return false;
        }else {
            PindianStar pindian = data.value<PindianStar>();
            if(pindian->reason == objectName() && pindian->isSuccess()){
                pindian->from->turnOver();
                pindian->to->turnOver();
            }
        }
        return false;
    }
};

class WenjiuPass:public TriggerSkill{
public:
    WenjiuPass():TriggerSkill("wenjiu_p"){
        events << PhaseChange ;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *guanyu, QVariant &data) const{
        Room *room = guanyu->getRoom() ;
        if(event == PhaseChange){
            if(guanyu->getPhase() == Player::Start){
                if(guanyu->askForSkillInvoke(objectName())){
                    room->setPlayerFlag(guanyu, "wenjiu_p");
                    guanyu->setMark("@wenjiu_p",0);
                    guanyu->gainMark("@wenjiu_p",2);
                    guanyu->drawCards(1);
                    guanyu->setFlags("wenjiu_p");
                }
            }else if(guanyu->getPhase() == Player::Finish){
                if(guanyu->getMark("@wenjiu_p") > 0){
                    guanyu->loseMark("@wenjiu_p");
                }
                if(guanyu->hasFlag("wenjiu_p")){
                    room->setPlayerFlag(guanyu, "-wenjiu_p");
                }
            }
        }
        return false;
    }
};

class WenjiuPassBuff: public TriggerSkill{
public:
    WenjiuPassBuff():TriggerSkill("#wenjiu_p"){
        events << Predamage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@wenjiu_p") > 0 && target->isAlive();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *guanyu, QVariant &data) const{
        Room *room = guanyu->getRoom() ;
        DamageStruct damage = data.value<DamageStruct>();

        const Card *reason = damage.card;
        if(reason == NULL)
            return false;

        if(reason->inherits("Slash") || reason->inherits("Duel")){
            LogMessage log;
            log.type = "#TriggerDamageUpSkill";
            log.from = guanyu;
            log.to << damage.to;
            log.arg = "wenjiu_p";
            log.arg2 = QString::number(damage.damage + 1);
            guanyu->getRoom()->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
            if(reason->inherits("Slash"))
                room->setPlayerMark(guanyu, "@wenjiu_p" , 0);
        }
        return false;
    }
};


class TuodaoPass: public OneCardViewAsSkill{
public:
    TuodaoPass():OneCardViewAsSkill("tuodao_p"){

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
    duel->setSkillName("tuodao_p");
    duel->setCancelable(false);

    CardUseStruct use;
    use.from = from;
    use.to << to;
    use.card = duel;
    room->useCard(use);
}


class BaonuPass: public TriggerSkill{
public:
    BaonuPass():TriggerSkill("baonu_p"){
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
    DuanhePass():OneCardViewAsSkill("duanhe_p"){
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
    TiejiPass():TriggerSkill("tieji_p~"){
        events << SlashProceed << Predamage;
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == SlashProceed){
            if(data.canConvert<SlashEffectStruct>()){
                SlashEffectStruct effect = data.value<SlashEffectStruct>();

                if(player->isAlive()){
                    ServerPlayer *machao = effect.from;

                    Room *room = machao->getRoom();
                    if(effect.from->askForSkillInvoke(objectName(), QVariant::fromValue(effect))){
                        room->playSkillEffect("tieji_p");

                        JudgeStruct judge;
                        judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                        judge.good = true;
                        judge.reason = objectName();
                        judge.who = machao;

                        room->judge(judge);
                        if(judge.isGood()){
                            if(judge.card->getSuit() == Card::Diamond){
                                effect.to->setFlags("tieji_p_damage");
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
            if(damage.to->hasFlag("tieji_p_damage")){
                damage.damage ++ ;
                data = QVariant::fromValue(damage);
                damage.to->setFlags("-tieji_p_damage");
            }
        }
        return false;
    }
};


class MashuPass: public DistanceSkill{
public:
    MashuPass():DistanceSkill("mashu_p"){

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
    LongweiPass():TriggerSkill("longwei_p"){
        events << CardLost;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *zhaoyun, QVariant &data) const{
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->to_place == Player::Equip && Sanguosha->getCard(move->card_id)->inherits("Weapon")){
            Room *room = zhaoyun->getRoom();
            QList<ServerPlayer *> targets;
            foreach(ServerPlayer *target, room->getOtherPlayers(zhaoyun)){
                if(zhaoyun->canSlash(target,false) && (target->getHp() > zhaoyun->getHp() || target->getHandcardNum() > zhaoyun->getHandcardNum()))
                    targets << target;
            }
            if(!targets.empty() && room->askForSkillInvoke(zhaoyun, objectName())){
                ServerPlayer *target = room->askForPlayerChosen(zhaoyun, targets, "longwei_p-slash");
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


class LonghouPass: public TriggerSkill{
public:
    LonghouPass():TriggerSkill("longhou_p"){
        events << SlashHit;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *zhaoyun, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = zhaoyun->getRoom();
        if(effect.to->getWeapon() != NULL && effect.to != zhaoyun && !zhaoyun->isNude() && zhaoyun->askForSkillInvoke(objectName(), QVariant::fromValue(effect.to))){
            room->moveCardTo(effect.to->getWeapon(),zhaoyun,Player::Hand);
            if(!room->askForDiscard(zhaoyun, objectName(), 2 , true)){
                room->loseHp(zhaoyun);
            }
        }
        return false;
    }
};


class GuanxingPass:public PhaseChangeSkill{
public:
    GuanxingPass():PhaseChangeSkill("guanxing_p"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *zhuge) const{
        if(zhuge->getPhase() == Player::Start && zhuge->askForSkillInvoke(objectName())){
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
    BeifaPass():ViewAsSkill("beifa_p"){
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
            room->setEmotion(player,"bad");
            DamageStruct damage;
            damage.from = zhuge;
            damage.to = player;
            room->damage(damage);
        }else{
            room->setEmotion(player,"good");
            room->loseHp(zhuge);
        }
    }
}

class JizhiPass:public TriggerSkill{
public:
    JizhiPass():TriggerSkill("jizhi_p"){
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
                room->playSkillEffect(objectName());
                yueying->drawCards(1);
                yueying->gainMark("@zhimou_p");
            }
        }
        return false;
    }
};

class ShipoPass:public TriggerSkill{
public:
    ShipoPass():TriggerSkill("shipo_p"){
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yueying, QVariant &data) const{
        return false;
    }
};

class JumouPass: public TriggerSkill{
public:
    JumouPass():TriggerSkill("jumou_p"){
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
                    if(p->hasSkill(objectName()) && p->getMark("@zhimou_p") >= p->getMaxHP() && room->askForSkillInvoke(p, objectName())){
                        p->obtainCard(use.card);
                        p->loseAllMarks("@zhimou_p");
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
    LiegongPass():OneCardViewAsSkill("liegong_p"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        int card_id = Self->getMark("liegong_p_mark");
        const Card *card = Sanguosha->getCard(card_id);
        return to_select->getFilteredCard()->getSuit() == card->getSuit();
        return !to_select->isEquipped() ;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Self->getMark("liegong_p_mark") > 0 && ! player->hasUsed("LiegongPassCard") ;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LiegongPassCard *card = new LiegongPassCard;
        card->addSubcard(card_item->getCard());
        return card;
    }
};

LiegongPassCard::LiegongPassCard(){
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
    room->playSkillEffect("liegong_p");
    int card_id = getSubcards().first();
    const Card *card = Sanguosha->getCard(card_id);
    Slash *slash = new Slash(card->getSuit(), card->getNumber());
    slash->addSubcard(this);
    slash->setSkillName("liegong_p");
    CardUseStruct use;
    use.card = slash;
    use.from = source;
    use.to = targets;
    room->useCard(use,false);
}


class LiegongPassMark: public TriggerSkill{
public:
    LiegongPassMark():TriggerSkill("#liegong_p_mark"){
        events << SlashEffect << PhaseChange;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *huangzhong, QVariant &data) const{
        Room *room = huangzhong->getRoom() ;
        if(event == SlashEffect){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            room->setPlayerMark(huangzhong,"liegong_p_mark",effect.slash->getEffectiveId());
            if(effect.slash->getSkillName() == "liegong_p"){
                room->playSkillEffect("liegong_p");
                foreach(const Card *card, effect.to->getEquips()){
                    if(card->getSuit() == effect.slash->getSuit())
                        room->throwCard(card->getEffectiveId());
                }
                room->slashResult(effect, NULL);
                return true;
            }
        }else if(event == PhaseChange && huangzhong->getPhase() == Player::Finish){
            room->setPlayerMark(huangzhong,"liegong_p_mark",0);
        }
        return false;
    }
};


class GongshenPass: public TriggerSkill{
public:
    GongshenPass():TriggerSkill("gongshen_p"){
        events << SlashHit;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *huangzhong, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        int n = 1 ;
        if(effect.slash->isRed())
            n++;
        huangzhong->gainMark("@gongshen_p",n);
        return false;
    }
};


class JianhunPass:public ViewAsSkill{
public:
    JianhunPass():ViewAsSkill("jianhun_p"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@gongshen_p") >= 5 ;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->inherits("Slash");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 1){
            JianhunPassCard *jpc = new JianhunPassCard ;
            jpc->addSubcard(cards.first()->getCard());
            return jpc ;
        }
        return NULL;
    }
};

JianhunPassCard::JianhunPassCard(){
}

bool JianhunPassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int n = 2 ;
    if(Self->getHp() == 1 || Self->getAttackRange() >= 4 || Self->getMark("@gongshen_p") >= 8)
        n ++ ;
    if(targets.length() >= n)
        return false;
    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;
    if(to_select == Self)
        return false;
    return true;
}

void JianhunPassCard::use(Room *room, ServerPlayer *huangzhong, const QList<ServerPlayer *> &targets) const{
    int n = 2 ;
    if(huangzhong->getHp() == 1 || huangzhong->getAttackRange() >= 4 || Self->getMark("@gongshen_p") >= 8)
        n ++ ;
    CardUseStruct use;
    use.from = huangzhong;
    use.card = Sanguosha->getCard(this->getSubcards().first());
    use.to = targets;
    LogMessage log;
    log.from = huangzhong;
    log.to = targets ;
    log.arg = QString::number(n);
    if(targets.length() == 1){
        log.type = "#JianhunPass1";
        room->sendLog(log);
        for(int i=0;i<n;i++){
            if(use.to.first()->isDead())
                break;
            room->useCard(use,false) ;
        }
    }else{
        log.type = "#JianhunPass2";
        room->sendLog(log);
        room->useCard(use,false) ;
    }
    huangzhong->loseAllMarks("@gongshen_p");
}


class FanjianPass: public OneCardViewAsSkill{
public:
    FanjianPass():OneCardViewAsSkill("fanjian_p"){

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
    room->playSkillEffect("fanjian_p");

    int card_id = getSubcards().first();
    const Card *card = Sanguosha->getCard(card_id);
    Card::Suit suit = room->askForSuit(target,"fanjian");

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

class KurouPass: public ZeroCardViewAsSkill{
public:
    KurouPass():ZeroCardViewAsSkill("kurou_p"){

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

class ZhaxiangPass:public ViewAsSkill{
public:
    ZhaxiangPass():ViewAsSkill("zhaxiang_p"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->isWounded() && !player->hasUsed("ZhaxiangPassCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 4 ;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 4){
            ZhaxiangPassCard *card = new ZhaxiangPassCard();
            card->addSubcards(cards);
            return card;
        }else
            return NULL;
    }
};

ZhaxiangPassCard::ZhaxiangPassCard(){
    once = true;
    target_fixed = true;
}

void ZhaxiangPassCard::use(Room *room, ServerPlayer *huanggai, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    RecoverStruct recover;
    recover.card = this;
    recover.who = huanggai;
    huanggai->getRoom()->recover(huanggai, recover);
}

class BaiyiPass:public TriggerSkill{
public:
    BaiyiPass():TriggerSkill("baiyi_p"){
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
    DujiangPass():TriggerSkill("dujiang_p"){
        events << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *lumeng, QVariant &data) const{
        if(lumeng->getPhase() == Player::Finish){
            Room *room = lumeng->getRoom();
            int n = lumeng->getEquips().length();
            if(n > 0 && room->askForSkillInvoke(lumeng, objectName())){
                if(n == 1){

                }else if(n == 2){
                    lumeng->drawCards(2);
                }else if(n == 3){
                    QList<ServerPlayer *> players = room->getOtherPlayers(lumeng);
                    int count = 0 ;
                    foreach(ServerPlayer *p, players){
                        if(! p->isNude()){
                            int card_id = room->askForCardChosen(lumeng, p, "he", objectName());
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
    LianyingPass():TriggerSkill("lianying_p"){
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
                        room->playSkillEffect("lianying_p");
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
    JieyinPass():OneCardViewAsSkill("jieyin_p"){

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

    room->playSkillEffect("jieyin_p");
    if(!effect.to->isNude()){
        int card_id = room->askForCardChosen(effect.from, effect.to, "he", "jieyin_p");
        const Card *card = Sanguosha->getCard(card_id);
        room->moveCardTo(card, effect.from, Player::Hand, false);
    }
}

class JinguoPass: public TriggerSkill{
public:
    JinguoPass():TriggerSkill("jinguo_p"){
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

class YuyuePass: public OneCardViewAsSkill{
public:
    YuyuePass():OneCardViewAsSkill("yuyue_p"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YuyuePassCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        YuyuePassCard *card = new YuyuePassCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};


YuyuePassCard::YuyuePassCard(){
}

bool YuyuePassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return ! to_select->getJudgingArea().empty() ;
}

void YuyuePassCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("yuyue_p");
    for(int i=0;i<effect.to->getJudgingArea().length();i++){
        if(effect.to->isDead())
            break;
        CardUseStruct card_use;
        card_use.card = slash;
        card_use.from = effect.from;
        card_use.to << effect.to;
        room->useCard(card_use, false);
    }
}


class ShuangxingPass: public PhaseChangeSkill{
public:
    ShuangxingPass():PhaseChangeSkill("shuangxing_p"){
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


class JieluePass: public TriggerSkill{
public:
    JieluePass():TriggerSkill("jielue_p"){
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
    ZhanshenPass():OneCardViewAsSkill("zhanshen_p"){
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
    NuozhanPass():TriggerSkill("nuozhan_p"){
        events << CardResponsed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
           return false;

        CardStar card_star = data.value<CardStar>();
        if(!card_star || !card_star->inherits("Jink"))
            return false;

        if(!player->isKongcheng() && player->askForSkillInvoke(objectName()))
            player->getRoom()->askForUseCard(player, "slash", "@askforslash");

        return false;
    }
};


class LijianPass: public OneCardViewAsSkill{
public:
    LijianPass():OneCardViewAsSkill("lijian_p"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("LijianPassCard");
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LijianPassCard *card = new LijianPassCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
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
    duel->setSkillName("lijian_p");
    duel->setCancelable(false);

    CardUseStruct use;
    use.from = from;
    use.to << to;
    use.card = duel;
    room->useCard(use);
}

class QingnangPass: public OneCardViewAsSkill{
public:
    QingnangPass():OneCardViewAsSkill("qingnang_p"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QingnangPassCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        QingnangPassCard *card = new QingnangPassCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};


QingnangPassCard::QingnangPassCard(){
    once = true;
    target_fixed = true;
}

void QingnangPassCard::use(Room *room, ServerPlayer *huatuo, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    room->playSkillEffect("qingnang_p");
    if(huatuo->isWounded()){
        RecoverStruct recover;
        recover.card = this;
        recover.who = huatuo;
        huatuo->getRoom()->recover(huatuo, recover);
    }else{
        huatuo->gainMark("qingnang_p_buff");
    }
}

class QingnangPassBuff: public PhaseChangeSkill{
public:
    QingnangPassBuff():PhaseChangeSkill("#qingnang_p"){
    }
    virtual bool onPhaseChange(ServerPlayer *huatuo) const{
        if(huatuo->getPhase() == Player::Start && huatuo->getMark("qingnang_p_buff") > 0){
            huatuo->loseAllMarks("qingnang_p_buff");
            if(huatuo->isWounded()){
                RecoverStruct recover;
                recover.who = huatuo;
                huatuo->getRoom()->recover(huatuo, recover);
                LogMessage log;
                log.type = "#QingnangPassBuff";
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
    YangshenPass():TriggerSkill("yangshen_p"){
        frequency = Compulsory;
        events << HpRecover ;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *huatuo, QVariant &data) const{
        Room *room = huatuo->getRoom();
        RecoverStruct recover = data.value<RecoverStruct>();
        if(recover.card && (recover.card->inherits("Peach"))){
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = huatuo;
            log.arg = objectName();
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
    MafeiPass():MasochismSkill("mafei_p"){

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
            from->addMark("mafei_p");
        }
    }
};

class MafeiPassBuff: public PhaseChangeSkill{
public:
    MafeiPassBuff():PhaseChangeSkill("#mafei_p"){
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return player->getPhase() == Player::Finish && player->getMark("mafei_p") > 0;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getHandcardNum() > 0){
            if (player->getHandcardNum() == 1){
                player->throwAllHandCards();
            }else{
                room->askForDiscard(player, "mafei", 1);
            }
            player->setMark("mafei_p", 0);
        }
        return false;
    }
};



class GuhuoPass: public OneCardViewAsSkill{
public:
    GuhuoPass():OneCardViewAsSkill("guhuo_p"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        int card_id = player->getMark("guhuo_p_mark");
        if(card_id > 0){
            const Card *card = Sanguosha->getCard(card_id);
            return card->isAvailable(player);
        }else
            return false;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        int card_id = Self->getMark("guhuo_p_mark");
        const Card *card = Sanguosha->getCard(card_id);
        return to_select->getFilteredCard()->getSuit() == card->getSuit();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        int card_id = Self->getMark("guhuo_p_mark");
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
    GuhuoPassMark():TriggerSkill("#guhuo_p"){
        events << CardUsed << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yuji, QVariant &data) const{
        Room *room = yuji->getRoom();
        static const QString mark = "guhuo_p_mark" ;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card){
                if(!use.card->isVirtualCard() && (use.card->getTypeId() == Card::Basic || (use.card->isNDTrick() && !use.card->inherits("Nullification")))){
                    room->setPlayerMark(yuji,mark,use.card->getId());
                }
                if(use.card->getSkillName() == "guhuo_p")
                    room->setPlayerMark(yuji,mark,0);
            }
        }else if(event == PhaseChange && yuji->getPhase() == Player::Finish){
            room->setPlayerMark(yuji,mark,0);
        }
        return false ;
    }
};

class BuguaPass: public PhaseChangeSkill{
public:
    BuguaPass():PhaseChangeSkill("bugua_p"){

    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && (target->getPhase() == Player::Start || target->getPhase() == Player::Finish);
    }

    virtual bool onPhaseChange(ServerPlayer *yuji) const{
        static QString flag = "bugua_p_used" ;
        if(yuji->hasFlag(flag) || !yuji->askForSkillInvoke(objectName()))
            return false;
        if(yuji->getPhase() == Player::Start)
            yuji->setFlags(flag);
        else if(yuji->getPhase() == Player::Finish)
            yuji->setFlags("-"+flag);
        Room *room = yuji->getRoom();
        yuji->drawCards(1);

        const Card *card = room->askForCard(yuji,".NTH!","@bugua_p-card");

        if(card)
            room->moveCardTo(card, NULL, Player::DrawPile);

        return false;
    }
};

class HuanshuPass: public TriggerSkill{
public:
    HuanshuPass():TriggerSkill("huanshu_p"){
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
                const Card *card = room->askForCard(yuji,".NTH!","@huanshu_p-card");
                if(card)
                    room->moveCardTo(card, damage.to, Player::Hand);
            }
        }
        return false;
    }
};

class HuangtianPass : public TriggerSkill{
public:
    HuangtianPass():TriggerSkill("huangtian_p"){
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
    LeijiPass():TriggerSkill("leiji_p"){
        frequency = Frequent;
        events << CardFinished;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *zhangjiao, QVariant &data) const{
        if(zhangjiao->getPhase() == Player::NotActive)
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        Room *room = zhangjiao->getRoom();
        if(use.card && use.card->getSuit() == Card::Spade && (use.card->inherits("TrickCard") || use.card->inherits("EquipCard")) && room->askForSkillInvoke(zhangjiao, objectName(), data)){
            ServerPlayer *target = room->askForPlayerChosen(zhangjiao,room->getAlivePlayers(),objectName());
            if(target){
                room->playSkillEffect(objectName());
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(spade):(.*)");
                judge.good = false;
                judge.reason = objectName();
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
    DajiPass():TriggerSkill("daji_p"){
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

class JiuchiPass: public OneCardViewAsSkill{
public:
    JiuchiPass():OneCardViewAsSkill("jiuchi_p"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Analeptic::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern.contains("analeptic");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Analeptic *analeptic = new Analeptic(card->getSuit(), card->getNumber());
        analeptic->setSkillName(objectName());
        analeptic->addSubcard(card->getId());

        return analeptic;
    }
};

class BenghuaiPass:public PassiveSkill{
public:
    BenghuaiPass():PassiveSkill("benghuai_p"){
        frequency = Compulsory;
        events << PhaseChange ;
    }

    virtual void onAcquire(ServerPlayer *player) const{
        Room *room = player->getRoom() ;
        player->setMark("_orginal_hp", player->getMaxHP());
        room->setPlayerProperty(player, "maxhp", player->getMaxHP() / 2);
    }

    virtual void onDetach(ServerPlayer *player) const{
        Room *room = player->getRoom() ;
        int ohp = player->getMark("_orginal_hp") ;
        if(ohp > 0){
            // TODO temp code
            if(player->isLord())
                ohp ++ ;
            room->setPlayerProperty(player, "maxhp", ohp);
        }

    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *dongzhuo, QVariant &data) const{
        if(dongzhuo->getPhase() == Player::Finish){
            Room *room = dongzhuo->getRoom();
            bool trigger_this = true ;
            QList<ServerPlayer *> players = room->getOtherPlayers(dongzhuo);
            foreach(ServerPlayer *player, players){
                if(dongzhuo->getHp() <= player->getHp()){
                    trigger_this = false;
                    break;
                }
            }
            if(trigger_this){
                room->playSkillEffect(objectName());
                room->setEmotion(dongzhuo, "bad");

                room->loseHp(dongzhuo);
            }
        }
        if(event == GameStart){
            PassiveSkill::trigger(event , dongzhuo , data) ;
        }
        return false;
    }
};

class BaonuePass: public TriggerSkill{
public:
    BaonuePass():TriggerSkill("baonue_p"){
        events << Damage;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *dongzhuo, QVariant &data) const{
        Room *room = dongzhuo->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.damage >= 2 && room->askForSkillInvoke(dongzhuo, objectName())){
            room->playSkillEffect(objectName());
            int n = damage.damage ;
            for(int i=0;i<n;i++){
                int card_id = room->drawCard();
                const Card *card = Sanguosha->getCard(card_id);
                room->moveCardTo(card, NULL, Player::Special, true);
                room->getThread()->delay();
                if(card->isBlack()){
                    room->obtainCard(dongzhuo, card_id);
                }else{
                    RecoverStruct recover;
                    recover.who = dongzhuo;
                    room->recover(dongzhuo, recover);
                }
            }
        }
        return false;
    }
};


class WeimuPass:public TriggerSkill{
public:
    WeimuPass():TriggerSkill("weimu_p"){
        events << CardEffected ;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *jiaxu, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        Room *room = jiaxu->getRoom();
        if(effect.from != jiaxu && !jiaxu->isKongcheng() && effect.card->inherits("TrickCard") && jiaxu->askForSkillInvoke(objectName())){
            QString suit_str = effect.card->getSuitString();
            QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
            QString prompt = QString("@@weimu_p:::%1").arg(suit_str);
            if(room->askForCard(jiaxu, pattern, prompt)){
                room->playSkillEffect(objectName());
                LogMessage log;
                log.type = "#WeimuPass";
                log.from = jiaxu;
                log.arg = effect.card->objectName();
                room->sendLog(log);
                jiaxu->drawCards(1);
                return true;
            }
        }
        return false ;
    }
};

class DumouPass:public TriggerSkill{
public:
    DumouPass():TriggerSkill("dumou_p"){
        events << CardEffect;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *jiaxu, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        Room *room = jiaxu->getRoom();
        if(effect.card->inherits("TrickCard") && !effect.multiple && effect.from != effect.to && !effect.to->isKongcheng() && jiaxu->askForSkillInvoke(objectName())){
            Card::Suit suit = room->askForSuit(jiaxu,"dumou");
            int card_id = effect.to->getRandomHandCardId();
            const Card *card = Sanguosha->getCard(card_id);

            room->showCard(effect.to, card_id);
            room->getThread()->delay();

            if((card->isRed() && (suit == Card::Heart || suit == Card::Diamond)) ||
                    (card->isBlack() && (suit == Card::Spade || suit == Card::Club))){
                jiaxu->gainMark("@dumou_p");
            }
            if(card->getSuit() == suit){
                DamageStruct damage;
                damage.from = jiaxu;
                damage.to = effect.to;
                room->damage(damage);
                jiaxu->gainMark("@dumou_p");
            }
        }
        return false ;
    }
};

class LuanwuPass: public ZeroCardViewAsSkill{
public:
    LuanwuPass():ZeroCardViewAsSkill("luanwu_p"){
    }

    virtual const Card *viewAs() const{
        return new LuanwuPassCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@dumou_p") >= 3;
    }
};

LuanwuPassCard::LuanwuPassCard(){
    target_fixed = true;
}

void LuanwuPassCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    source->loseMark("@dumou_p",3);
    room->broadcastInvoke("animate", "lightbox:$luanwu");
    room->playSkillEffect("luanwu");

    QList<ServerPlayer *> players = room->getOtherPlayers(source);
    foreach(ServerPlayer *player, players){
        if(player->isAlive())
            room->cardEffect(this, source, player);
    }
}

void LuanwuPassCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    QList<ServerPlayer *> players = room->getOtherPlayers(effect.to);
    QList<int> distance_list;
    int nearest = 1000;
    foreach(ServerPlayer *player, players){
        int distance = effect.to->distanceTo(player);
        distance_list << distance;

        nearest = qMin(nearest, distance);
    }

    QList<ServerPlayer *> luanwu_targets;
    int i;
    for(i=0; i<distance_list.length(); i++){
        if(distance_list.at(i) == nearest && effect.to->canSlash(players.at(i))){
            luanwu_targets << players.at(i);
        }
    }

    const Card *slash = NULL;
    if(!luanwu_targets.isEmpty() && (slash = room->askForCard(effect.to, "slash", "@luanwu-slash"))){
        ServerPlayer *to_slash;
        if(luanwu_targets.length() == 1)
            to_slash = luanwu_targets.first();
        else
            to_slash = room->askForPlayerChosen(effect.to, luanwu_targets, "luanwu");
        room->cardEffect(slash, effect.to, to_slash);
    }else
        room->loseHp(effect.to);
}

class BaimaPass:public TriggerSkill{
public:
    BaimaPass():TriggerSkill("baima_p"){
        events << PhaseChange;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *gongsun, QVariant &) const{
        if(gongsun->getPhase() == Player::Play){
            Room *room = gongsun->getRoom();
            int n = 0 ;
            if(gongsun->getDefensiveHorse() != NULL || gongsun->getOffensiveHorse() != NULL)
                n ++ ;
            if(n > 0 && room->askForSkillInvoke(gongsun, objectName())){
                LogMessage log;
                log.type = "#TriggerDrawSkill";
                log.from = gongsun;
                log.arg = objectName();
                log.arg2 = QString::number(n);
                room->sendLog(log);
                gongsun->drawCards(n);
            }
        }
        return false;
    }
};

class YicongPass: public DistanceSkill{
public:
    YicongPass():DistanceSkill("yicong_p"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        if(from->hasSkill(objectName()) && from->getHp() >= 3)
            correct -= (from->getHp() - 2);
        if(to->hasSkill(objectName()) && to->getHp() < 3)
            correct +=  (3 - to->getHp()) ;

        return correct;
    }
};

class YicongPassEffect:public TriggerSkill{
public:
    YicongPassEffect():TriggerSkill("#yicong_p"){
        events << HpRecover << HpLost << Damaged;
    }

    virtual int getPriority() const{
        return -3;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *gongsun, QVariant &data) const{
        Room *room = gongsun->getRoom();
        if(event == HpRecover){
            RecoverStruct recover = data.value<RecoverStruct>();
            if(gongsun->getHp() >= 3 && gongsun->getHp() - recover.recover < 3){
                room->playSkillEffect("yicong",1);
            }
        }else{
            int lost = 0;
             if(event == HpLost){
                lost = data.value<int>();
             }else if(event == Damaged){
                DamageStruct damage = data.value<DamageStruct>();
                lost = damage.damage;
             }
            if(gongsun->getHp() < 3 && gongsun->getHp() + lost >= 3)
            room->playSkillEffect("yicong",2);
        }
        return false;
    }
};

class ZhuiliePass: public TriggerSkill{
public:
    ZhuiliePass():TriggerSkill("zhuilie_p"){
        events << SlashHit;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *gongsun, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        QStringList horses;
        if(effect.to->getDefensiveHorse())
            horses << "dhorse";
        if(effect.to->getOffensiveHorse())
            horses << "ohorse";

        if(horses.isEmpty())
            return false;

        Room *room = gongsun->getRoom();
        if(!gongsun->getWeapon() || gongsun == effect.to || !gongsun->askForSkillInvoke(objectName(), data))
            return false;

        QString horse_type;
        if(horses.length() == 2)
            horse_type = room->askForChoice(gongsun, objectName(), horses.join("+"));
        else
            horse_type = horses.first();

        if(horse_type == "dhorse")
            room->moveCardTo(effect.to->getDefensiveHorse(),gongsun,Player::Hand);
        else if(horse_type == "ohorse")
            room->moveCardTo(effect.to->getOffensiveHorse(),gongsun,Player::Hand);

        return false;
    }
};

class WushenPass: public TriggerSkill{
public:
    WushenPass():TriggerSkill("wushen_p"){
        events << SlashEffect << SlashProceed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *shenguanyu, QVariant &data) const{
        Room *room = shenguanyu->getRoom();

        if(event == SlashEffect){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();

            switch(effect.slash->getSuit()){
            case Card::Heart:{
                if(shenguanyu->isWounded() && shenguanyu->askForSkillInvoke(objectName(), QVariant::fromValue(effect.to))){
                    room->playSkillEffect(objectName());
                    RecoverStruct recover;
                    recover.who = shenguanyu;
                    room->recover(shenguanyu, recover);
                }
                break;
            }
            case Card::Diamond:{
                if(shenguanyu->askForSkillInvoke(objectName(), QVariant::fromValue(effect.to))){
                    room->playSkillEffect(objectName());
                    shenguanyu->drawCards(1);
                }
                break;
            }
            case Card::Club:{
                if(effect.to && effect.to->isAlive() && !effect.to->isKongcheng() && shenguanyu->askForSkillInvoke(objectName(), QVariant::fromValue(effect.to))){
                    room->playSkillEffect(objectName());
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
                    room->playSkillEffect(objectName());
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
    WuhunPass():TriggerSkill("wuhun_p"){
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
    QishenPass():DistanceSkill("qishen_p"){
        frequency = Compulsory;
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
    SheliePass():PhaseChangeSkill("shelie_p"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *shenlumeng) const{
        if(shenlumeng->getPhase() == Player::Draw){
            Room *room = shenlumeng->getRoom();

            if(shenlumeng->askForSkillInvoke(objectName())){
                shenlumeng->drawCards(2);
                room->playSkillEffect(objectName());
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
        return true ;
    }
    virtual bool willThrow() const{
        return false;
    }
};

PassPackage::PassPackage()
    :Package("pass")
{
    General *shizu = new General(this,"shizu_e","evil",3, true, true);
    shizu->addSkill(new ShiqiPass);

    General *gongshou = new General(this,"gongshou_e","evil",3, false, true);
    gongshou->addSkill(new QianggongPass);
    gongshou->addSkill(new Skill("shenshe_p"));

    General *jianshi = new General(this,"jianshi_e","evil",3, false, true);
    jianshi->addSkill(new PojiaPass);
    jianshi->addSkill(new ZhanshangPass);

    General *qibing = new General(this,"qibing_e","evil",3, true, true);
    qibing->addSkill(new QishuPass);
    qibing->addSkill(new XunmaPass);

    General *huwei = new General(this,"huwei_e","evil",3, true, true);
    huwei->addSkill(new ChenwenPass);
    huwei->addSkill(new ZhongzhuangPass);

    General *leishi = new General(this,"leishi_e","evil",3, true, true);
    leishi->addSkill(new DianjiPass);
    leishi->addSkill(new LeilingPass);

    General *daomo = new General(this,"daomo_e","evil",3, true, true);
    daomo->addSkill(new LianzhanPass);
    daomo->addSkill(new DouzhiPass);

    General *wuniang = new General(this,"wuniang_e","evil",3, false, true);
    wuniang->addSkill(new Skill("guwu_p"));
    wuniang->addSkill(new XiangxiaoPass);

    skills << new TipoPass << new QuanhengPass
           << new DuanyanPass << new XiongziPass << new QiangongPass
           << new ZhaoliePass << new JunshenPass << new BadaoPass << new ShoujiePass << new HonglangPass << new GuoshiPass << new WujiPass
                << new Skill("tieji_p")
           << new WanghouPass << new QiangyunPass << new XiaoxiongPass
           << new HujiangPass << new AoguPass << new ZhongyiPass  << new CaiqingPass << new DuoyiPass
           << new LiangjiangPass << new FenyongPass << new WuliePass << new QuanmouPass << new DuduPass
             << new ZhejiePass << new DanmouPass
           << new FeijiangPass << new ZhunuePass << new DoushenPass
              ;

    skills << new HujiangPassBuff ;
    related_skills.insertMulti("hujiang_p","#hujiang_p");

    addMetaObject<DuanyanPassCard>();
    addMetaObject<QuanhengPassCard>();
    addMetaObject<XunmaPassCard>();
    addMetaObject<DianjiPassCard>();

    addMetaObject<LiangjiangPassCard>();
    addMetaObject<QuanmouPassCard>();
    addMetaObject<ZhejiePassCard>();
    addMetaObject<HujiangPassCard>();

    addMetaObject<LiegongPassCard>();
    addMetaObject<JianhunPassCard>();
    addMetaObject<TuodaoPassCard>();
    addMetaObject<DuanhePassCard>();
    addMetaObject<BeifaPassCard>();
    addMetaObject<LuoyiPassCard>();
    addMetaObject<FangzhuPassCard>();
    addMetaObject<FanjianPassCard>();
    addMetaObject<KurouPassCard>();
    addMetaObject<ZhaxiangPassCard>();
    addMetaObject<JieyinPassCard>();
    addMetaObject<YuyuePassCard>();
    addMetaObject<LijianPassCard>();
    addMetaObject<QingnangPassCard>();
    addMetaObject<LuanwuPassCard>();

    patterns[".NTH!"] = new NothrowHandcardsPattern;
    patterns[".NT!"] = new NothrowPattern;
}
ADD_PACKAGE(Pass)

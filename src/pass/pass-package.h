#ifndef PASSPACKAGE_H
#define PASSPACKAGE_H

#include "gamerule.h"
#include "scenario.h"
#include "maneuvering.h"

class DuanyanPassCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DuanyanPassCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QuanhengPassCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QuanhengPassCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class RendePassCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE RendePassCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class JijiangPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JijiangPassCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class TuodaoPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TuodaoPassCard();
     virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class DuanhePassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DuanhePassCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class BeifaPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BeifaPassCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class LiegongPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LiegongPassCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class JianhunPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JianhunPassCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class LuoyiPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LuoyiPassCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class TuxiPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TuxiPassCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class FangzhuPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FangzhuPassCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ZhihengPassCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhihengPassCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class FanjianPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FanjianPassCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class KurouPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE KurouPassCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ZhaxiangPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhaxiangPassCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};


class JieyinPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JieyinPassCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class YuyuePassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YuyuePassCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LijianPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LijianPassCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class QingnangPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QingnangPassCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class LuanwuPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LuanwuPassCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DianjiPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DianjiPassCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XunmaPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XunmaPassCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class PassPackage: public Package{
    Q_OBJECT

public:
    PassPackage();
};

#endif // PASSPACKAGE_H

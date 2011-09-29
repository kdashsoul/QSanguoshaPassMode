#ifndef PASSMODESCENARIO_H
#define PASSMODESCENARIO_H

#include "gamerule.h"
#include "scenario.h"
#include "maneuvering.h"

class DuanyanCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DuanyanCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QuanhengCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QuanhengCard();
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

class ZhaxiangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhaxiangCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};


class JieyinPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JieyinPassCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class YuyueCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YuyueCard();
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

class YaoshuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YaoshuCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class PassModeItemCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE PassModeItemCard();

    enum Item{
        Warlock,
        Recover,
        Knight,
        Strong,
        Handsome,
        OnePeach,
    };

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;

    void getItemEffect(Room *room, ServerPlayer *source, const QString &item_str) const;
private:
    QMap<QString, PassModeItemCard::Item> items;
};

class PassModeScenario : public Scenario{
    Q_OBJECT

public:
    explicit PassModeScenario();

    virtual bool exposeRoles() const;
    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual void onTagSet(Room *room, const QString &key) const;
    virtual bool generalSelection() const;
private:
    void addGeneralAndSkills();
};

class PassModeRule: public ScenarioRule{
public:
    PassModeRule(Scenario *scenario);

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;
private:
    QMap<QString, int> item_lottery;
};

struct SaveDataStruct{
    SaveDataStruct();

    enum WrongVersion{
        DifferentSkills,
        UnknownLordName,
        ExceptMaxHp,

        VersionConfirmed,
    };

    int size;

    int stage;
    int exp;
    int nirvana;
    int lord_maxhp;

    int times;

    QString lord;
    QString skills;
    QString reward_list;

    bool read_success;

    bool canRead() const;
    bool checkDataFormat() const;
    QString getWrongType(WrongVersion error) const;
private:
    QMap<WrongVersion, QString> error_type;

    static int default_size;
};

class PassMode: public GameRule{
    Q_OBJECT

public:
    PassMode(QObject *parent);

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;

    void stageStartDraw(Room *room, ServerPlayer *player = NULL) const;
    void initGameStart(ServerPlayer *player) const;
    void initNextStageStart(ServerPlayer *player) const;
    void setLoadedStageInfo(Room *room) const;
    void setNextStageInfo(Room *room, int stage, bool save_loaded = false) const;
    bool goToNextStage(ServerPlayer *player, int stage) const;

    void setTimesDifficult(Room *room) const;
    void proceedSpecialReward(Room *room,const QString pattern, QVariant data) const;
    bool resetPlayerSkills(SaveDataStruct *savedata) const;

    void getIntoShop(Room *room) const;
    void buyItem(Room *room) const;
    void sellItem(Room *room) const;

    bool getRewardItem(Room *room, QString &item_name) const;
    bool sellRewardItem(Room *room, QString &item_name) const;

    bool askForLearnSkill(ServerPlayer *lord) const;
    bool askForLearnHiddenSkill(ServerPlayer *lord, QString &skills, int &min_exp) const;

    bool askForLoadData(Room *room) const;
    bool askForSaveData(Room *room, int told_stage) const;
    bool askForSaveData(SaveDataStruct *save) const;
    SaveDataStruct *askForReadData() const;
    SaveDataStruct *catchSaveInfo(Room *room, int stage = -1) const;

    SaveDataStruct::WrongVersion checkDataVersion(SaveDataStruct *savedata) const;
    bool sendWrongVersionLog(Room *room, SaveDataStruct *savedata) const;
private:
    QList<QString> enemy_list;
    QMap<QString, int> exp_map;
    QMap<QString, int> skill_map;
    QMap<QString, QPair<QString, int> > skill_map_hidden;

    QMap<QString, QString> hidden_reward;
    QMap<QString, int> shop_items;

    static QString version;

    mutable jmp_buf env;
};

class HeroPackage: public Package{
    Q_OBJECT

public:
    HeroPackage();
};

typedef SaveDataStruct *SaveDataStar;
Q_DECLARE_METATYPE(SaveDataStruct);
Q_DECLARE_METATYPE(SaveDataStar);
#endif // PASSMODESCENARIO_H

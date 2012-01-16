#ifndef PASSMODESCENARIO_H
#define PASSMODESCENARIO_H

#include "scenario.h"
#include "gamerule.h"
#include "engine.h"

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
};

struct SaveDataStruct{
    SaveDataStruct();

    int size;

    int stage;
    int exp;
    int lord_maxhp;

    int times;

    QString lord;
    QString skills;
    QString reward_list;

    bool read_success;

    bool canRead() const;
    bool checkDataFormat() const;

    enum WrongType{
        Struct_Right,
        Ex_HP,
        Ex_Skills,
        Ex_Exp
    };
private:
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
    bool resetPlayerSkills(SaveDataStruct *savedata) const;

    bool askForLearnSkill(ServerPlayer *lord) const;
    void getLearnSkillInfo(ServerPlayer *lord, QString &skills, int &min_exp) const;

    bool askForLoadData(Room *room) const;
    bool askForSaveData(Room *room, int told_stage) const;
    bool askForSaveData(SaveDataStruct *save) const;
    SaveDataStruct *askForReadData() const;
    SaveDataStruct *catchSaveInfo(Room *room, int stage = -1) const;
    SaveDataStruct::WrongType checkDataValid(SaveDataStruct *savedata) const;

    static const QString default_hero;
private:
    QList<QString> enemy_list;
    QMap<QString, int> exp_map;
    QMap<QString, int> skill_map;

    QMap<QString, QVariant> spec_general;

    QMap<QString, QString> hidden_reward;

    static const QString version;
    static const QString savePath;
    mutable jmp_buf env;
};

typedef SaveDataStruct *SaveDataStar;
Q_DECLARE_METATYPE(SaveDataStruct)
Q_DECLARE_METATYPE(SaveDataStar)
#endif // PASSMODESCENARIO_H

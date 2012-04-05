#ifndef PASSMODESCENARIO_H
#define PASSMODESCENARIO_H

#include "scenario.h"
#include "gamerule.h"
#include "client.h"
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

    QStringList roomTags;
    QStringList playerMarks;

    QString general_name;
    QStringList skills;
    QStringList skills_enhance;

    bool read_success;
};

struct SkillAttrStruct{
    SkillAttrStruct();
    QList<int> values ;
    int limit_times ;

    int getLimitTimes() const ;
    int getValue(const int level = 1) const;
};

class PassMode: public GameRule{
    Q_OBJECT

public:
    PassMode(QObject *parent);

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;

    static QStringList getNeedSaveRoomTagName();
    static QStringList getNeedSavePlayerMarkName();

    void stageStartDraw(Room *room, ServerPlayer *player = NULL) const;
    void initGameStart(ServerPlayer *player) const;
    void initNextStageStart(ServerPlayer *player) const;
    void setNextStageInfo(Room *room,bool save_loaded = false) const;
    bool goToNextStage(Room *room) const;

    void resetPlayer(ServerPlayer *player) const;

    bool askForLoadData(Room *room) const;
    bool askForSaveData(Room *room) const;
    bool askForSaveData(SaveDataStruct *save) const;
    SaveDataStruct *askForReadData() const;
    SaveDataStruct *catchSaveInfo(Room *room) const;

    static QMap<QString, int> getExpMap();
    static QStringList getStageList() ;
    static QMap<QString, SkillAttrStruct *> getSkillMap();
    static QMap<QString, QStringList> getGeneralMap();

private:
    QMap<QString, QString> hidden_reward;

    static const QString version;
    static const QString savePath;

    mutable jmp_buf env;
};

typedef SaveDataStruct *SaveDataStar;
Q_DECLARE_METATYPE(SaveDataStruct)
Q_DECLARE_METATYPE(SaveDataStar)
#endif // PASSMODESCENARIO_H

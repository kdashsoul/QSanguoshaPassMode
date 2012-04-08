#ifndef PASSMODESCENARIO_H
#define PASSMODESCENARIO_H

#include "scenario.h"
#include "gamerule.h"
#include "client.h"
#include "engine.h"

struct SaveDataStruct{
    SaveDataStruct();

    QMap<QString,QVariant> roomTags;
    QStringList playerMarks;

    QString general_name;
    QStringList skills;
    QStringList skills_enhance;
    QStringList abilities;

    bool read_success;
};

struct SkillAttrStruct{
    SkillAttrStruct();
    QList<int> values ;
    int limit_times ;

    int getLimitTimes() const ;
    int getValue(const int level = 1) const;
};

class PassModeRule: public ScenarioRule{
Q_OBJECT
public:
    PassModeRule(Scenario *scenario);

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;
    void loadStage(QString path);
    void addNPC(QString feature);
    void assign(QStringList &generals, QStringList &roles) const;
    QStringList existedGenerals() const;
private:
    QList< QMap<QString, QString> > players;
    int stage ;
};

class PassMode: public GameRule{
    Q_OBJECT

public:
    PassMode(QObject *parent);

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;

    static QStringList getNeedSaveRoomTagName();
    static QStringList getNeedSavePlayerMarkName();
    int getStage() const;

    void stageStartDraw(Room *room) const;
    void initGameStart(ServerPlayer *player) const;
    void initNextStageStart(ServerPlayer *player) const;
    void setNextStageInfo(Room *room,bool save_loaded = false) const;
    bool goToNextStage(Room *room) const;

    void resetPlayer(ServerPlayer *player) const;

    static bool removeSaveData();
    static bool loadData(Room *room,SaveDataStruct *save_data = NULL);
    static bool askForSaveData(Room *room);
    static SaveDataStruct *getSaveData();
    static SaveDataStruct* catchSaveInfo(Room *room);

    static QMap<QString, int> getExpMap();
    static QStringList getStageList() ;
    static QMap<QString, SkillAttrStruct *> getSkillMap();
    static QMap<QString, QStringList> getGeneralMap();
    static const int maxStage;
private:
    QMap<QString, QString> hidden_reward;

    static const QString version;
    static const QString savePath;

    mutable jmp_buf env;
};

class PassModeScenario : public Scenario{
    Q_OBJECT

public:
    explicit PassModeScenario(const QString &name);
    void setupStage(QString name) const;
    virtual void assign(QStringList &generals, QStringList &roles) const
    {
        PassModeRule *rule = qobject_cast<PassModeRule*>(getRule());
        rule->assign(generals,roles);
    }
    virtual int getPlayerCount() const
    {
        QStringList generals,roles;
        assign(generals,roles);
        return roles.length();
    }
    virtual void onTagSet(Room *room, const QString &key) const;
private:
    void addGeneralAndSkills();
};

typedef SaveDataStruct *SaveDataStar;
Q_DECLARE_METATYPE(SaveDataStruct)
Q_DECLARE_METATYPE(SaveDataStar)
#endif // PASSMODESCENARIO_H

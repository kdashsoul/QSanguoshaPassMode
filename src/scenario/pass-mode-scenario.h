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

    QString skill_name ;
    QList<int> values ;
    int limit_times ;
    QMap<int,int> limit_plus ;

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
    void initGameStart(Room *room) const;
    QString getPlayerProp(ServerPlayer *player,QString prop_name,QString default_value = "") const;
    QMap<QString, QString> getPlayerProps(ServerPlayer *player) const;
private:
    QList< QMap<QString, QString> > players;
    int stage ;
};

class PassMode{
public:
    PassMode();

    static QStringList getNeedSaveRoomTagNames();
    static QStringList getNeedSavePlayerMarkNames();
    static QStringList getEplTagNames();
    static int getScore(Room *room);
    static int getFinalScore(Room *room);
    static QChar getFinalRank(int score);
    int getStage() const;

    static bool removeSaveData();
    static bool loadData(Room *room,SaveDataStruct *save_data = NULL);
    static bool saveData(Room *room,SaveDataStruct *save_data = NULL);
    static SaveDataStruct *getSaveData();
    static SaveDataStruct* catchSaveInfo(Room *room);
    static bool canUseEnhancedSkill(ServerPlayer *player, const QString &skill_name, int index);

    static QMap<QString, SkillAttrStruct *> getSkillMap();
    static QMap<QString, QStringList> getGeneralMap();
    static const int maxStage;
private:
    static const QString version;
    static const QString savePath;

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

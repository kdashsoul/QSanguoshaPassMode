sgs.card_base_value = {
	Slash = 3 ,
	ThunderSlash = 3.3 ,
	FireSlash = 3.9 ,
	Jink = 4.5 ,
	Analeptic = 5 ,
	Peach = 9 ,

    ExNihilo=12,
    Snatch=9,
    Nullification=9,
    Indulgence=8,
    SupplyShortage=8,
    Duel=6.5,
    Dismantlement=5.6,
    IronChain = 5.5,
    FireAttack=5,
    Collateral=5,
    ArcheryAttack=4.5,
    SavageAssault=3.9,
    AmazingGrace=3,
    Lightning = 2 ,
	
	Crossbow = 7,
	DoubleSword = 5 ,
	QinggangSword = 5 ,
	Blade = 6 ,
	Spear = 5 ,
	Axe = 7 ,
	Halberd = 5 ,
	KylinBow = 6 ,
	Fan = 5 ,
	GudingBlade = 5,
	IceSword = 5 ,
	YitianSword = 7 ,

	EightDiagram = 10 ,
	RenwangShield = 10 ,
	Vine = 8 ,
	SilverLion = 7,
	
	DefensiveHorse = 8 ,
	OffensiveHorse = 5 ,
	
	Shit = -10,
}

sgs.weapon_range =
{	
	Crossbow = 1,
	Blade = 3,
	Spear = 3,
	DoubleSword =2,
	QinggangSword=2,
	Axe=3,
	KylinBow=5,
	Halberd=4,
	IceSword=2,
	Fan=4,
	MoonSpear=3,
	GudingBlade=2,	
}

sgs.ai_use_value =
{
--skill cards
		XinzhanCard = 4.4,
        TianyiCard = 8.5,
        XianzhenCard = 9.2,
        XianzhenSlashCard = 9.2,
        HuangtianCard = 8.5,
        JijiangCard=8.5,
        DimengCard=3.5,
		JujianCard=6.7,
		QiangxiCard=2.5,
		LijianCard=8.5,
		RendeCard=8.5,
		MinceCard=5.9,
		JuejiCard=6,
--normal cards
        ExNihilo=10,
        
        Snatch=9,
        Collateral=8.8,
        
        Indulgence=8,
        SupplyShortage=7,
        IronChain = 6.4,
        
        Peach = 6,
        Dismantlement=5.6,


        --retain_value=5
        
        FireAttack=4.8,
        
        
        FireSlash = 4.4,
        ThunderSlash = 4.5,
        Slash = 4.6,
        
        ArcheryAttack=3.8,
        SavageAssault=3.9,
        Duel=3.7,
        
        
        AmazingGrace=3,
        
        --special
        Analeptic = 5.98,
        Jink=8.9,
}

sgs.ai_use_priority = {
--priority of using an active card
--special
		SilverLion = 10,
        Lightning=9,
--skill cards
		XinzhanCard = 9.2,
        TianyiCard = 10,
		JieyinCard = 4.2,
        HuangtianCard = 10,
        XianzhenCard = 9.2,
        XianzhenSlashCard = 2.6,
        JijiangCard = 2.4,
        DimengCard=2.3,
		LijianCard = 4,
		QingnangCard=4.2,
		RendeCard= 5.8,
		MingceCard=4,
		JujianCard = 5.6,
		ZhihengCard = 0.5,
--
        Collateral=5.8,
        Peach = 4.1,

        Dismantlement=4.5,
        Snatch=4.3,
        ExNihilo=4.6,

        ArcheryAttack=3.5,
        SavageAssault=3.5,

        
        Duel=2.9,
        IronChain = 2.8,

        Analeptic = 2.7,

        FireSlash = 2.6,
        ThunderSlash = 2.5,
        Slash = 2.4,

        FireAttack=2,
        AmazingGrace=1.5,
		GodSalvation=1
		
        --god_salvation
        --deluge
        --supply_shortage
        --earthquake
        --indulgence
        --mudslide
        --lightning
        --typhoon
}

sgs.ai_keep_value = {
    Shit = 12,
    Peach = 10,
    Analeptic = 9,
    Jink = 8,
    Nullification = 7,
    ExNihilo=6,
    Slash = 4,
    ThunderSlash = 4.5,
    FireSlash = 5,
    AmazingGrace=-1,
    Lightning=-1,
}

sgs.zhangfei_keep_value = 
{
Peach = 6,
Analeptic = 5.8,
Jink = 5.7,
FireSlash = 5.6,
ThunderSlash = 5.5,
Slash = 5.4,
ExNihilo = 4.7
}

sgs.zhaoyun_keep_value = 
{
Peach = 6,
Analeptic = 5.8,
Jink = 5.7,
FireSlash = 5.7,
ThunderSlash = 5.6,
Slash = 5.5,
ExNihilo = 4.7
}

sgs.huangyueying_keep_value = 
{
Peach 		= 6,
Analeptic 	= 5.9,
ExNihilo	= 5.7,
Snatch 		= 5.7,
Dismantlement = 5.6,
IronChain 	= 5.5,
SavageAssault=5.4,
Duel 		= 5.3,
ArcheryAttack = 5.2,
AmazingGrace = 5.1,
Collateral 	= 5,
FireAttack	=4.9
}

sgs.sunshangxiang_keep_value = 
{
Peach = 6,
Jink = 5.2,
Vine=5.1,
RenwangShield=5.1,
EightDiagram=5.1,
Crossbow = 5,
Blade = 5,
Spear = 5,
DoubleSword =5,
QinggangSword=5,
Axe=5,
KylinBow=5,
Halberd=5,
IceSword=5,
Fan=5,
MoonSpear=5,
GudingBlade=5,
DefensiveHorse = 5,
OffensiveHorse = 5
}
sgs.xiahouyuan_keep_value = sgs.sunshangxiang_keep_value

sgs.dianwei_keep_value = 
{
Peach = 6,
Jink = 5.1,
Crossbow = 5,
Blade = 5,
Spear = 5,
DoubleSword =5,
QinggangSword=5,
Axe=5,
KylinBow=5,
Halberd=5,
IceSword=5,
Fan=5,
MoonSpear=5,
GudingBlade=5,
OffensiveHorse = 5
}

sgs.zhangjiao_keep_value = 
{
Peach = 6,
Jink = 5.9 ,
Analeptic = 5.8
}

sgs.xiaoqiao_suit_value = 
{
spade = 5,
heart = 5
}

sgs.zhangjiao_suit_value = 
{
spade = 4.9,
club = 3.2
}

sgs.huatuo_suit_value = 
{
heart = 6,
diamond = 6
}

sgs.zhenji_suit_value = 
{
spade = 4.1,
club = 4.2
}

sgs.fazheng_suit_value = 
{
heart = 3.9
}

sgs.simayi_suit_value = 
{
heart = 3.9,
club = 3.8,
spade = 3.5
}

sgs.daqiao_suit_value = 
{
diamond = 4.9
}

sgs.xuhuang_suit_value = 
{
spade = 3.9,
club = 3.9
}

sgs.ganning_suit_value = 
{
spade = 3.9,
club = 3.9
}

sgs.wolong_suit_value = 
{
spade = 3.9,
club = 3.9
}

sgs.dongzhuo_suit_value = 
{
spade = 4.6,
}

sgs.zhouyu_suit_value = 
{
diamond = -1
}

--dongzhuo
jiuchi_skill={}
jiuchi_skill.name="jiuchi"
table.insert(sgs.ai_skills,jiuchi_skill)
jiuchi_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards,true)

	for _,acard in ipairs(cards) do
		if (acard:getSuit() == sgs.Card_Spade) then
			card = acard
			break
		end
	end
	
    if not card then return nil end
	local number = card:getNumberString()
    local card_id = card:getEffectiveId()
    local card_str = ("analeptic:jiuchi[spade:%s]=%d"):format(number, card_id)
	local analeptic = sgs.Card_Parse(card_str)
	
    assert(analeptic)
    return analeptic
end

sgs.ai_skill_invoke.baonue = function(self, data)
	return self.player:getRole() == "loyalist" --or (self.player:getRole() == "renegade" and self.room:getLord():getHp() < 3)
end

--xuhuang
duanliang_skill={}
duanliang_skill.name="duanliang"
table.insert(sgs.ai_skills,duanliang_skill)
duanliang_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("he")	
    cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards,true)
	for _,acard in ipairs(cards)  do
		if (acard:isBlack()) and (acard:inherits("BasicCard") or acard:inherits("EquipCard")) and (self:getUseValue(acard)<(sgs.ai_use_value["SupplyShortage"] or 0)) then
			card = acard
			break
		end
	end
	
    if not card then return nil end
    local suit = card:getSuitString()
	local number = card:getNumberString()
    local card_id = card:getEffectiveId()
    local card_str = ("supply_shortage:duanliang[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
		
    assert(skillcard)
    return skillcard
end

--lusu
dimeng_skill={}
dimeng_skill.name="dimeng"
table.insert(sgs.ai_skills,dimeng_skill)
dimeng_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("DimengCard") then return end
    return sgs.Card_Parse("@DimengCard=.")
end


sgs.ai_skill_use_func["DimengCard"]=function(card,use,self)
    local cardNum=self.player:getHandcardNum()
	
	self:sort(self.enemies,"handcard")
	self:sort(self.friends_noself,"handcard")
	
	--local lowest_enemy=self.enemies[1]
	local lowest_friend=self.friends_noself[1]
	
	self:sort(self.enemies,"expect")
	if lowest_friend then
		for _,enemy in ipairs(self.enemies) do 
			local hand1=enemy:getHandcardNum()
			local hand2=lowest_friend:getHandcardNum()
			--local hand3=lowest_enemy:getHandcardNum()
			if (hand1 > hand2) then 
				if (hand1-hand2) <= cardNum then 
					use.card=card
					if use.to then 
						use.to:append(enemy)
						use.to:append(lowest_friend)
						return 
					end
				end
			end
		end
	end
end


function SmartAI:getBeggar()
	local least = math.huge
	local players = self.room:getOtherPlayers(self.player)
	for _, player in sgs.qlist(players) do
		least = math.min(player:getHandcardNum(), least)		
	end

	self:sort(self.friends_noself)
	for _, friend in ipairs(self.friends_noself) do
		if friend:getHandcardNum() == least then			
			return friend
		end
	end
end

sgs.ai_skill_invoke.haoshi = function(self, data)
	if self.player:getHandcardNum() <= 1 then
		return true
	end

	if self:getBeggar() then
		return true
	else
		return false
	end
end

sgs.ai_skill_use["@@haoshi!"] = function(self, prompt)
	local beggar = self:getBeggar()
	
	local cards = self.player:getHandcards()
	local n = math.floor(self.player:getHandcardNum()/2)
	local card_ids = {}
	for i=1, n do
		table.insert(card_ids, cards:at(i-1):getEffectiveId())
	end
	
	return "@HaoshiCard=" .. table.concat(card_ids, "+") .. "->" .. beggar:objectName()
end

--caopi
sgs.ai_skill_invoke.xingshang = true

sgs.ai_skill_use["@@fangzhu"] = function(self, prompt)
	self:sort(self.friends_noself)
	local target
	for _, friend in ipairs(self.friends_noself) do
		if not friend:faceUp() then
			target = friend
			break
		end

		if friend:hasSkill("jushou") and friend:getPhase() == sgs.Player_Play then			
			target = friend
			break
		end
	end

	if not target then
		local x = self.player:getLostHp()
		if x >= 3 then
			target = self:getOneFriend()
		else
			self:sort(self.enemies)
			for _, enemy in ipairs(self.enemies) do
				if enemy:faceUp() and not enemy:hasSkill("jushou") then
					target = enemy
					break
				end
			end
		end
	end

	if target then
		return "@FangzhuCard=.->" .. target:objectName()
	else
		return "."
	end
end

--FIX yuanshu
sgs.ai_skill_invoke.songwei = function(self, data)
    return self:isFriend(self.room:getLord())
end

-- sunjian
sgs.ai_skill_choice.yinghun = function(self, choices)
	if self:isFriend(self.yinghun) then
		return "dxt1"
	else
		return "d1tx"
	end
end

sgs.ai_skill_use["@@yinghun"] = function(self, prompt)       
	local x = self.player:getLostHp()
	if x == 1 and #self.friends == 1 then
		return "."
	end

	if #self.friends > 1 then
		self:sort(self.friends, "chaofeng")
		self.yinghun = self:getOneFriend()
	else
		self:sort(self.enemies, "chaofeng")
		self.yinghun = self.enemies[1]
	end
	
	if self.yinghun then
		return "@YinghunCard=.->" .. self.yinghun:objectName()
	else
		return "."
	end
end

--menghuo
sgs.ai_skill_invoke["zaiqi"] = function(self, data)
	return self.player:getLostHp() >= 2
end

--zhurong
sgs.ai_skill_invoke.lieren = function(self, data)
	local damage = data:toDamage()
	if self:isFriend(damage.to) then return false end
	
	local mnum = 0
	local mcard = self:getMaxCard() 
	if mcard then mnum = mcard:getNumber() end
    if self:getOverflow() >= 0 or mnum >= 10 then return true end
    return false
end
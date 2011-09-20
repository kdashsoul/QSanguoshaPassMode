-- this scripts contains the AI classes for generals of fire package

-- wolong

sgs.ai_skill_invoke.bazhen = true

local huoji_skill={}
huoji_skill.name="huoji"
table.insert(sgs.ai_skills,huoji_skill)
huoji_skill.getTurnUseCard=function(self)
    local dummy_use={}
    dummy_use.isDummy=true
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	local card
	self:sortByCardNeed(cards)
	
	for _,acard in ipairs(cards) do
		if (acard:isRed()) and not acard:inherits("Peach") then--and (self:getUseValue(acard)<sgs.ai_use_value["FireAttack"]) then
			card = acard
			break
		end
	end
	
	if not card then return nil end
    local suit = card:getSuitString()
	local number = card:getNumberString()
    local card_id = card:getEffectiveId()
	local card_str = ("fire_attack:huoji[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	self:useCardFireAttack(skillcard, dummy_use)
	if dummy_use.card then return skillcard end
end

-- pangtong
sgs.ai_skill_invoke.niepan = function(self, data)
	local dying = data:toDying()
	local peaches = 1 - dying.who:getHp()

	local cards = self.player:getHandcards()
	local n = 0
	for _, card in sgs.qlist(cards) do
		if card:inherits "Peach" or card:inherits "Analeptic" then
			n = n + 1
		end
	end
	return n < peaches
end

local lianhuan_skill={}
lianhuan_skill.name="lianhuan"
table.insert(sgs.ai_skills,lianhuan_skill)
lianhuan_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	
	local card
	
	self:sortByUseValue(cards,true)
	
	for _,acard in ipairs(cards)  do
		if (acard:getSuit() == sgs.Card_Club) then--and (self:getUseValue(acard)<sgs.ai_use_value["IronChain"]) then
			card = acard
			break
		end
	end
	
    if not card then return nil end
	local number = card:getNumberString()
    local card_id = card:getEffectiveId()
    local card_str = ("iron_chain:lianhuan[club:%s]=%d"):format(number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	
    assert(skillcard)
    return skillcard
end


--yuanshao
local luanji_skill={}
luanji_skill.name="luanji"
table.insert(sgs.ai_skills,luanji_skill)
luanji_skill.getTurnUseCard=function(self)
	local first_found, second_found = false, false
	local first_card, second_card
	if self.player:getHandcardNum() >= 2 then
		local cards = self.player:getHandcards()
		local same_suit=false
		cards = sgs.QList2Table(cards)
		for _, fcard in ipairs(cards) do
			if not (fcard:inherits("Peach") or fcard:inherits("ExNihilo") or fcard:inherits("SavageAssault") or fcard:inherits("ArcheryAttack")) then
				first_card = fcard
				first_found = true
				for _, scard in ipairs(cards) do
					if first_card ~= scard and scard:getSuitString() == first_card:getSuitString() and not (scard:inherits("Peach") or scard:inherits("ExNihilo") or scard:inherits("SavageAssault") or scard:inherits("ArcheryAttack")) then
						second_card = scard
						second_found = true
						break
					end
				end
				if second_card then break end
			end
		end
	end
	
	if first_found and second_found then
		local luanji_card = {}
		local first_suit, first_number, first_id = first_card:getSuitString(), first_card:getNumberString(), first_card:getId()
		local second_suit, second_number, second_id = second_card:getSuitString(), second_card:getNumberString(), second_card:getId()
		local card_str = ("archery_attack:luanji[%s:%s]=%d+%d"):format(first_suit, first_number, first_id, second_id)
		local archeryattack = sgs.Card_Parse(card_str)
		assert(archeryattack)
		return archeryattack
	end
end
sgs.ai_skill_use_func["LuanjiCard"]=function(card,use,self)
	local vaa = sgs.Sanguosha:cloneCard("archery_attack", sgs.Card_NoSuit, 0)
	if self:getAoeValue(vaa) < 10 then return end
    use.card=card
end



--shuangxiong
sgs.ai_skill_invoke["shuangxiong"]=function(self,data)
    if self.player:isSkipped(sgs.Player_Play) or self.player:getHp() < 2 then
		return false
	end
    local handnum=0
    local cards=self.player:getCards("h")
    cards=sgs.QList2Table(cards)
    
    for _,card in ipairs(cards) do  
        if self:getUseValue(card)<8 then
			handnum=handnum+1
		end
    end
    
    handnum=handnum/2
    self:sort(self.enemies, "hp")
    for _, enemy in ipairs(self.enemies) do
        if (self:getSlashNumber(enemy)+enemy:getHp()<=handnum) and (self:getSlashNumber(self.player)>=self:getSlashNumber(enemy)) then return true end
    end
	
    return self.player:getHandcardNum()>=self.player:getHp()
end


local shuangxiong_skill={}
shuangxiong_skill.name="shuangxiong"
table.insert(sgs.ai_skills,shuangxiong_skill)
shuangxiong_skill.getTurnUseCard=function(self)

    if not self.player:getMark("shuangxiong") then return nil end
    local mark=self.player:getMark("shuangxiong")
    
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	
	local card
	
	self:sortByUseValue(cards,true)
	
	for _,acard in ipairs(cards)  do
		if (acard:isRed() and (mark==2)) or (acard:isBlack() and (mark==1)) then
			card = acard
			break
		end
	end
	
    if not card then return nil end
    local suit = card:getSuitString()
	local number = card:getNumberString()
    local card_id = card:getEffectiveId()
    local card_str = ("duel:shuangxiong[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	
    assert(skillcard)
    return skillcard
end


--xunyu
local quhu_skill={}
quhu_skill.name="quhu"
table.insert(sgs.ai_skills,quhu_skill)
quhu_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("QuhuCard") or self.player:isKongcheng() then return end
	local max_card = self:getMaxCard()
	local max_point = max_card:getNumber()
	return sgs.Card_Parse("@QuhuCard=" .. max_card:getEffectiveId())
end

sgs.ai_skill_playerchosen.quhu = function(self, targets)
	local target
	for _, player in sgs.qlist(targets) do
		if self:isEnemy(player) then
			if not target or target:getHp() >= player:getHp() then
				target = player
			end
		end
	end
	return target
end

sgs.ai_skill_use_func["QuhuCard"]=function(card,use,self)
	self:sort(self.enemies, "handcard")
	local max_card = self:getMaxCard()
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() > self.player:getHp() then
			local enemy_max_card = self:getMaxCard(enemy)
			if enemy_max_card and max_card:getNumber() > enemy_max_card:getNumber() then
				for _, enemy2 in ipairs(self.enemies) do
					if (enemy:objectName() ~= enemy2:objectName()) and enemy:inMyAttackRange(enemy2) then
						if use.to then
							use.to:append(enemy)
						end
						use.card=card
						return
					end
				end
			end
		end
	end
	self:sort(self.friends_noself, "handcard")
    for i=#self.friends_noself,1,-1 do 
    	local friend = self.friends_noself[i]
		if SmartAI.GetValue(friend) > 5  or (self:hasSkills(sgs.need_kongcheng,friend) and friend:getHandcardNum() == 1) then
		    local friend_min_card = self:getMinCard(friend)
			if friend_min_card and (not friend_min_card:inherits("Peach")) and max_card:getNumber() > friend_min_card:getNumber() then
				for _, enemy in ipairs(self.enemies) do
					if friend:inMyAttackRange(enemy) and enemy:getHp() <= 2 then
						if use.to then
							use.to:append(friend)
						end
						use.card=card
						return
					end
				end
			end
		end
    end
end


sgs.ai_skill_use["@@jieming"] = function(self, prompt)
	self:sort(self.friends)

	local max_x = 0
	local target
	for _, friend in ipairs(self.friends) do
		local x = math.min(friend:getMaxHP(), 5) - friend:getHandcardNum()		

		if x > max_x then
			max_x = x
			target = friend
		end
	end

	if target then
		return "@JiemingCard=.->" .. target:objectName()
	else
		return "."
	end
end


--taishici
local tianyi_skill={}
tianyi_skill.name="tianyi"
table.insert(sgs.ai_skills,tianyi_skill)
tianyi_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("TianyiCard") or self.player:isKongcheng() then return end
	return sgs.Card_Parse("@TianyiCard=.")
end

sgs.ai_skill_use_func["TianyiCard"]=function(card,use,self)
	local target,scard_str,slashIsEffect
	local max_card = self:getMaxCard()
	if not max_card then return end
	local slashNum=self:getSlashNumber()
	if self:canViewAs(max_card,"Slash") then slashNum = slashNum - 1 end
	
    local max_card = self:getMaxCard()
	local max_point = max_card:getNumber()
	
	if slashNum >= 1 then
	    local slash = self:getCard("Slash")
	    local dummy_use={isDummy=true}
	    self:useBasicCard(slash,dummy_use,true)
	    slashIsEffect = dummy_use.card
	end
	self:sort(self.enemies, "handcard")

	--self:log("tianyi used"..max_point)
	if slashIsEffect then
		for _, enemy in ipairs(self.enemies) do
		    local enemy_max_card = self:getMaxCard(enemy)
			if not (enemy:hasSkill("kongcheng") and enemy:getHandcardNum() == 1) and (enemy_max_card and max_point > enemy_max_card:getNumber()) then
			    target = enemy
	            scard_str = "@TianyiCard=" .. max_card:getId()
	            break
			end
		end
		if slashNum >= 2 and not scard_str then
			self:sort(self.friends_noself, "handcard")
            for i=#self.friends_noself,1,-1 do 
            	local friend = self.friends_noself[i]
				if SmartAI.GetValue(friend) > 5  or (self:hasSkills(sgs.need_kongcheng,friend) and friend:getHandcardNum() == 1) then
				    local friend_min_card = self:getMinCard(friend)
					if friend_min_card and max_point > friend_min_card:getNumber() then
					    target = friend
			            scard_str = "@TianyiCard=" .. max_card:getId()
			            break
					end
				end
            end
		end
	else
		local cards = self.player:getHandcards()
		cards=sgs.QList2Table(cards)
		self:sortByCardNeed(cards)
		if self:getOverflow() > 0 or self:cardNeed(cards[1]) <= 5  then
			for _, enemy in ipairs(self.enemies) do
				if  not enemy:isKongcheng() and (not self:hasSkills(sgs.need_kongcheng,enemy) and enemy:getHandcardNum() == 1) then
				    target = enemy
		            scard_str = "@TianyiCard=" .. cards[1]:getId()
		            break
				end
			end
		end
	end
	if target and scard_str then
		if use.to then
			use.to:append(target)
		end
		use.card = sgs.Card_Parse(scard_str)
	end
end



-- pangde
sgs.ai_skill_invoke.mengjin = function(self, data)
	local effect = data:toSlashEffect()
	return not self:isFriend(effect.to) or (self:hasSkills(sgs.lose_equip_skill,effect.to) and not effect.to:getEquips():isEmpty())
end


--dianwei
local qiangxi_skill={}
qiangxi_skill.name="qiangxi"
table.insert(sgs.ai_skills,qiangxi_skill)
qiangxi_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("QiangxiCard") then return end
	return sgs.Card_Parse("@QiangxiCard=.")
end

sgs.ai_skill_use_func["QiangxiCard"]=function(card,use,self)
	local target,scard_str
	local weapon = self.player:getWeapon()
	if weapon then
		local hand_weapon, cards
		cards = self.player:getHandcards()
		for _, card in sgs.qlist(cards) do
			if card:inherits("Weapon") then
				hand_weapon = card
				break
			end
		end
		self:sort(self.enemies)
		for _, enemy in ipairs(self.enemies) do
			if hand_weapon and self.player:inMyAttackRange(enemy) then
				target = enemy
				scard_str = "@QiangxiCard=" .. hand_weapon:getId()
			elseif self.player:distanceTo(enemy) <= 1 then
				target = enemy
				scard_str = "@QiangxiCard=" .. weapon:getId()
			end
		end
	else
		self:sort(self.enemies, "hp")
		for _, enemy in ipairs(self.enemies) do
			if self.player:inMyAttackRange(enemy) and self.player:getHp() > enemy:getHp() and self.player:getHp() > 2 then
				target = enemy
				scard_str = "@QiangxiCard=."
			end
		end
	end

	if target and scard_str then
		if use.to then
			use.to:append(target)
		end
		use.card = sgs.Card_Parse(scard_str)
	end
end

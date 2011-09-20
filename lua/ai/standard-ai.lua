
-- jianxiong
sgs.ai_skill_invoke.jianxiong = function(self, data)
        return not sgs.Shit_HasShit(data:toCard())
end

sgs.ai_skill_invoke.jijiang = function(self, data)
	if self:getSlashNumber(self.player,true)<=0 and #self.friends_noself > 0 then 
		self.jijiang_used = true 
		return true
	end
	return false
end

sgs.ai_skill_choice.jijiang = function(self , choices)
	if self.player:hasSkill("yongsi") or self.player:hasSkill("jijiang") then
		if self:getSlashNumber(self.player) <= 0 then return "ignore" end
	end
    if self:isFriend(self.room:getLord()) then return "accept" end
    return "ignore"
end

sgs.ai_skill_choice.hujia = function(self , choices)
	if self.player:hasSkill("yongsi") or self.player:hasSkill("hujia") then
		if self:getJinkNumber(self.player) <= 0 then return "ignore" end
	end
    if self:isFriend(self.room:getLord()) then return "accept" end
    return "ignore"
end

-- hujia
sgs.ai_skill_invoke.hujia = function(self, data)
	if self:getJinkNumber(self.player,true)<=0 and not self.hujia_used and #self.friends_noself > 0 then 
		--self.hujia_used = true 
		return true
	end
	return false
end

-- tuxi
sgs.ai_skill_use["@@tuxi"] = function(self, prompt)
	self:sort(self.enemies, "expect")
	
	local first_index, second_index
	for i=1, #self.enemies do																			
		if self:hasSkills(sgs.need_kongcheng,self.enemies[i]) and self.enemies[i]:getHandcardNum() == 1  then 
			local bullshit
		elseif not self.enemies[i]:isKongcheng() then
			if not first_index then 
				first_index = i 
			else 
				second_index = i 
			end
		end
		if second_index then break end
	end
	
	if first_index and not second_index then
		local others = self.room:getOtherPlayers(self.player)
		for _, other in sgs.qlist(others) do
			if (not self:isFriend(other) or (self:hasSkills(sgs.need_kongcheng,other) and other:getHandcardNum() == 1)) and (self.enemies[first_index]:objectName()) ~= (other:objectName()) and not other:isKongcheng() then 
				return ("@TuxiCard=.->%s+%s"):format(self.enemies[first_index]:objectName(), other:objectName())
			end
		end
	end
	
	if not second_index then return "." end
	
	--self:log(self.enemies[first_index]:getGeneralName() .. "+" .. self.enemies[second_index]:getGeneralName())
	local first = self.enemies[first_index]:objectName()
	local second = self.enemies[second_index]:objectName()
	return ("@TuxiCard=.->%s+%s"):format(first, second)
end

-- yiji (frequent)

-- tiandu, same as jianxiong
sgs.ai_skill_invoke.tiandu = sgs.ai_skill_invoke.jianxiong

-- ganglie
sgs.ai_skill_invoke.ganglie = function(self, data)
	local target = data:toPlayer()
	if self:hasSkills(sgs.need_kongcheng,target) and target:getHandcardNum() == 2 or target:hasSkill("lianying") and target:getHandcardNum() >= 2 then
		return self:isFriend(target)
	end
    return not self:isFriend(target)
end

-- fankui 
sgs.ai_skill_invoke.fankui = function(self, data) 
	local target = data:toPlayer()
	if self:hasSkills(sgs.lose_equip_skill,target) then
		if self:isFriend(target) then
			return not target:getEquips():isEmpty()
		else
			return not target:isKongcheng()
		end
	end
	if self:hasSkills(sgs.need_kongcheng,target) and target:getHandcardNum() == 1 then
		if self:isFriend(target) then
			return true
		else
			return not target:getEquips():isEmpty()
		end
	end
	if self:isFriend(target) and self.player:getHandcardNum() <= 2 and target:getHandcardNum() - target:getHp() > 1 then
		return true 
	end
	
	return not self:isFriend(target)
end


-- tieji
sgs.ai_skill_invoke.tieji = function(self, data) 
	local effect = data:toSlashEffect()
	local flag = false
	local zhangjiao = self:findHasSkill(sgs.QList2Table(self.room:getAlivePlayers()),"guidao")
	if zhangjiao and self:hasSuit("spade|club",true,zhangjiao) then return self:isFriend(zhangjiao) end
	return not self:isFriend(effect.to) 
end


sgs.ai_skill_use["@@liuli"] = function(self, prompt)
	local others=self.room:getOtherPlayers(self.player)												
	others=sgs.QList2Table(others)
	local source
	for _, player in ipairs(others) do 
		if player:hasFlag("slash_source") then
			source = player
			break
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy,true) and not (source:objectName() == enemy:objectName()) then	
            local cards = self.player:getCards("he")
            cards=sgs.QList2Table(cards)
            for _,card in ipairs(cards) do
                if (self.player:getWeapon() and card:getId() == self.player:getWeapon():getId()) and self.player:distanceTo(enemy)>1 then local bullshit
                elseif card:inherits("OffensiveHorse") and self.player:getAttackRange()==self.player:distanceTo(enemy)
                    and self.player:distanceTo(enemy)>1 then
                    local bullshit
                else
                    return "@LiuliCard="..card:getEffectiveId().."->"..enemy:objectName()
                end
            end
		end
	end
	for _, friend in ipairs(self.friends_noself) do
		if self.player:canSlash(friend,true) and not (source:objectName() == friend:objectName()) 
			and ((self:isWeak() and self:getJinkNumber() == 0 and (getDefense(friend) - getDefense(self.player) > 4 or self:getJinkNumber(friend) > 0) ) 
				or (friend:hasSkill("tiandu") and self:isEquip("EightDiagram",friend)) or (friend:hasSkill("leiji") and self:getJinkNumber(friend) > 0)) then	
            local cards = self.player:getCards("he")
            cards=sgs.QList2Table(cards)
            for _,card in ipairs(cards) do
                if (self.player:getWeapon() and card:getId() == self.player:getWeapon():getId()) and self.player:distanceTo(friend)>1 then local bullshit
                elseif card:inherits("OffensiveHorse") and self.player:getAttackRange()==self.player:distanceTo(friend)
                    and self.player:distanceTo(friend)>1 then
                    local bullshit
                else
                    return "@LiuliCard="..card:getEffectiveId().."->"..friend:objectName()
                end
            end
		end
	end	
	return "."
end

sgs.ai_skill_invoke["luoyi"]=function(self,data)
	if self.player:isSkipped(sgs.Player_Play) then return false end
    local cards=self.player:getHandcards()
    cards=sgs.QList2Table(cards)
    for _,card in ipairs(cards) do
        if self:canViewAs(card,"Slash") or self:canViewAs(self.room:peek(),"Slash") then
            for _,enemy in ipairs(self.enemies) do
                if self.player:canSlash(enemy,true) and self:slashIsEffective(card, enemy) and (not enemy:getArmor() or self:isEquip("RenwangShield",enemy) or self:isEquip("Vine",enemy)) and 
                	self:canHit(enemy,player) then -- AI Cheat							
                	--self:log("luoyi------------------target:"..enemy:getGeneralName())
					self:speak("luoyi")
                    return true
                    
                end
            end
        elseif card:inherits("Duel") then
	        local dummy_use={}
	        dummy_use.isDummy=true        
        	self:useCardDuel(card, dummy_use)
        	if dummy_use.card then return true end
        end
    end
    return false
end


sgs.ai_skill_invoke["@guicai"]=function(self,prompt)
    local judge = self.player:getTag("Judge"):toJudge()
    local who = judge.who
	if self:needRetrial(judge) then
		if judge.reason == "luoshen" and self:isFriend(judge.who) then
			if not (self:hasCard(who,"Crossbow","he") or self.player:getHandcardNum() > 2) or self.player:getRole() == "renegade" then
				return "."
			end
		elseif judge.reason == "eight_diagram" then
			if(self:getJinkNumber(who) > 0 and self.player:getHandcardNum() <= self.player:getHp()) then
				return "."
			end
		elseif judge.reason == "tieji" then
			if(self.player:getHandcardNum() <= self.player:getHp()) then
				return "."
			end	
		end
		local cards = sgs.QList2Table(self.player:getHandcards())		
		local card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return "@GuicaiCard=" .. card_id
		end
	end
	return "."
end

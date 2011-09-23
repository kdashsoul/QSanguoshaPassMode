local yaoshu_skill={}
yaoshu_skill.name="yaoshu"
table.insert(sgs.ai_skills,yaoshu_skill)
yaoshu_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("YaoshuCard") or self.player:isKongcheng() or #self.enemies == 0 then return end
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)

	self:sortByCardNeed(cards)
	
	for _,card in ipairs(cards)  do
		if card:getSuit() == sgs.Card_Spade then
			return sgs.Card_Parse("@YaoshuCard="..card:getEffectiveId())
		end
	end

end

sgs.ai_skill_use_func["YaoshuCard"]=function(card,use,self)
	if #self.enemies == 0 then return end 
	self:sort(self.enemies, "hp")
	use.card = card
	if use.to then use.to:append(self.enemies[1]) end
	return
end

local fanjian_pass_skill={}
fanjian_pass_skill.name="fanjian_pass"
table.insert(sgs.ai_skills,fanjian_pass_skill)
fanjian_pass_skill.getTurnUseCard=function(self)
    if self.player:isKongcheng() or self.player:hasUsed("FanjianPassCard") or #self.enemies == 0 then return end
	
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	
	self:sortByCardNeed(cards)
	
	for _, card in ipairs(cards) do
		if not (card:inherits("Peach") or card:inherits("Analeptic")) then
			return sgs.Card_Parse("@FanjianPassCard="..card:getEffectiveId())
		end
	end
end

sgs.ai_skill_use_func["FanjianPassCard"]=function(card,use,self)
	if #self.enemies == 0 then return end 
	self:sort(self.enemies, "hp")
	use.card = card
	if use.to then use.to:append(self.enemies[1]) end
	return
end


local jieyin_pass_skill={}
jieyin_pass_skill.name="jieyin_pass"
table.insert(sgs.ai_skills,jieyin_pass_skill)
jieyin_pass_skill.getTurnUseCard=function(self)
	if self.player:isKongcheng() then return end
	if self.player:hasUsed("JieyinPassCard") then return end
	
	local card
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	self:sortByCardNeed(cards)
	card = cards[1]
	
	if card then
		return sgs.Card_Parse("@JieyinPassCard="..card:getEffectiveId())
	end
end

sgs.ai_skill_use_func["JieyinPassCard"]=function(card,use,self)
	self:sort(self.friends, "hp")
	for _, friend in ipairs(self.friends) do
		if friend:getGeneral():isMale() and friend:isWounded() then
			use.card=card
			if use.to then use.to:append(friend) end
			return
		end
	end
end


sgs.ai_skill_invoke.jueduan = function(self, data)
	local target = data:toPlayer()
	return not self:isFriend(target)
end

sgs.ai_skill_invoke.longhou  = function(self, data)
	local target = data:toPlayer()
	return not self:isFriend(target) and not self.player:isKongcheng()
end

sgs.ai_skill_invoke.jielue = sgs.ai_skill_invoke.jueduan
sgs.ai_skill_invoke.wushen_pass = sgs.ai_skill_invoke.jueduan

sgs.ai_skill_invoke.longwei = function(self, data)
	return self.enemies and #self.enemies > 0
end

local kurou_pass_skill={}
kurou_pass_skill.name="kurou_pass"
table.insert(sgs.ai_skills,kurou_pass_skill)
kurou_pass_skill.getTurnUseCard=function(self,inclusive)
    if  self.player:getHp() > 3 and self.player:getHandcardNum() <= self.player:getHp() then
		return sgs.Card_Parse("@KurouPassCard=.")
    end
		
	if self:isEquip("Crossbow") then
        for _, enemy in ipairs(self.enemies) do
            if self.player:canSlash(enemy,true) and (self.player:getHp()>1 or self:getAnalepticNum() > 0) then
                return sgs.Card_Parse("@KurouPassCard=.")
            end
        end
    end
end

sgs.ai_skill_use_func["KurouPassCard"]=function(card,use,self)
	if not use.isDummy then self:speak("kurou") end
	use.card=card
end

local zhaxiang_skill={}
zhaxiang_skill.name="zhaxiang"
table.insert(sgs.ai_skills,zhaxiang_skill)
zhaxiang_skill.getTurnUseCard=function(self)
    if self.player:getHandcardNum() < 4 or (not self.player:isWounded()) or self.player:hasUsed("ZhaxiangCard") then return end
	local unpreferedCards={}
	
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	
	self:sortByCardNeed(cards)
	
	for _, card in ipairs(cards) do
		if not (card:inherits("Peach") or card:inherits("Analeptic")) then
			table.insert(unpreferedCards,card:getId())
		end
		if #unpreferedCards == 4 then break end
	end
	
	if #unpreferedCards == 4 then
		return sgs.Card_Parse("@ZhaxiangCard="..table.concat(unpreferedCards,"+"))
	end
end

sgs.ai_skill_use_func["ZhaxiangCard"]=function(card,use,self)
	use.card = card
end


local zhiheng_pass_skill={}
zhiheng_pass_skill.name="zhiheng_pass"
table.insert(sgs.ai_skills,zhiheng_pass_skill)
zhiheng_pass_skill.getTurnUseCard=function(self)
	local unpreferedCards={}
	local cards=sgs.QList2Table(self.player:getHandcards())
	
	if not self.player:hasUsed("ZhihengPassCard") then 
		if self.player:getHp() < 3 then
			local zcards = self.player:getCards("h")
			for _, zcard in sgs.qlist(zcards) do
				if not zcard:inherits("Peach") and not zcard:inherits("ExNihilo") then
					table.insert(unpreferedCards,zcard:getId())
				end
			end
			zcards = self.player:getEquips()
			for _, zcard in sgs.qlist(zcards) do
				if not zcard:inherits("Armor") or zcard:inherits("SilverLion") then
					table.insert(unpreferedCards,zcard:getId())
				end
			end
		else 
			local j_num=self:getJinkNumber()-1
			local p_num=self:getPeachNum()-2
			if self:isEquip("EightDiagram") or self:isEquip("RenwangShield") then j_num=j_num+1 end
			for _,card in ipairs(cards) do
				if card:inherits("Jink") and j_num>0 then 
					table.insert(unpreferedCards,card:getId())
					j_num=j_num-1
				elseif card:inherits("Peach") and p_num>0 then 
					table.insert(unpreferedCards,card:getId())
					p_num=p_num-1
				end
			end
			
	        for _,card in ipairs(cards) do
	        	if card:inherits("Slash") and not self:isEquip("Crossbow") then 
	        		table.insert(unpreferedCards,card:getId()) 
	            elseif card:inherits("EquipCard") then
	                if card:inherits("Weapon") or card:inherits("OffensiveHorse")  or (card:inherits("DefensiveHorse") and self.player:getDefensiveHorse()) then
	                    table.insert(unpreferedCards,card:getId())
	                elseif (card:inherits("Armor") and self.player:getArmor()) then
	                	if not card:inherits("SilverLion") then
	                		table.insert(unpreferedCards,card:getId())
	                	end
	                end
	            elseif card:inherits("TrickCard") and not card:inherits("Nullification") then
	                 table.insert(unpreferedCards,card:getId())
	            end
	        end

			if self.player:getWeapon() then														
				table.insert(unpreferedCards, self.player:getWeapon():getId())
			end
				
			if self:isEquip("SilverLion") and self.player:isWounded() then
				table.insert(unpreferedCards, self.player:getArmor():getId())
			end	
				
			local equips=self.player:getEquips()
			for _,equip in sgs.qlist(equips) do
				if equip:inherits("OffensiveHorse") and self.player:getWeapon() then
					table.insert(unpreferedCards, equip:getId())
					break
				end
			end			
		end
	end	
	
	if #unpreferedCards>0 then 
		return sgs.Card_Parse("@ZhihengPassCard="..table.concat(unpreferedCards,"+")) 
	end
end

sgs.ai_skill_use_func["ZhihengPassCard"]=function(card,use,self)
	use.card = card
end

-- tuxi
sgs.ai_skill_use["@@tuxi_pass"] = function(self, prompt)
	if not self.enemies or #self.enemies == 0 or self.enemies[1]:isNude() then return "." end 
	self:sort(self.enemies, "hp")
	return ("@TuxiPassCard=.->%s"):format(self.enemies[1]:objectName())
end


local luoyi_pass_skill={}
luoyi_pass_skill.name="luoyi_pass"
table.insert(sgs.ai_skills,luoyi_pass_skill)
luoyi_pass_skill.getTurnUseCard=function(self)
    if self.player:isKongcheng() or self.player:hasUsed("LuoyiPassCard") or #self.enemies == 0 then return end
	
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	
	self:sortByCardNeed(cards)

	for _, card in ipairs(cards) do
		if card:inherits("EquipCard") then
			return sgs.Card_Parse("@LuoyiPassCard="..card:getEffectiveId())
		end
	end
		
	local usecards ={}
	for _, card in ipairs(cards) do
		if not (card:inherits("Peach") or card:inherits("Analeptic") or (self:getSlashNumber() == 1 and card:inherits("Slash"))) then
			table.insert(usecards, card:getEffectiveId())
			if #usecards == 2 then
				local card_str = ("@LuoyiPassCard=%d+%d"):format(usecards[1],usecards[2])
				return sgs.Card_Parse(card_str)
			end
		end
	end
end

sgs.ai_use_priority.LuoyiPassCard = 3 

sgs.ai_skill_use_func["LuoyiPassCard"]=function(scard,use,self)
    local cards=self.player:getHandcards()
    cards=sgs.QList2Table(cards)
    for _,card in ipairs(cards) do
        if self:canViewAs(card,"Slash") then
            for _,enemy in ipairs(self.enemies) do
                if self.player:canSlash(enemy,true) and self:slashIsEffective(card, enemy) and (not enemy:getArmor() or self:isEquip("RenwangShield",enemy) or self:isEquip("Vine",enemy)) and 
                	self:canHit(enemy,player) then
                    use.card = scard
                end
            end
        elseif card:inherits("Duel") then
	        local dummy_use={}
	        dummy_use.isDummy=true        
        	self:useCardDuel(card, dummy_use)
        	if dummy_use.card then use.card = scard end
        end
    end
end
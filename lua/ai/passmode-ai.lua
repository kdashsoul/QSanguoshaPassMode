function SmartAI:slashValid(enemy, slash, player)
	player = player or self.player
	slash = slash or sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	return player:canSlash(enemy) and self:slashIsEffective(slash, enemy) and not self:slashProhibit(slash, enemy)
end

function SmartAI:getAnalepticNum(player)
	player = player or self.player
	local n = 0
	local cards = player:getCards("h")
	for _, card in sgs.qlist(cards) do
		if self:canViewAs(card, "Analeptic", player) then		
			n = n + 1
		end
	end
	return n
end

function SmartAI:getSlashNumber(player, only_self)
	player = player or self.player
	local n = 0
	local cards = player:getCards("h")
	for _, card in sgs.qlist(cards) do
		if self:canViewAs(card, "Slash", player) then		
			n = n + 1
		end
	end
	if player:hasWeapon("spear") then
		n = n + math.floor((cards:length() - n)/2)
	end	
	cards = player:getCards("e")
	for _, card in sgs.qlist(cards) do
		if self:getSkillViewCard(card, "Slash", true, player) then		
			n = n + 1
		end
	end
	
	if player:hasSkill("wushuang") then
		n = n * 2
	end
	
	if only_self then return n end

	if (player:isLord() or (player:hasSkill("weidi") and self.room:getLord():hasSkill("jijiang"))) and player:hasSkill("jijiang") then
		local lieges = self.room:getLieges("shu", player)
		for _, liege in sgs.qlist(lieges) do
			if self:isFriend(liege, player) then
				n = n + self:getSlashNumber(liege)
			end
		end
	end
	
	return n
end

function SmartAI:isCompulsoryView(card,class_name,player)
	player = player or self.player
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if class_name == "Slash" then
	    if self.player:hasSkill("wushen") and card:getSuit() == sgs.Card_Heart then return ("slash:wushen[%s:%s]=%d"):format(suit, number, card_id) end
		if self.player:hasSkill("jiejiu") and card:inherits("Analeptic") then return ("slash:jiejiu[%s:%s]=%d"):format(suit, number, card_id) end
	end	
end

function SmartAI:getSkillViewCard(card,class_name,only_equip,player)
	player = player or self.player
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if class_name == "Slash" then
		if player:hasSkill("wusheng") then
			if card:isRed() and not card:inherits("Peach") then
				return ("slash:wusheng[%s:%s]=%d"):format(suit, number, card_id)
			end
		end	
		if not only_equip then
			if player:hasSkill("longdan") and card:inherits("Jink") then
				return ("slash:longdan[%s:%s]=%d"):format(suit, number, card_id)
			elseif player:hasSkill("zhanshen_p") and (card:inherits("TrickCard") or card:inherits("EquipCard"))then
				return ("slash:zhanshen_p[%s:%s]=%d"):format(suit, number, card_id)
			end
		end
	elseif class_name == "Jink" then
		if not only_equip then
			if player:hasSkill("longdan") and card:inherits("Slash") then
				return ("jink:longdan[%s:%s]=%d"):format(suit, number, card_id)
			elseif player:hasSkill("qingguo") and card:isBlack() then
				return ("jink:qingguo[%s:%s]=%d"):format(suit, number, card_id)
			end
		end
	elseif class_name == "Peach" then
		if player:hasSkill("jijiu") and card:isRed() then 
			return ("peach:jijiu[%s:%s]=%d"):format(suit, number, card_id)
		end
	elseif class_name == "Analeptic" then
		if not only_equip then
			if player:hasSkill("jiuchi") and card:getSuit() == sgs.Card_Spade then 
				return ("analeptic:jiuchi[%s:%s]=%d"):format(suit, number, card_id)
			end
		end	
	end
end

function SmartAI:canViewAs(card,class_name,player)
	player = player or self.player
	local r1 = card:inherits(class_name) or self:getSkillViewCard(card,class_name,false,player)
	local r2 = self:isCompulsoryView(card,"Slash",player)
	if class_name == "Slash" then
		return r1 or r2
	else
		return r1 and not r2
	end 
end

--------------------------------BASE--FUNCTION--OVER--------------------------------------------------------------------

local dianji_p_skill={
	name = "dianji_p" ,
	getTurnUseCard = function(self)
		if self.player:hasUsed("DianjiPassCard") or self.player:isKongcheng() or #self.enemies == 0 then return end
	    local cards = self.player:getCards("h")	
	    cards=sgs.QList2Table(cards)

		self:sortByCardNeed(cards)
		
		for _,card in ipairs(cards)  do
			if card:getSuit() == sgs.Card_Spade and not card:inherits("Shit") then
				return sgs.Card_Parse("@DianjiPassCard="..card:getEffectiveId())
			end
		end
	end
}
table.insert(sgs.ai_skills,dianji_skill)

sgs.ai_skill_use_func["DianjiPassCard"]=function(card,use,self)
	if #self.enemies == 0 then return end 
	self:sort(self.enemies, "hp")
	use.card = card
	if use.to then use.to:append(self.enemies[1]) end
	return
end


local xunma_p_skill={
	name = "xunma_p" ,
	getTurnUseCard = function(self)
		if self.player:isKongcheng() then return end
	    local cards = self.player:getCards("h")	
	    cards=sgs.QList2Table(cards)

		for _,card in ipairs(cards)  do
			if card:inherits("Horse") then
				return sgs.Card_Parse("@XunmaPassCard="..card:getEffectiveId())
			end
		end
	end
}
table.insert(sgs.ai_skills,xunma_p_skill)

sgs.ai_skill_use_func["XunmaPassCard"]=function(card,use,self)
	use.card = card
end

sgs.ai_use_priority.XunmaPassCard = 8.7 

local rende_p_skill={
	name = "rende_p" ,
	getTurnUseCard = function(self)
		if self.player:usedTimes("RendePassCard") < 1 or self:getOverflow() > 0 or self:getCard("Shit") then 
			local card_id = self:getCardRandomly(self.player, "h")
			return sgs.Card_Parse("@RendePassCard=" .. card_id)
		end
	end
}
table.insert(sgs.ai_skills,rende_p_skill)

sgs.ai_skill_use_func["RendePassCard"]=function(rende_card,use,self)
    if self.player:usedTimes("RendePassCard") < 1 then
		local cards = self.player:getHandcards()
		for _, friend in ipairs(self.friends_noself) do
			if friend:getHp() == 1 then
				for _, hcard in sgs.qlist(cards) do
					if hcard:inherits("Analeptic") or hcard:inherits("Peach") then 
						use.card = sgs.Card_Parse("@RendePassCard=" .. hcard:getId())
						if use.to then use.to:append(friend) end
						return
					end
				end
			end
			if friend:hasSkill("paoxiao") then
				for _, hcard in sgs.qlist(cards) do
					if hcard:inherits("Slash") then 
						use.card = sgs.Card_Parse("@RendePassCard=" .. hcard:getId())
						if use.to then use.to:append(friend) end
						return
					end
				end
			elseif friend:hasSkill("qingnang") and friend:getHp() < 2 and friend:getHandcardNum() < 1 then
				for _, hcard in sgs.qlist(cards) do
					if hcard:isRed() and not (hcard:inherits("ExNihilo") or hcard:inherits("Peach")) then 
						use.card = sgs.Card_Parse("@RendePassCard=" .. hcard:getId())
						if use.to then use.to:append(friend) end
						return
					end
				end
			elseif friend:hasSkill("jizhi") then
				for _, hcard in sgs.qlist(cards) do
					if hcard:getTypeId() == sgs.Card_Trick then 
						use.card = sgs.Card_Parse("@RendePassCard=" .. hcard:getId())
						if use.to then use.to:append(friend) end
						return
					end
				end
			elseif friend:hasSkill("guose") then
				for _, hcard in sgs.qlist(cards) do
					if hcard:getSuit() == sgs.Card_Diamond then 
						use.card = sgs.Card_Parse("@RendePassCard=" .. hcard:getId())
						if use.to then use.to:append(friend) end
						return
					end
				end
			elseif friend:hasSkill("leiji") then
				for _, hcard in sgs.qlist(cards) do
					if hcard:getSuit() == sgs.Card_Spade then 
						use.card = sgs.Card_Parse("@RendePassCard=" .. hcard:getId())
						if use.to then use.to:append(friend) end
						return
					end
				end
			elseif friend:hasSkill("xiaoji") then
				for _, hcard in sgs.qlist(cards) do
					if hcard:inherits("EquipCard") then 
						use.card = sgs.Card_Parse("@RendePassCard=" .. hcard:getId())
						if use.to then use.to:append(friend) end
						return
					end
				end
			end
			
		end
	end
	
	local shit
	shit = self:getCard("Shit")
	if shit then
		use.card = sgs.Card_Parse("@RendePassCard=" .. shit:getId())
		self:sort(self.enemies,"hp")
		if use.to then use.to:append(self.enemies[1]) end
		return
	end
	
	if self:getOverflow()>0 or (self.player:isWounded() and self.player:usedTimes("RendePassCard") < 2 and not self.player:isKongcheng()) then 
		if #self.friends_noself == 0 then return end
		
		self:sort(self.friends_noself, "handcard")
		local friend = self.friends_noself[1]
		local card_id = self:getCardRandomly(self.player, "h")
		if not sgs.Sanguosha:getCard(card_id):inherits("Shit") then
			use.card = sgs.Card_Parse("@RendePassCard=" .. card_id)
		end
		if use.to then use.to:append(friend) end
		return
    end
end

local jijiang_p_skill={
	name = "jijiang_p",
	getTurnUseCard = function(self)
		if not (self.player:canSlashWithoutCrossbow() or self.player:hasWeapon("crossbow")) or self.player:hasUsed("JijiangPassCard") then return end
		return sgs.Card_Parse("@JijiangPassCard=.")
	end
}
table.insert(sgs.ai_skills,jijiang_p_skill)
sgs.ai_use_priority.JijiangPassCard = 7

sgs.ai_skill_use_func["JijiangPassCard"] = function(card,use,self)
	if self.player:getHp() == 1 and self.player:getMark("@renyi_p") == 0 then return end
	self:sort(self.enemies, "hp")
	for _,enemy in ipairs(self.enemies) do
		local def=getDefense(enemy)
		local amr=enemy:getArmor()
	    if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
	    elseif self:slashIsEffective(nil, enemy) and not self:slashProhibit(nil, enemy) then
			if use.to then 
				use.to:append(enemy) 
			end
			use.card = card
			return
		end
	end
end

sgs.ai_skill_choice.jijiang_p = function(self , choices)
	if self.player:getHp() > 2 then 
		return "losehp"
	else
		return "losemark"
	end
end


local badao_p_skill={
	name = "badao_p" ,
	getTurnUseCard = function(self)
		if self.player:isKongcheng() or self.player:getMark("@jianxiong_p") < 3 then return end
		local fcard
		local cards = self.player:getHandcards()
		local same_suit=false
		cards = sgs.QList2Table(cards)
		self:sortByCardNeed(cards)
		local red,black
		for _, fcard in ipairs(cards) do
			if not (fcard:inherits("Peach") or fcard:inherits("ExNihilo") or fcard:inherits("SavageAssault") or fcard:inherits("ArcheryAttack")) then
				if fcard:isRed() then 
					red = sgs.Card_Parse(("archery_attack:badao_p[%s:%s]=%d"):format(fcard:getSuitString(), fcard:getNumberString(), fcard:getId()))
				else
					black = sgs.Card_Parse(("savage_assault:badao_p[%s:%s]=%d"):format(fcard:getSuitString(), fcard:getNumberString(), fcard:getId()))
				end
				if red and black then break end
			end
		end

		local vaa = self:getAoeValue(sgs.Sanguosha:cloneCard("archery_attack", sgs.Card_NoSuit, 0))
		local vsa = self:getAoeValue(sgs.Sanguosha:cloneCard("savage_assault", sgs.Card_NoSuit, 0))
		
		if vaa < -10 then red = nil end
		if vsa < -10 then black = nil end
		
		if red and black then
			if vsa > vaa then return black else return red end
		elseif red then return red
		elseif black then return black
		end

	end
}
table.insert(sgs.ai_skills,badao_p_skill)

sgs.ai_skill_use_func["BadaoPassCard"]=function(card,use,self)
    use.card=card
end



local fanjian_p_skill={
	name = "fanjian_p",
	getTurnUseCard = function(self)
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
}
table.insert(sgs.ai_skills,fanjian_p_skill)

sgs.ai_skill_use_func["FanjianPassCard"]=function(card,use,self)
	if not self.enemies or #self.enemies == 0 then return end
	self:sort(self.enemies, "hp")
	use.card = card
	if use.to then use.to:append(self.enemies[1]) end
end


local jieyin_p_skill={
	name = "jieyin_p",
	getTurnUseCard = function(self)
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
}
table.insert(sgs.ai_skills,jieyin_p_skill)

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


local tuodao_p_skill={
	name = "tuodao_p",
	getTurnUseCard = function(self)
		if self.player:isNude() then return end
		if self.player:hasUsed("TuodaoPassCard") then return end
		
		local card
		local cards = self.player:getCards("he")
		cards=sgs.QList2Table(cards)
		self:sortByCardNeed(cards)
		
		for _, card in ipairs(cards) do
			if (card:inherits("Weapon") and card:isBlack()) or (card:inherits("Horse") and card:isRed())then
				return sgs.Card_Parse("@TuodaoPassCard="..card:getEffectiveId())
			end
		end
	end
}
table.insert(sgs.ai_skills,tuodao_p_skill)

sgs.ai_skill_use_func["TuodaoPassCard"]=function(card,use,self)
	self:sort(self.enemies, "hp")
	local n = self:getSlashNumber()
	for _, enemy in ipairs(self.enemies) do
		if n > self:getSlashNumber(enemy) then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end
end

sgs.ai_skill_invoke["wenjiu_p"] = function(self)
	player = self.player
	if not player:containsTrick("indulgence") and player:getMark("@wenjiu_p") > 0 then return false end
	return true
end


local duanhe_p_skill={
	name = "duanhe_p",
	getTurnUseCard = function(self)
		if self.player:isNude() or self.player:getHp() <= 1 then return end
		
		local card
		local cards = self.player:getCards("he")
		cards=sgs.QList2Table(cards)
		self:sortByCardNeed(cards)
		
		for _, card in ipairs(cards) do
			if card:inherits("Weapon") then
				return sgs.Card_Parse("@DuanhePassCard="..card:getEffectiveId())
			end
		end
	end
}
table.insert(sgs.ai_skills,duanhe_p_skill)

sgs.ai_skill_use_func["DuanhePassCard"]=function(card,use,self)
	local scard = sgs.Card_Parse(sgs.QList2Table(card:getSubcards())[1])
	local range = sgs.weapon_range[scard:className()] or 0
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() + enemy:getHandcardNum() >= range then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end
	
end


sgs.ai_skill_invoke["jueduan_p"] = function(self, data)
	local target = data:toPlayer()
	return not self:isFriend(target)
end

sgs.ai_skill_invoke["longhou_p"]  = function(self, data)
	local target = data:toPlayer()
	return not self:isFriend(target) and (self.player:getHandcardNum() > 2 or self.player:getHp() > 2)
end

sgs.ai_skill_choice["jianxiong_p"] = function(self,choices, data)
	local card = data:toCard()
	if card:inherits("Lightning") or (card:inherits("Slash") and not self.player:hasWeapon("crossbow")) then
		return "draw"
	end
	return "gain"
end

sgs.ai_skill_invoke["dujiang_p"]  = function(self, data)
	return (self.player:getEquips():length() > 1 and self.player:getEquips():length() < 4)
end

sgs.ai_skill_invoke["jielue_p"] = sgs.ai_skill_invoke["jueduan_p"]
sgs.ai_skill_invoke["wushen_p"] = sgs.ai_skill_invoke["jueduan_p"]
sgs.ai_skill_invoke["shipo_p"] = sgs.ai_skill_invoke["jueduan_p"]
sgs.ai_skill_invoke["tongji_p"] = function(self, data)
	local target = data:toPlayer()
	return not self:isFriend(target) and not self.player:isKongcheng()
end

sgs.ai_skill_invoke["jumou_p"] = true
sgs.ai_skill_invoke["zhaolie_p"] = true
sgs.ai_skill_invoke["qianggong_p"] = sgs.ai_skill_invoke.liegong
sgs.ai_skill_invoke["nuozhan_p"] = function(self, data)
	if self.enemies and #self.enemies > 0 and self:getSlashNumber() > 0 then
		for _,enemy in ipairs(self.enemies) do
			if self:slashValid(enemy, sgs.Card_Parse(card_id), player) then
				return true
			end
		end	
	end
	return false
end

sgs.ai_skill_invoke["longwei_p"] = function(self, data)
	player = player or self.player
	if self.enemies and #self.enemies > 0 then
		self:sort(self.enemies, "hp")
		local target = self.enemies[1]
		local can_longwei = target:getHp() > player:getHp() or target:getHandcardNum() > player:getHandcardNum() 
		if can_longwei and self.player:canSlash(target) and not self:slashProhibit(slash ,target)  then
			return true
		end
	end
	return false
end

sgs.ai_skill_playerchosen["longwei_p_slash"] = function(self,targets)
	local slash = sgs.Card_Parse(("slash[%s:%s]"):format(sgs.Card_NoSuit, 0))
	self:sort(targets, "hp")
	for _, target in ipairs(targets) do
		if self:isEnemy(target) and self.player:canSlash(target) and not self:slashProhibit(slash ,target) then
			return target
		end
	end
end

sgs.ai_skill_discard["tongji_p"] = function(self)
	local to_discard = {}
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)
	self:sortByCardNeed(cards)
	table.insert(to_discard, cards[1]:getEffectiveId())
	return to_discard
end


sgs.ai_skill_invoke["@quwu_p"] = function(self, data)
	player = self.player
	if player:getOffensiveHorse() then return player:getOffensiveHorse():getId() end
	if player:getWeapon() then return player:getWeapon():getId() end
	if player:getArmor() then return player:getArmor():getId() end
	if player:getDefensiveHorse() then return player:getDefensiveHorse():getId() end
end

local kurou_p_skill={
	name = "kurou_p",
	getTurnUseCard = function(self,inclusive)
	    if  self.player:getHp() > 2 and (self.player:getHandcardNum() < self.player:getHp() or self.player:getLostHp() == 0) then
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
}
table.insert(sgs.ai_skills,kurou_p_skill)

sgs.ai_skill_use_func["KurouPassCard"]=function(card,use,self)
	use.card=card
end

local zhaxiang_p_skill={
	name = "zhaxiang_p",
	getTurnUseCard = function(self)
	    if self.player:getCards("he"):length() < 4 or (not self.player:isWounded()) or self.player:hasUsed("ZhaxiangPassCard") then return end
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
		if #unpreferedCards < 4 then
			cards = self.player:getEquips()
			for _, zcard in sgs.qlist(cards) do
				if not zcard:inherits("Armor") or zcard:inherits("SilverLion") then
					table.insert(unpreferedCards,zcard:getId())
				end
				if #unpreferedCards == 4 then break end
			end
		end
		if #unpreferedCards == 4 then
			return sgs.Card_Parse("@ZhaxiangPassCard="..table.concat(unpreferedCards,"+"))
		end
	end
}
table.insert(sgs.ai_skills,zhaxiang_p_skill)

sgs.ai_skill_use_func["ZhaxiangPassCard"]=function(card,use,self)
	use.card = card
end


local zhiheng_p_skill={
	name = "zhiheng_p" ,
	getTurnUseCard = function(self)
		if not self.player:hasUsed("ZhihengPassCard") then 
			return sgs.Card_Parse("@ZhihengPassCard=.")
		end
	end
}
table.insert(sgs.ai_skills, zhiheng_p_skill)

sgs.ai_skill_use_func["ZhihengPassCard"] = function(card, use, self)
	local unpreferedCards={}
	local cards=sgs.QList2Table(self.player:getHandcards())
	
	if self.player:getHp() < 3 then
		local zcards = self.player:getCards("he")
		for _, zcard in sgs.qlist(zcards) do
			if not zcard:inherits("Peach") and not zcard:inherits("ExNihilo") then
				table.insert(unpreferedCards,zcard:getId())
			end	
		end
	end
	
	if #unpreferedCards == 0 then 
		if self:getCardsNum("Slash")>1 then 
			self:sortByKeepValue(cards)
			for _,card in ipairs(cards) do
				if card:inherits("Slash") then table.insert(unpreferedCards,card:getId()) end
			end
			table.remove(unpreferedCards,1)
		end
		
		local num=self:getCardsNum("Jink")-1							
		if self.player:getArmor() then num=num+1 end
		if num>0 then
			for _,card in ipairs(cards) do
				if card:inherits("Jink") and num>0 then 
					table.insert(unpreferedCards,card:getId())
					num=num-1
				end
			end
		end
        for _,card in ipairs(cards) do
            if card:inherits("EquipCard") then
                if card:inherits("Weapon") or
                (card:inherits("DefensiveHorse") and self.player:getDefensiveHorse()) or
                card:inherits("OffensiveHorse") or
                (card:inherits("Armor") and self.player:getArmor()) or
                 card:inherits("AmazingGrace") or
                 card:inherits("Lightning") then
                    table.insert(unpreferedCards,card:getId())
                end
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
	
	if #unpreferedCards>0 then 
		use.card = sgs.Card_Parse("@ZhihengPassCard="..table.concat(unpreferedCards,"+")) 
		return 
	end
end

-- tuxi
sgs.ai_skill_use["@@tuxi_p"] = function(self, prompt)
	self:sort(self.enemies, "handcard")

	local first_index, second_index
	for i=1, #self.enemies do
		if self:hasSkills(sgs.need_kongcheng, self.enemies[i]) and self.enemies[i]:getHandcardNum() == 1 then
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
			if (not self:isFriend(other) or (self:hasSkills(sgs.need_kongcheng, other) and other:getHandcardNum() == 1)) and
				self.enemies[first_index]:objectName() ~= other:objectName() and not other:isKongcheng() then
				return ("@TuxiPassCard=.->%s+%s"):format(self.enemies[first_index]:objectName(), other:objectName())
			end
		end
	end

	if not first_index then return "." end
	local first = self.enemies[first_index]:objectName()
	if not second_index then 
		return ("@TuxiPassCard=.->%s"):format(first)
	end
	local second = self.enemies[second_index]:objectName()
	return ("@TuxiPassCard=.->%s+%s"):format(first, second)
end


local luoyi_p_skill={
	name = "luoyi_p" ,
	getTurnUseCard = function(self)
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
}
table.insert(sgs.ai_skills,luoyi_p_skill)

sgs.ai_use_priority.LuoyiPassCard = 3 

sgs.ai_skill_use_func["LuoyiPassCard"]=function(scard,use,self)
	player = player or self.player
    local cards=self.player:getHandcards()
    cards=sgs.QList2Table(cards)
    for _,card in ipairs(cards) do
        if self:canViewAs(card,"Slash",self.player) then
            for _,enemy in ipairs(self.enemies) do
                if self:slashValid(enemy, sgs.Card_Parse(card:getEffectiveId()), player) and enemy:getHandcardNum()< 2 then
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

local lijian_p_skill={
	name = "lijian_p" ,
	getTurnUseCard = function(self)
		if self.player:hasUsed("LijianPassCard") then
			return 
		end
		if not self.player:isNude() then
			local card
			local card_id
			if self.player:getArmor() and self.player:isWounded() and self.player:getArmor():objectName()=="silver_lion" then
				card = sgs.Card_Parse("@LijianPassCard=" .. self.player:getArmor():getId())
			elseif self.player:getHandcardNum() > self.player:getHp() then
				local cards = self.player:getHandcards()
				cards=sgs.QList2Table(cards)
				
				for _, acard in ipairs(cards) do
					if not acard:inherits("Peach") and not acard:inherits("Shit") then 
						card_id = acard:getEffectiveId()
						break
					end
				end
			elseif not self.player:getEquips():isEmpty() then
				local player=self.player
				if player:getWeapon() then card_id=player:getWeapon():getId()
				elseif player:getOffensiveHorse() then card_id=player:getOffensiveHorse():getId()
				elseif player:getDefensiveHorse() then card_id=player:getDefensiveHorse():getId()
				elseif player:getArmor() and player:getHandcardNum()<=1 then card_id=player:getArmor():getId()
				end
			end
			if not card_id then
				cards=sgs.QList2Table(self.player:getHandcards())
				for _, acard in ipairs(cards) do
					if not acard:inherits("Peach") and not acard:inherits("Shit") then 
						card_id = acard:getEffectiveId()
						break
					end
				end
			end
			if not card_id then
				return nil
			else
				card = sgs.Card_Parse("@LijianPassCard=" .. card_id)
				return card
			end
		end
	end
}
table.insert(sgs.ai_skills,lijian_p_skill)

sgs.ai_skill_use_func["LijianPassCard"]=function(card,use,self)
	local findFriend_maxSlash=function(self,first)
		self:log("Looking for the friend!")
		local maxSlash = 0
		local friend_maxSlash
		for _, friend in ipairs(self.friends) do
			if self:getSlashNumber(friend)>= maxSlash then
				maxSlash=self:getSlashNumber(friend)
				friend_maxSlash = friend
			end
		end
		if friend_maxSlash then
			local safe = false
			if (first:hasSkill("ganglie") or first:hasSkill("enyuan")) then
				if (first:getHp()<=1 and self:getSlashNumber(first)==0) then safe=true end
			elseif friend_maxSlash:hasSkill("wushuang") and self:getSlashNumber(first) < 2 then 
				safe = true
			elseif (self:getSlashNumber(friend_maxSlash) >= self:getSlashNumber(first)) then safe=true end
			if safe then return friend_maxSlash end
		else self:log("unfound")
		end
		return nil
	end
	if not self.player:hasUsed("LijianPassCard") then 
		self:sort(self.enemies, "hp")
		local males = {}
		local first, second
		local zhugeliang_kongcheng,guojia_yiji
		local gift
		local firstSlashNum = 0
		for _, enemy in ipairs(self.enemies) do
			if #males==1 and males[1]:getHp()<=1 and #self.friends_noself > 0 then
				gift = true
				break 
			end
			if zhugeliang_kongcheng and #males==1 then table.insert(males, zhugeliang_kongcheng) end
			if not enemy:hasSkill("wuyan") then
				if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then	zhugeliang_kongcheng=enemy
				elseif enemy:hasSkill("yiji") and enemy:getHp()>1 then guojia_yiji = enemy
				else table.insert(males, enemy)	end
				if #males >= 2 then	break end
			end
		end
		if #males==1 and guojia_yiji and not gift then table.insert(males, guojia_yiji) end
		if #males>0 then 
			firstSlashNum = self:getSlashNumber(males[1]) 
		end
		if (#males==1) and #self.friends>0 and not gift then
			first = males[1]
			if zhugeliang_kongcheng then table.insert(males, zhugeliang_kongcheng)
			else
				local friend_maxSlash = findFriend_maxSlash(self,first)
				if friend_maxSlash and firstSlashNum <= self:getSlashNumber(friend_maxSlash) then table.insert(males, friend_maxSlash) end
			end
		end
		
		if (#males >= 2) then
			first = males[1]
			second = males[2]
		end
		
		if second or (first and gift) then
			local lord = self.room:getLord()
			if (first:getHp()<=1) then
				if self.player:isLord() or isRolePredictable() then 
					local friend_maxSlash = findFriend_maxSlash(self,first)
					if friend_maxSlash then second=friend_maxSlash end
				elseif (not lord:hasSkill("wuyan")) then 
					if (self.role=="rebel") and (not first:isLord()) then
						second = lord
					else
						if (self.role=="loyalist" or self.role=="renegade") and (not (first:hasSkill("ganglie") and first:hasSkill("enyuan")) or lord:getHp() >= 3)
							and	(firstSlashNum <= self:getSlashNumber(lord)) then
							second = lord
						end
					end
				end
			end
		
			if first and second then
				if use.to then 
			        use.to:append(first)
			        use.to:append(second)
			        self.lijian_used = true
				end
			end
            use.card=card
		end
	end
end
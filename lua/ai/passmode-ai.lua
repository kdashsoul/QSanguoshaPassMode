local yaoshu_skill={}
yaoshu_skill.name="yaoshu"
table.insert(sgs.ai_skills,yaoshu_skill)
yaoshu_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("YaoshuCard") or self.player:isKongcheng() or #self.enemies == 0 then return end
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)

	self:sortByCardNeed(cards)
	
	for _,card in ipairs(cards)  do
		if card:getSuit() == sgs.Card_Spade and not card:inherits("Shit") then
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

local rende_pass_skill={}
rende_pass_skill.name="rende_pass"
table.insert(sgs.ai_skills,rende_pass_skill)
rende_pass_skill.getTurnUseCard=function(self)
	local rendecards={}
	local retaincards={}
    local dummy_use={}
    local jn = self:getJinkNumber()
    dummy_use.isDummy=true
	local cards=sgs.QList2Table(self.player:getHandcards())
		
	if #self.friends_noself > 0 then 
		for _, card in ipairs(cards) do
			if not (card:inherits("ExNihilo") or card:inherits("Indulgence")) then
				if card:inherits("Jink") and jn == 1 then
					table.insert(retaincards,card:getId())
				else
					if card:inherits("Jink") then jn = jn - 1 end
					self:sortByNeedCard(self.friends_noself,card)
					for _, friend in ipairs(self.friends_noself) do
						if (not friend:containsTrick("indulgence") or friend:getHandcardNum() + 2 < friend:getHp()) and not (friend:hasSkill("kongcheng") and friend:isKongcheng()) then
							if self:cardNeed(card,friend) >= (self:cardNeed(card,self.player) + (self.player:getHp() / 2) ) then
								table.insert(rendecards,card:getId())
							else
								table.insert(retaincards,card:getId())
							end
						end
					end
				end
			end
		end
	end
	
	local overflow = self.player:getHandcardNum() - #rendecards - self.player:getHp()
	
	if #rendecards < 1 and self.player:isWounded() and self.player:getHandcardNum() >= 1 then
		for _, card_id in ipairs(retaincards) do
			table.insert(rendecards,card_id)
			if #rendecards == 1 then break end
		end
	end

	
	if  overflow > 0 and #retaincards > 0 then
		for _, card_id in ipairs(retaincards) do
			table.insert(rendecards,card_id)
			overflow = overflow - 1
			if overflow == 0 then break end
		end
	end	
	

	if #rendecards > 0 then
		return sgs.Card_Parse("@RendePassCard="..table.concat(rendecards,"+")) 
	end
end

sgs.ai_skill_use_func["RendePassCard"]=function(rende_card,use,self)
	if #self.friends_noself == 0 then return end
	local card 
	local cards = sgs.QList2Table(rende_card:getSubcards())
	for _, card_id in ipairs(cards) do
		card = sgs.Sanguosha:getCard(card_id)
		self:sortByNeedCard(self.friends_noself,card)
		for _, friend in ipairs(self.friends_noself) do
			if not (friend:containsTrick("indulgence") and friend:getHandcardNum() - 2 > friend:getHp()) and not (friend:hasSkill("kongcheng") and friend:isKongcheng()) then
				use.card = sgs.Card_Parse("@RendePassCard="..card_id) 
				if use.to then 
					use.to:removeAt(0)
			        use.to:append(friend)
					break
				end
			end
		end
	end
end

local rende_pass_skill={}
rende_pass_skill.name="rende_pass"
table.insert(sgs.ai_skills,rende_pass_skill)
rende_pass_skill.getTurnUseCard=function(self)
	local rendecards={}
	local retaincards={}
    local dummy_use={}
    local jn = self:getJinkNumber()
    dummy_use.isDummy=true
	local cards=sgs.QList2Table(self.player:getHandcards())
		
	if #self.friends_noself > 0 then 
		for _, card in ipairs(cards) do
			if not (card:inherits("ExNihilo") or card:inherits("Indulgence")) then
				if card:inherits("Jink") and jn == 1 then
					table.insert(retaincards,card:getId())
				else
					if card:inherits("Jink") then jn = jn - 1 end
					self:sortByNeedCard(self.friends_noself,card)
					for _, friend in ipairs(self.friends_noself) do
						if (not friend:containsTrick("indulgence") or friend:getHandcardNum() + 2 < friend:getHp()) and not (friend:hasSkill("kongcheng") and friend:isKongcheng()) then
							if self:cardNeed(card,friend) >= (self:cardNeed(card,self.player) + (self.player:getHp() / 2) ) then
								table.insert(rendecards,card:getId())
							else
								table.insert(retaincards,card:getId())
							end
						end
					end
				end
			end
		end
	end
	
	local overflow = self.player:getHandcardNum() - #rendecards - self.player:getHp()
	
	if #rendecards < 1 and self.player:isWounded() and self.player:getHandcardNum() >= 1 then
		for _, card_id in ipairs(retaincards) do
			table.insert(rendecards,card_id)
			if #rendecards == 1 then break end
		end
	end

	
	if  overflow > 0 and #retaincards > 0 then
		for _, card_id in ipairs(retaincards) do
			table.insert(rendecards,card_id)
			overflow = overflow - 1
			if overflow == 0 then break end
		end
	end	
	

	if #rendecards > 0 then
		return sgs.Card_Parse("@RendePassCard="..table.concat(rendecards,"+")) 
	end
end

local jijiang_pass_skill={}
jijiang_pass_skill.name="jijiang_pass"
table.insert(sgs.ai_skills,jijiang_pass_skill)
jijiang_pass_skill.getTurnUseCard=function(self)
	if not (self.player:canSlashWithoutCrossbow() or self.player:hasWeapon("crossbow")) or self.player:hasUsed("JijiangPassCard") then return end
	return sgs.Card_Parse("@JijiangPassCard=.")
end

sgs.ai_skill_use_func["JijiangPassCard"]=function(card,use,self)
	if not self.enemies or #self.enemies == 0 then return end
	if self.player:getHp() == 1 and self.player:getMark("@renyi") == 0 then return end
	self:sort(self.enemies, "hp")
	for _,enemy in ipairs(self.enemies) do
		local def=getDefense(enemy)
		local amr=enemy:getArmor()
		local eff=(not amr) or self.player:hasWeapon("qinggang_sword") or not 
				((amr:inherits("Vine") and not self.player:hasWeapon("fan")))
	    if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
	    elseif self:slashProhibit(nil, enemy) then
	    elseif eff then
			if use.to then 
				use.to:append(enemy) 
			end
			use.card = card
		end
	end
end

sgs.ai_skill_choice.jijiangcost = function(self , choices)
	if self.player:getHp() > 2 then 
		return "losehp"
	else
		return "losemark"
	end
end


local badao_pass_skill={}
badao_pass_skill.name="badao_pass"
table.insert(sgs.ai_skills,badao_pass_skill)
badao_pass_skill.getTurnUseCard=function(self)
	if self.player:isKongcheng() or self.player:getMark("@jianxiong") < 3 then return end
	local fcard
	local cards = self.player:getHandcards()
	local same_suit=false
	cards = sgs.QList2Table(cards)
	self:sortByCardNeed(cards)
	local red,black
	for _, fcard in ipairs(cards) do
		if not (fcard:inherits("Peach") or fcard:inherits("ExNihilo") or fcard:inherits("SavageAssault") or fcard:inherits("ArcheryAttack")) then
			if fcard:isRed() then 
				red = sgs.Card_Parse(("archery_attack:badao_pass[%s:%s]=%d"):format(fcard:getSuitString(), fcard:getNumberString(), fcard:getId()))
			else
				black = sgs.Card_Parse(("savage_assault:badao_pass[%s:%s]=%d"):format(fcard:getSuitString(), fcard:getNumberString(), fcard:getId()))
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

sgs.ai_skill_use_func["BadaoPassCard"]=function(card,use,self)
    use.card=card
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
	if not self.enemies or #self.enemies == 0 then return end
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


sgs.ai_skill_invoke.jueduan_pass = function(self, data)
	local target = data:toPlayer()
	return not self:isFriend(target)
end

sgs.ai_skill_invoke.longhou  = function(self, data)
	local target = data:toPlayer()
	return not self:isFriend(target) and not self.player:isKongcheng()
end

sgs.ai_skill_invoke.jianxiong_pass  = function(self, data)
	local card = data:toCard() 
	return not sgs.Shit_HasShit(card) -- and not (card:inherits("Slash") and not self.player:hasWeapon("crossbow"))
end

sgs.ai_skill_invoke.dujiang_pass  = function(self, data)
	return (self.player:getEquips():length() > 1 and self.player:getEquips():length() < 4)
end

sgs.ai_skill_invoke.jielue_pass = sgs.ai_skill_invoke.jueduan_pass
sgs.ai_skill_invoke.wushen_pass = sgs.ai_skill_invoke.jueduan_pass
sgs.ai_skill_invoke.tongji_pass = sgs.ai_skill_invoke.jueduan_pass
sgs.ai_skill_invoke.jumou_pass = true
sgs.ai_skill_invoke.zhaolie_pass = true
sgs.ai_skill_invoke.jinghong_pass = true
sgs.ai_skill_invoke.nuozhan_pass = function(self, data)
	if self.enemies and #self.enemies > 0 and self:getSlashNumber() > 0 then
		for _,enemy in ipairs(self.enemies) do
			if self:slashValid(enemy, sgs.Card_Parse(card_id), player) then
				return true
			end
		end	
	end
	return false
end
sgs.ai_skill_invoke.yitian = sgs.ai_skill_invoke.nuozhan_pass
sgs.ai_skill_invoke.longwei = function(self, data)
	return self.enemies and #self.enemies > 0
end

sgs.ai_skill_invoke["@quwu_pass"] = function(self, data)
	player = self.player
	if player:getOffensiveHorse() then return player:getOffensiveHorse():getId() end
	if player:getWeapon() then return player:getWeapon():getId() end
	if player:getArmor() then return player:getArmor():getId() end
	if player:getDefensiveHorse() then return player:getDefensiveHorse():getId() end
end

local kurou_pass_skill={}
kurou_pass_skill.name="kurou_pass"
table.insert(sgs.ai_skills,kurou_pass_skill)
kurou_pass_skill.getTurnUseCard=function(self,inclusive)
    if  self.player:getHp() > 2 and self.player:getHandcardNum() < self.player:getHp() then
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
    if self.player:getCards("he"):length() < 4 or (not self.player:isWounded()) or self.player:hasUsed("ZhaxiangCard") then return end
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
	
	cards = self.player:getEquips()
	for _, zcard in sgs.qlist(cards) do
		if not zcard:inherits("Armor") or zcard:inherits("SilverLion") then
			table.insert(unpreferedCards,zcard:getId())
		end
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

local lijian_pass_skill={}
lijian_pass_skill.name="lijian_pass"
table.insert(sgs.ai_skills,lijian_pass_skill)
lijian_pass_skill.getTurnUseCard=function(self)
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
	return nil
end

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
		--if (#males==0) then self:log("No real men!") end
		if #males>0 then 
			firstSlashNum = self:getSlashNumber(males[1]) 
		end
		if (#males==1) and #self.friends>0 and not gift then
			--self:log("Only 1")
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

sgs.ai_skill_use["slash"]=function(self, data)
	player = player or self.player	
	local slash_id = self:getCardId("Slash")
	if not slash_id then return "." end
	local target
	self:sort(self.enemies, "hp")
	
	for _,enemy in ipairs(self.enemies) do
		if self:slashValid(enemy, sgs.Card_Parse(card_id), player) then
			target = enemy
		end
	end
	
	if target then
		return slash_id.."->"..target:objectName()
	end
end	
sgs.ai_skill_invoke.ice_sword=function(self, data)
	if self.player:hasFlag("drank") then return false end
	local effect = data:toSlashEffect() 
	local target = effect.to
	if self:isFriend(target) then return false end
	local hasPeach
	local cards = target:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:inherits("Peach") or card:inherits("Analeptic") then hasPeach = true break end
	end
	if hasPeach then return true end
	if (target:getArmor()) and target:getHp() > 2 then return true end
	return false
end

local spear_skill={}
spear_skill.name="spear"
table.insert(sgs.ai_skills,spear_skill)
spear_skill.getTurnUseCard=function(self,inclusive)
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
    
    if #cards<2 then return end
    if not self:hasSkills(sgs.need_kongcheng) then
	    if #cards<(self.player:getHp()+1) then return end
	    if self:getCardId("Slash") then return end
    end
    
    self:sortByUseValue(cards,true)
    
    local suit1 = cards[1]:getSuitString()
	local card_id1 = cards[1]:getEffectiveId()
	
	local suit2 = cards[2]:getSuitString()
	local card_id2 = cards[2]:getEffectiveId()
	
	local suit=sgs.Card_NoSuit
	if cards[1]:isBlack() and cards[2]:isBlack() then suit=sgs.Card_Club
	elseif cards[1]:isBlack()==cards[2]:isBlack() then suit=sgs.Card_Diamond end
	
	local card_str = ("slash:spear[%s:%s]=%d+%d"):format(suit, 0, card_id1, card_id2)
	
    local slash = sgs.Card_Parse(card_str)
    
    return slash
    
end

local qixi_skill={}
qixi_skill.name="qixi"
table.insert(sgs.ai_skills,qixi_skill)
qixi_skill.getTurnUseCard=function(self,inclusive)
    local cards = self.player:getCards("he")	
    cards=sgs.QList2Table(cards)
	
	local black_card
	self:sortByUseValue(cards,true)
	local has_weapon=false
	
	for _,card in ipairs(cards)  do
	    if card:inherits("Weapon") and card:isRed() then has_weapon=true end
	end
	local overflow = self:getOverflow()

	for _,card in ipairs(cards)  do
		if card:isBlack() then
			local suit = card:getSuitString()
			local number = card:getNumberString()
			local card_id = card:getEffectiveId()
			local vdis = sgs.Card_Parse(("dismantlement:qixi[%s:%s]=%d"):format(suit, number, card_id))	
			if ((self:cardNeed(card)<self:cardNeed(vdis)) or inclusive) then
			    local shouldUse=true
			    if card:inherits("Armor") then
					if card:inherits("SilverLion") and self:hasEquip(card) then
						shouldUse=self.player:isWounded()
					else
		                if (not self.player:getArmor()) or self:hasEquip(card) then shouldUse=false end
					end
	            end
	            
	            if card:inherits("Weapon") then
	                if not self.player:getWeapon() then shouldUse=false
	                elseif self:hasEquip(card) and not has_weapon then shouldUse=false
	                end
	            end
			    
			    if shouldUse then
				    black_card = card
				    overflow = overflow - 1
				    break
				end
			end
		end
	end

	if black_card then
		local suit = black_card:getSuitString()
		local number = black_card:getNumberString()
		local card_id = black_card:getEffectiveId()
		local card_str = ("dismantlement:qixi[%s:%s]=%d"):format(suit, number, card_id)
		local dismantlement = sgs.Card_Parse(card_str)
		assert(dismantlement)
        return dismantlement
	end
end

local wusheng_skill={}
wusheng_skill.name="wusheng"
table.insert(sgs.ai_skills,wusheng_skill)
wusheng_skill.getTurnUseCard=function(self,inclusive)
    local cards = self.player:getCards("he")	
    cards=sgs.QList2Table(cards)
	
	local red_card
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards)  do
		if card:isRed() and not card:inherits("Slash") and not card:inherits("Peach") 				--not peach
			and ((self:getUseValue(card)<sgs.ai_use_value["Slash"]) or inclusive) then
			red_card = card
			break
		end
	end

	if red_card then		
		local suit = red_card:getSuitString()
    	local number = red_card:getNumberString()
		local card_id = red_card:getEffectiveId()
		local card_str = ("slash:wusheng[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)
		
		assert(slash)
        return slash
	end
end

local longdan_skill={}
longdan_skill.name="longdan"
table.insert(sgs.ai_skills,longdan_skill)
longdan_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	
	local jink_card
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards)  do
		if card:inherits("Jink") then
			jink_card = card
			break
		end
	end
	
    if not jink_card then return nil end
	local suit = jink_card:getSuitString()
	local number = jink_card:getNumberString()
	local card_id = jink_card:getEffectiveId()
	local card_str = ("slash:longdan[%s:%s]=%d"):format(suit, number, card_id)
	local slash = sgs.Card_Parse(card_str)
    assert(slash)
    
    return slash
end


local fanjian_skill={}
fanjian_skill.name="fanjian"
table.insert(sgs.ai_skills,fanjian_skill)
fanjian_skill.getTurnUseCard=function(self)
    if self.player:isKongcheng() then return end
    if self.player:hasUsed("FanjianCard") then return end
	
	local cards = self.player:getHandcards()
	
	for _, card in sgs.qlist(cards) do
		if card:getSuit() == sgs.Card_Diamond and self.player:getHandcardNum() == 1 then
			return nil
		elseif card:inherits("Peach") or card:inherits("Analeptic") then
			return nil
		end
	end
	
	local card_str = "@FanjianCard=."
	local fanjianCard = sgs.Card_Parse(card_str)
    assert(fanjianCard)
    return fanjianCard
end

sgs.ai_skill_use_func["FanjianCard"]=function(card,use,self)
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do								
		if (not enemy:hasSkill("qingnang")) or (enemy:getHp() == 1 and enemy:getHandcardNum() == 0 and not enemy:getEquips()) then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end
end

local jieyin_skill={}
jieyin_skill.name="jieyin"
table.insert(sgs.ai_skills,jieyin_skill)
jieyin_skill.getTurnUseCard=function(self)
	if self.player:getHandcardNum()<2 then return end
	if self.player:hasUsed("JieyinCard") then return end
		
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
		
	self:sortByUseValue(cards,true)
		
	local first  = cards[1]:getEffectiveId()
	local second = cards[2]:getEffectiveId()

	local card_str = ("@JieyinCard=%d+%d"):format(first, second)
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func["JieyinCard"]=function(card,use,self)
	self:sort(self.friends, "hp")
	for _, friend in ipairs(self.friends) do
		if friend:getGeneral():isMale() and friend:isWounded() then
			use.card=card
			if use.to then use.to:append(friend) end
			return
		end
	end
end

local qingnang_skill={}
qingnang_skill.name="qingnang"
table.insert(sgs.ai_skills,qingnang_skill)
qingnang_skill.getTurnUseCard=function(self)
	if self.player:isKongcheng() then return end
    if self.player:hasUsed("QingnangCard") then return end
		
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
		
	self:sortByKeepValue(cards)

	local card_str = ("@QingnangCard=%d"):format(cards[1]:getId())
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func["QingnangCard"]=function(card,use,self)
	self:sort(self.friends, "hp")
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() then
			use.card=card
			if use.to then use.to:append(friend) end
			return
		end
	end
end

local kurou_skill={}
kurou_skill.name="kurou"
table.insert(sgs.ai_skills,kurou_skill)
kurou_skill.getTurnUseCard=function(self,inclusive)
    if  (self.player:getHp() > 3 and self.player:getHandcardNum() > self.player:getHp()) or	(self.player:getHp() - self.player:getHandcardNum() >= 2) then
		return sgs.Card_Parse("@KurouCard=.")
    end
		
	if self:isEquip("Crossbow") then
        for _, enemy in ipairs(self.enemies) do
            if self.player:canSlash(enemy,true) and (self.player:getHp()>1 or self:getAnalepticNum() > 0) then
                return sgs.Card_Parse("@KurouCard=.")
            end
        end
    end
end

sgs.ai_skill_use_func["KurouCard"]=function(card,use,self)
	if not use.isDummy then self:speak("kurou") end
	use.card=card
end

local jijiang_skill={}
jijiang_skill.name="jijiang"
table.insert(sgs.ai_skills,jijiang_skill)
jijiang_skill.getTurnUseCard=function(self)
	if not self:slashIsAvailable() then return nil end
	if self.jijiang_used then return nil end
	local card_str = "@JijiangCard=."
	local slash = sgs.Card_Parse(card_str)
	assert(slash)
	return slash
end

sgs.ai_skill_use_func["JijiangCard"]=function(card,use,self)
	self:sort(self.enemies, "expect")
	local target_count=0
    for _, enemy in ipairs(self.enemies) do
		if ((self.player:canSlash(enemy, not no_distance)) or (use.isDummy and (self.player:distanceTo(enemy)<=self.predictedRange)))
			and self:objectiveLevel(enemy)>3 and self:slashIsEffective(card, enemy) and not self:slashProhibit(nil, enemy) then
                use.card=card
                if use.to then 
                    use.to:append(enemy) 
                    self.jijiang_used=true
                end
                target_count=target_count+1
                if self.slash_targets<=target_count then return end
		end
   end
end

local rende_skill={}
rende_skill.name="rende"
table.insert(sgs.ai_skills,rende_skill)
rende_skill.getTurnUseCard=function(self)
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
							--self:log("card:"..card:className().."friend-".. friend:getGeneralName()..":"..self:cardNeed(card,friend).."liubei:"..self:cardNeed(card,self.player))				
							local retain_const = 0 
							if self.player:getRole() == "renegade" or friend:getRole() == "renegade" then retain_const = (#self.friends_noself + 1) end
							if self:cardNeed(card,friend) >= (self:cardNeed(card,self.player) + (self.player:getHp() / 2) + retain_const) then
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
	
	if #rendecards < 2 and self.player:isWounded() and self.player:getHandcardNum() >= 2 then
		for _, card_id in ipairs(retaincards) do
			table.insert(rendecards,card_id)
			if #rendecards == 2 then break end
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
		return sgs.Card_Parse("@RendeCard="..table.concat(rendecards,"+")) 
	end
end

sgs.ai_skill_use_func["RendeCard"]=function(rende_card,use,self)
	if #self.friends_noself == 0 then return end
	local card 
	--self:log(rende_card:getSubcards():length())
	local cards = sgs.QList2Table(rende_card:getSubcards())
	for _, card_id in ipairs(cards) do
		card = sgs.Sanguosha:getCard(card_id)
		self:sortByNeedCard(self.friends_noself,card)
		for _, friend in ipairs(self.friends_noself) do
			if not (friend:containsTrick("indulgence") and friend:getHandcardNum() - 2 > friend:getHp()) and not (friend:hasSkill("kongcheng") and friend:isKongcheng()) then
				use.card = sgs.Card_Parse("@RendeCard="..card_id) 
				if use.to then 
					use.to:removeAt(0)
			        use.to:append(friend)
					break
				end
			end
		end
	end
end

local guose_skill={}
guose_skill.name="guose"
table.insert(sgs.ai_skills,guose_skill)
guose_skill.getTurnUseCard=function(self,inclusive)
    local cards = self.player:getCards("he")	
    cards=sgs.QList2Table(cards)
	
	local card
	
	self:sortByUseValue(cards,true)
	
	local has_weapon=false
	
	for _,acard in ipairs(cards)  do
	    if acard:inherits("Weapon") and not (acard:getSuit() == sgs.Card_Diamond) then has_weapon=true end
	end
	
	for _,acard in ipairs(cards)  do
		if (acard:getSuit() == sgs.Card_Diamond) and ((self:getUseValue(acard)<sgs.ai_use_value["Indulgence"]) or inclusive) then
		    local shouldUse=true
		    
		    if acard:inherits("Armor") then
                if not self.player:getArmor() then shouldUse=false 
                elseif self:hasEquip(acard) then shouldUse=false
                end
            end
            
            if acard:inherits("Weapon") then
                if not self.player:getWeapon() then shouldUse=false
                elseif self:hasEquip(acard) and not has_weapon then shouldUse=false
                end
            end
		    
		    if shouldUse then
			    card = acard
			    break
			end
		end
	end
	
    if not card then return end
	local number = card:getNumberString()
    local card_id = card:getEffectiveId()
	local card_str = ("indulgence:guose[diamond:%s]=%d"):format(number, card_id)	
	local indulgence = sgs.Card_Parse(card_str)
	
    assert(indulgence)
    
    return indulgence
end

local zhiheng_skill={}
zhiheng_skill.name="zhiheng"
table.insert(sgs.ai_skills,zhiheng_skill)
zhiheng_skill.getTurnUseCard=function(self)
	local unpreferedCards={}
	local cards=sgs.QList2Table(self.player:getHandcards())
	
	if not self.player:hasUsed("ZhihengCard") then 
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
		return sgs.Card_Parse("@ZhihengCard="..table.concat(unpreferedCards,"+")) 
	end
end
sgs.ai_skill_use_func["ZhihengCard"]=function(card,use,self)
	use.card = card
end

local lijian_skill={}
lijian_skill.name="lijian"
table.insert(sgs.ai_skills,lijian_skill)
lijian_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("LijianCard") then
		return 
	end
	if not self.player:isNude() then
		local card
		local card_id
		if self.player:getArmor() and self.player:isWounded() and self.player:getArmor():objectName()=="silver_lion" then
			card = sgs.Card_Parse("@LijianCard=" .. self.player:getArmor():getId())
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
			card = sgs.Card_Parse("@LijianCard=" .. card_id)
			return card
		end
	end
	return nil
end

sgs.ai_skill_use_func["LijianCard"]=function(card,use,self)
	local findFriend_maxSlash=function(self,first)
		self:log("Looking for the friend!")
		local maxSlash = 0
		local friend_maxSlash
		for _, friend in ipairs(self.friends_noself) do
			if self:getSlashNumber(friend)>= maxSlash and friend:getGeneral():isMale() then
				maxSlash=self:getSlashNumber(friend)
				friend_maxSlash = friend
			end
		end
		if friend_maxSlash then
			local safe = false
			if (first:hasSkill("ganglie") or first:hasSkill("enyuan")) then
				if (first:getHp()<=1 and self:getSlashNumber(first)==0) then safe=true end
			elseif (self:getSlashNumber(friend_maxSlash) >= first:getHandcardNum()) then safe=true end
			if safe then return friend_maxSlash end
		else self:log("unfound")
		end
		return nil
	end

	if not self.player:hasUsed("LijianCard") then 
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
			if enemy:getGeneral():isMale() and not enemy:hasSkill("wuyan") then
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
		if (#males==1) and #self.friends_noself>0 and not gift then
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
				elseif (lord:getGeneral():isMale()) and (not lord:hasSkill("wuyan")) then 
					if (self.role=="rebel") and (not first:isLord()) then
						second = lord
					else
						if (self.role=="loyalist" or self.role=="renegade") and (not (first:hasSkill("ganglie") and first:hasSkill("enyuan")) or lord:getHp() >= 3)
							and	(firstSlashNum <= self:getSlashNumber(lord)) then
							--the first male maybe have a "Slash" Card
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


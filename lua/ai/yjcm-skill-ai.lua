-- gaoshun
local jiejiu_skill={}
jiejiu_skill.name="jiejiu"
table.insert(sgs.ai_skills,jiejiu_skill)
jiejiu_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	
	local anal_card
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards)  do
		if card:inherits("Analeptic") then 
			anal_card = card
			break
		end
	end

	if anal_card then		
		local suit = anal_card:getSuitString()
    	local number = anal_card:getNumberString()
		local card_id = anal_card:getEffectiveId()
		local card_str = ("slash:jiejiu[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)
		
		assert(slash)
        return slash
	end
end

local xianzhen_skill={}
xianzhen_skill.name="xianzhen"
table.insert(sgs.ai_skills,xianzhen_skill)
xianzhen_skill.getTurnUseCard=function(self)
    
    if self.xianzhen_used then 
        local card_str = "@XianzhenSlashCard=."
	    local card = sgs.Card_Parse(card_str)
	    return card
	end
    
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
    
    local max_card = self:getMaxCard()
    if not max_card then return nil end
	local max_point = max_card:getNumber()
	
	local slashNum=self:getSlashNumber(self.player)
	if max_card:inherits("Slash") then slashNum=slashNum-1 end
	
	if slashNum<2 then return nil end
	
	self:sort(self.enemies, "hp")
	
	for _, enemy in ipairs(self.enemies) do
	
	    local enemy_max_card = self:getMaxCard(enemy)
		if enemy_max_card and max_point > enemy_max_card:getNumber() then
		    
		    local slash=self:getCard("Slash")
		    local dummy_use={}
            dummy_use.isDummy=true
            
            local no_distance=true
		    self:useBasicCard(slash,dummy_use,no_distance)
		    
		    if dummy_use.card then 
		        
		    
		        local card_id = max_card:getEffectiveId()
			    local card_str = "@XianzhenCard=" .. card_id
			    local card = sgs.Card_Parse(card_str)

		        return card
		    end
		end
	end
end

sgs.ai_skill_use_func["XianzhenSlashCard"]=function(card,use,self)
	local target = self.player:getTag("XianzhenTarget"):toPlayer() 
	local slash = self:getCard("Slash")
    if slash and not target:isDead() and self:slashValid(target,slash) then
        use.card=card
    end
end

sgs.ai_skill_use_func["XianzhenCard"]=function(card,use,self)
	self:sort(self.enemies, "handcard")
	local max_card = self:getMaxCard(self.player)
	local max_point = max_card:getNumber()
	
	for _, enemy in ipairs(self.enemies) do
	    local enemy_max_card = self:getMaxCard(enemy)
		if not (enemy:hasSkill("kongcheng") and enemy:getHandcardNum() == 1) 
			and (enemy_max_card and max_point > enemy_max_card:getNumber()) then
		    if use.to then 
		        self.xianzhen_used = true
		        use.to:append(enemy)
            end
            use.card=card
            break
		end
	end
end

local xinzhan_skill={}
xinzhan_skill.name="xinzhan"
table.insert(sgs.ai_skills,xinzhan_skill)
xinzhan_skill.getTurnUseCard=function(self,inclusive)
	if not self.player:hasUsed("XinzhanCard") and self.player:getHandcardNum() > self.player:getMaxHP() then
		return sgs.Card_Parse("@XinzhanCard=.") 
	end
end

sgs.ai_skill_use_func["XinzhanCard"]=function(card,use,self)
	use.card=card
end

--xusheng
sgs.ai_skill_invoke.pojun = function(self, data)
	local damage = data:toDamage()
	
	if not damage.to:faceUp() then
		return self:isFriend(damage.to)
	end		
	
	local good = damage.to:getHp() > 2	
	if self:isFriend(damage.to) then
		return good
	elseif self:isEnemy(damage.to) then
		return not good
	end
end


--caozhi
sgs.ai_skill_invoke.jiushi= true

-- wuguotai
sgs.ai_skill_invoke.buyi = function(self, data)
	local dying = data:toDying()
	return self:isFriend(dying.who)
end

sgs.ai_cardshow.buyi = function(self, requestor)
	assert(self.player:objectName() == requestor:objectName())

	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:getTypeId() ~= sgs.Card_Basic then
			return card
		end
	end

	return self.player:getRandomHandCard()
end

local ganlu_skill={}
ganlu_skill.name="ganlu"
table.insert(sgs.ai_skills,ganlu_skill)
ganlu_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("GanluCard") then return end
	return sgs.Card_Parse("@GanluCard=.")
end

sgs.ai_skill_use_func["GanluCard"]=function(card,use,self)
	local lost_hp = self.player:getLostHp()
	local t1,t2
	local max_equip = 0
	local target
	
	if not self.player:hasUsed("GanluCard") then
		local lose_equip_skiller = {}
		for _, skill_name in ipairs(sgs.lose_equip_skill:split("|")) do
			local friend = self:findHasSkill(self.friends,skill_name)
			if friend then 
				table.insert(lose_equip_skiller,friend) 
			end
		end	

		if #lose_equip_skiller >= 2 then
			local n1 = self:getEquipNumber(lose_equip_skiller[1])
			local n2 = self:getEquipNumber(lose_equip_skiller[2])
			if n1 + n2 ~= 0 and math.abs(n1 - n2) <= lost_hp then
				t1 = n1
				t2 = n2
			end
		end
		
		if not t2 and #lose_equip_skiller >= 1 then
			t1 = lose_equip_skiller[1]
			local n1 = self:getEquipNumber(t1)
			for _, friend in ipairs(self.friends) do
				if friend:objectName() ~= t1:objectName() then
					local n2 = self:getEquipNumber(friend)
					if math.abs(n1 - n2) <= lost_hp and (n1 + n2 ~= 0) then 
						if n2 >= max_equip then
							max_equip = n2
							t2 = friend
						end
					end
				end
			end
			if not t2 then
				for _, enemy in ipairs(self.enemies) do
					local n2 = self:getEquipNumber(enemy)
					if n2 > 0 and math.abs(n1 - n2) <= lost_hp and n2 >= n1 then 
						if n2 >= max_equip then
							max_equip = n2
							t2 = enemy
						end
					end
				end
			end
		end
		if not t2 then
			local max_diff = 0
			for _, friend in ipairs(self.friends) do
				local n1 = self:getEquipNumber(friend)
				for _, enemy in ipairs(self.enemies) do
					if not self:hasSkills(sgs.lose_equip_skill,enemy) then
						local n2 = self:getEquipNumber(enemy)
						if n2 > 0 and math.abs(n1 - n2) <= lost_hp and n2 > n1 and math.abs(n1 - n2) > max_diff then
							t1 = friend
							t2 = enemy
						end
					end
				end
			end	
		end
	end
	
	if t1 and t2 then
		if use.to then
			use.to:append(t1)
			use.to:append(t2)
		end
		use.card = card
	end
end

--lingtong
sgs.ai_skill_choice.xuanfeng = function(self, choices)
	self:sort(self.enemies, "expect")
	local slash = sgs.Card_Parse(("slash[%s:%s]"):format(sgs.Card_NoSuit, 0))
	for _, enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy)<=1 then
			return "damage"
		elseif not self:slashProhibit(slash ,enemy) then
			return "slash"
		end
	end
	return "nothing"
end

sgs.ai_skill_playerchosen.xuanfeng_damage = function(self,targets)
	self:sort(self.enemies, "expect")
	for _, enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy)<=1 then return enemy end
	end
end

sgs.ai_skill_playerchosen.xuanfeng_slash = function(self,targets)
	local slash = sgs.Card_Parse(("slash[%s:%s]"):format(sgs.Card_NoSuit, 0))
	self:sort(self.enemies, "expect")
	for _, enemy in ipairs(self.enemies) do
		if not self:slashProhibit(slash ,enemy) then return enemy end
	end
--	self:log("unfound")
	return self.enemies:at(math.random(0, targets:length() - 1))
end

--fazheng
local xuanhuo_skill={}
xuanhuo_skill.name="xuanhuo"
table.insert(sgs.ai_skills,xuanhuo_skill)
xuanhuo_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("XuanhuoCard") then return end
	return sgs.Card_Parse("@XuanhuoCard=.")
end

sgs.ai_skill_use_func["XuanhuoCard"]=function(card,use,self)
	if self.player:isKongcheng() or self:getSuitNum("heart",false) == 0 then return end
	local cards = self.player:getHandcards()
	local card2friend , card2enemy , target
	cards=sgs.QList2Table(cards)
	self:sortByCardNeed(cards,true)
	for _, card in ipairs(cards)do
		if card:getSuit() == sgs.Card_Heart then
			if not card:inherits("Peach") then
				card2enemy = card
			elseif not card:inherits("Shit") then
				card2friend = card
			else
				card2friend = card
				card2enemy = card
			end
			if card2enemy and card2friend then break end
		end	
	end
		
	if card2friend then
		local friend = self:findHasSkill(self.friends_noself,sgs.lose_equip_skill)
		if friend and friend:getEquips():length() > 0 then
			target = friend 
		else
			for _, friend in ipairs(self.friends_noself) do
				if self:isLionValid(friend) then
					target = friend
					break
				end
			end
		end
	end
	if not target and card2enemy then
		for _, enemy in ipairs(self.enemies) do
			if self:isEquip("EightDiagram",enemy) or self:isEquip("RenwangShield",enemy) or enemy:getDefensiveHorse() then
				target = enemy
			end
		end
	end
	if not target and card2friend and self:getOverflow() > 0 and #self.friends_noself > 0 then
		target = self.friends_noself[1]
	end
	
	if target then
		local card_id
		if self:isFriend(target) then
			card_id = card2friend:getEffectiveId()
		else
			card_id = card2enemy:getEffectiveId()
		end
		if use.to then
			use.to:append(target)
		end
		use.card = sgs.Card_Parse("@XuanhuoCard=" .. card_id)
	end
end


sgs.ai_skill_playerchosen["xuanhuo"] = function(self, targets)
	local friend = self:findHasSkill(self.friends,sgs.lose_equip_skill)
	if friend then return friend end
	
	for _, player in sgs.qlist(targets) do
		if (player:getHandcardNum() <= 2 or player:getHp() < 2) and self:isFriend(player) and not player:getArmor() then
			return player
		end
	end
	
	for _, player in sgs.qlist(targets) do
		if self:isFriend(player) then
			return player
		end
	end	
end


--xushu
local jujian_skill={}
jujian_skill.name="jujian"
table.insert(sgs.ai_skills,jujian_skill)
jujian_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("JujianCard") then return end
	return sgs.Card_Parse("@JujianCard=.")
end

sgs.ai_skill_use_func["JujianCard"]=function(card,use,self)
	if #self.friends_noself == 0 or self.player:isAllNude() then return end
	
	local trick_num, basic_num, equip_num = 0, 0, 0
	local card_str,target
	local abandon_cards = {}
	local slash_num = self:getSlashNumber(self.player)
	local jink_num = self:getJinkNumber(self.player)
	
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	self:sortByCardNeed(cards, true)
	
	if self.player:getHp() <= 2 and self.player:getHandcardNum() >=3 and self:getPeachNum() == 0 then
		for _, card in ipairs(cards) do 
			if card:getTypeId() == sgs.Card_Trick and not card:inherits("ExNihilo") then trick_num = trick_num + 1
			elseif card:getTypeId() == sgs.Card_Basic then basic_num = basic_num + 1
			elseif card:getTypeId() == sgs.Card_Equip then equip_num = equip_num + 1
			end
		end
	
		local result_class
		if trick_num >= 3 then result_class = "TrickCard"
		elseif equip_num >= 3 then result_class = "EquipCard"
		elseif basic_num >= 3 then result_class = "BasicCard"
		end
		
		if self:isLionValid() and equip_num >= 2 then
			result_class = "EquipCard"
			table.insert(abandon_cards, self.player:getArmor():getId())
		end
		
		for _, fcard in ipairs(cards) do 
			if #abandon_cards == 3 then break end
			if fcard:inherits(result_class) and not fcard:inherits("ExNihilo") then
				table.insert(abandon_cards, fcard:getId())
			end
		end
	else 
		for _, card in ipairs(cards) do
			if #abandon_cards == 3 then break end
			if card:inherits("Slash") and slash_num > 1 then
				if not self:isEquip("Crossbow") then
					table.insert(abandon_cards, card:getId())
					slash_num = slash_num - 1
				end
			elseif card:inherits("Jink") and jink_num > 1 then
				table.insert(abandon_cards, card:getId())
				jink_num = jink_num - 1
			elseif not card:inherits("Nullification") and not card:inherits("EquipCard") and 
				not card:inherits("Peach") and not card:inherits("Indulgence") and not card:inherits("SupplyShortage") then
				table.insert(abandon_cards, card:getId())	
			end
		end	
	end
	
	for _, friend in ipairs(self.friends_noself) do
		if (friend:getHandcardNum()<2) or (friend:getHandcardNum()<friend:getHp()+1) then
			target = friend
		end
	end
	
	if #abandon_cards > 0 then
		if use.to then
			use.to:append(target)
		end
		use.card = sgs.Card_Parse("@JujianCard=" .. table.concat(abandon_cards, "+"))
	end
end


--chengong
mingce_skill={}
mingce_skill.name="mingce"
table.insert(sgs.ai_skills,mingce_skill)
mingce_skill.getTurnUseCard=function(self)
	if self.player:isNude() or #self.friends_noself == 0 then return end
	local card
	if self:isLionValid() then card = self.player:getArmor() end
	if not card then
		local hcards = self.player:getCards("h")
		hcards = sgs.QList2Table(hcards)
		self:sortByCardNeed(hcards, true)

		for _, hcard in ipairs(hcards) do
			if hcard:inherits("Slash") or hcard:inherits("EquipCard") then
				card = hcard
				break
			end
		end
	end
	if card then
		card = sgs.Card_Parse("@MingceCard=" .. card:getEffectiveId()) 
		return card
	end
end

sgs.ai_skill_use_func["MingceCard"]=function(card,use,self)
	if self.player:hasUsed("MingceCard") then return end
	local target
	self:sort(self.friends_noself, "expect")
	self:sort(self.enemies, "expect")
	
	for _, friend in ipairs(self.friends_noself) do
		for _, enemy in ipairs(self.enemies) do
			if self:isWeak(enemy) and self:slashValid(enemy,nil,friend) then
				target = friend
			end 
		end
	end

	if not target then
		target = self.friends_noself[1]
	end

	if target then
		use.card=card
		if use.to then
			use.to:append(target)
		end
	end
end

sgs.ai_skill_choice.mingce = function(self , choices)
	self:sort(self.enemies, "expect")
	for _, enemy in ipairs(self.enemies) do
		if (self:isWeak(enemy) or self:getOverflow() >= 0) and self:slashValid(enemy) then
			return "use"
		end 
	end
    return "draw"
end

sgs.ai_skill_playerchosen.mingce = function(self,targets)
	local slash = sgs.Card_Parse(("slash[%s:%s]"):format(sgs.Card_NoSuit, 0))
	local targetlist=sgs.QList2Table(targets)

	self:sort(targetlist, "expect")
	for _, target in ipairs(targetlist) do
		if self:isEnemy(target) and self.player:canSlash(target) and not self:slashProhibit(slash ,target) then
			return target
		end
	end
end
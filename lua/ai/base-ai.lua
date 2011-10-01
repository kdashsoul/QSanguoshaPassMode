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
	--self:log("view-------------------------view"..class_name)
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
			elseif player:hasSkill("zhanshen_pass") and (card:inherits("TrickCard") or card:inherits("EquipCard"))then
				return ("slash:zhanshen_pass[%s:%s]=%d"):format(suit, number, card_id)
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


function SmartAI:getCardValue(card,player)
	player = player or self.player
	--self:log(card:objectName()..player:getGeneralName()..SmartAI.GetExpect(player))
	local expect = SmartAI.GetExpect(player) 
	if card:inherits("TrickCard") then
		if card:inherits("Duel") or card:inherits("FireAttack") then
			if self:hasSkills("jianxiong",player)  then
				expect = expect + 10
			end
		elseif card:inherits("IronChain") then
			if self:isEquip("Vine",player)  then
				expect = expect - 10
			elseif self:isEquip("SilverLion",player)  then
				expect = expect + 5
			end
		elseif card:inherits("SupplyShortage") then
			expect = expect + player:getHandcardNum() * 2
			if self:hasSkills("yongsi",player)  then
				expect = expect - 10
			end
			if self:hasSkills("tiandu|guanxing",player)  then
				expect = expect + 10
			end
		elseif card:inherits("Indulgence") then
			expect = expect + (player:getHp() - player:getHandcardNum()) * 2
			if self:hasSkills("keji|conghui|guanxing",player) then
				expect = expect + 10
			end
		end
	end
	return expect
end

function SmartAI:sortByCardValue(players,card)
	local compare_func = function(a,b)
		local value1 = self:getCardValue(card,a)
		local value2 = self:getCardValue(card,b)
		return value1 < value2
	end
	table.sort(players, compare_func)
end

function SmartAI:getDistanceLimit(card,player)
	player = player or self.player
	if player:hasSkill("qicai") then
		return nil
	end
	if card:inherits("Snatch") then
		return 1
	elseif card:inherits("SupplyShortage") then
		if player:hasSkill("duanliang") then
			return 2
		else
			return 1
		end
	end
end

function SmartAI:getOverflow(player)
	player = player or self.player
	-- keji ?
	return math.max(player:getHandcardNum() - player:getHp(), 0)
end

function SmartAI:getJiemingChaofeng(player)
	local max_x , chaofeng = 0 , 0
    for _, friend in ipairs(self:getFriends(player)) do
		local x = math.min(friend:getMaxHP(), 5) - friend:getHandcardNum()		
		if x > max_x then
			max_x = x
		end
    end
	if max_x < 2 then
		chaofeng = 5 - max_x * 2
	else
		chaofeng = (-max_x) * 2
	end    
    return chaofeng
end
--jueji
local jueji_skill={}
jueji_skill.name="jueji"
table.insert(sgs.ai_skills,jueji_skill)
jueji_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("JuejiCard") then
		return 
	end
	local card
	local first_found, second_found = false, false
	local first_card, second_card
	if self.player:getHandcardNum() >= 2 then
		local cards = self.player:getHandcards()
		local same_suit=false
		cards = sgs.QList2Table(cards)
		for _, fcard in ipairs(cards) do
			if not fcard:inherits("Peach") then
				first_card = fcard
				first_found = true
				for _, scard in ipairs(cards) do
					if first_card ~= scard and first_card:sameColorWith(scard) and not scard:inherits("Peach") then
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
		local jueji_card = {}
		local first_suit, first_number, first_id = first_card:getSuitString(), first_card:getNumberString(), first_card:getId()
		local second_suit, second_number, second_id = second_card:getSuitString(), second_card:getNumberString(), second_card:getId()
		table.insert(jueji_card, first_card:getId())
		table.insert(jueji_card, second_card:getId())
		card = sgs.Card_Parse("@JuejiCard=" .. table.concat(jueji_card,"+")) 
		assert(card)
		return card
	end	
	return nil
end


sgs.ai_skill_use_func["JuejiCard"]=function(card,use,self)
	local first_found, second_found = false, false
	self:sort(self.enemies,"defense")
	for _,enemy in ipairs(self.enemies) do
		if not (enemy:containsTrick("supply_shortage") or not enemy:faceUp() or enemy:hasSkill("tuxi") or enemy:hasSkill("shelie")) then
			if not first_found then 
				first_found = enemy 
			else 
				second_found = enemy 
				use.card=card
				if use.to then 
					use.to:append(first_found)
					use.to:append(second_found)
					return 
				end
			end
		end
	end
end

-- lexue

local jiangboyue_ai = SmartAI:newSubclass "jiangboyue"

function jiangboyue_ai:activate(use)
	if not self.player:hasUsed("LexueCard") then
		self:sort(self.friends_noself, "handcard")
		if #self.friends_noself>0 then
			local friend = self.friends_noself[#self.friends_noself]
			if use.to and not friend:isKongcheng() then 
				use.to:append(friend) 
				use.card = sgs.Card_Parse("@LexueCard=.")
				return 
			end
		end

		self:sort(self.enemies,"handcard")
		for _,enemy in ipairs(self.enemies) do
			if use.to and not enemy:isKongcheng() then
				use.to:append(enemy) 
				use.card = sgs.Card_Parse("@LexueCard=.")
				return 
			end
		end 
	else
		local lexue_card_id = self.player:getMark("lexue")
		if lexue_card_id ~= 0 then
	    	local lexue_card = sgs.Sanguosha:getCard(lexue_card_id)
		    local cards = self.player:getCards("h")	
		    cards=sgs.QList2Table(cards)
		    for _, card in ipairs(cards) do
		    	if card:getSuit() == lexue_card:getSuit() and self:getUseValue(lexue_card) >= self:getUseValue(card) then
				    local suit = card:getSuitString()
					local number = card:getNumberString()
				    local card_id = card:getEffectiveId()
				    local card_str = ("%s:lexue[%s:%s]=%d"):format(lexue_card:objectName(),suit, number, card_id)
					local use_card = sgs.Card_Parse(card_str)
					assert(use_card)
					local type = use_card:getTypeId()
					if type == sgs.Card_Basic then
						self:useBasicCard(use_card, use, self.slash_distance_limit)
					elseif type == sgs.Card_Trick then
						self:useTrickCard(use_card, use)
					elseif type == sgs.Card_Skill then
						self:useSkillCard(use_card, use)
					else
						self:useEquipCard(use_card, use)
					end
					if use:isValid() then
						self.toUse=nil
						return
					end
				end
			end
		end
	end
	super.activate(self, use)
end
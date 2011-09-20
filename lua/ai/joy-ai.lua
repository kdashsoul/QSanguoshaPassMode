-- when enemy using the peach
sgs.ai_skill_invoke["grab_peach"] = function(self, data)
	local struct= data:toCardUse()
	return self:isEnemy(struct.from)
end

local gale_shell_skill = {}
gale_shell_skill.name = "gale-shell"
table.insert(sgs.ai_skills, gale_shell_skill)
gale_shell_skill.getTurnUseCard = function(self)
	local cards = self.player:getHandcards()
	local fcard
	for _, card in sgs.qlist(cards) do
		if card:objectName() == "gale-shell" then fcard = card break end
	end
	if not fcard then return "." end
	
	return sgs.Card_Parse(fcard:getEffectiveId() .."->".. final:objectName())
end
	
sgs.ai_skill_use_func["GaleShell"] = function(card, use, self)
	local players = self.room:getOtherPlayers(self.player)
	local targets
	for _, target in sgs.qlist(players) do
		if self.player:distanceTo(target) <= 1 then 
			table.insert(targets, target)
		end
	end
	
	local final
	for _, select in ipairs(targets) do
		if self.player:isEnemy(select) then
			final = select
			break
		end
	end
	if not final then return "." end
	
	if use.to then
		use.to:append(final)
	end
	use.card = card
end
	
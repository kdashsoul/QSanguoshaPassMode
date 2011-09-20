-- xiaoyong
sgs.ai_skill_invoke.xiaoyong = function(self, data)
	local damage = data:toDamage()
	return not self:isFriend(damage.to)
end
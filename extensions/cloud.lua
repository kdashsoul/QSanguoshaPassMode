module("extensions.cloud", package.seeall)

extension = sgs.Package("cloud") 

-- zhangchunhua 
shangshi=sgs.CreateTriggerSkill{

	name="shangshi",
	events={sgs.Damaged,sgs.CardLost,sgs.HpLost},
	priority=-1,
	frequency=sgs.Skill_Frequent,
	on_trigger=function(self,event,player,data)
		local room=player:getRoom()
		if player:getLostHp()<=player:getHandcardNum() then return end
		--if player:getPhase()==sgs.Player_Discard then return end
		
		local move=data:toCardMove()
		if move and not move.from_place==sgs.Player_Hand then return end
		
		
		if not room:askForSkillInvoke(player,self:objectName()) then return end
		
		local x = player:getLostHp()-player:getHandcardNum()
		local log=sgs.LogMessage()
		log.type ="#shangshi"
		log.arg  =player:getGeneralName()
		log.arg2 =x
		room:sendLog(log)
		
		player:drawCards(x)
	end,
}

jueqing=sgs.CreateTriggerSkill{
	name="jueqing",
	events={sgs.Predamage},
	frequency=sgs.Skill_Compulsory,
	on_trigger=function(self,event,player,data)
		local room=player:getRoom()
		local damage=data:toDamage()
		
		local log=sgs.LogMessage()
		log.type ="#jueqing"
		log.arg  =player:getGeneralName()
		room:sendLog(log)
		
		room:loseHp(damage.to,damage.damage)
		return true
	end,
}

--yuejin
xiaoyong=sgs.CreateTriggerSkill{
	name="xiaoyong",
	events={sgs.Damage},
	on_trigger=function(self,event,player,data)
		local damage=data:toDamage()
		local room=player:getRoom()
		--if not damage.card then return end
		if damage.to:isDead() then return end
		if damage.to:getHandcardNum()<=damage.to:getHp() then return end
		if not room:askForSkillInvoke(player,self:objectName(),data) then return end
		room:askForDiscard(damage.to,self:objectName(),damage.to:getHandcardNum()-damage.to:getHp(),false,false)
	end
}

zhangchunhua=sgs.General(extension,"zhangchunhua","wei",3,false)
zhangchunhua:addSkill(shangshi)
zhangchunhua:addSkill(jueqing)

yuejin=sgs.General(extension,"yuejin","wei")
yuejin:addSkill(xiaoyong)

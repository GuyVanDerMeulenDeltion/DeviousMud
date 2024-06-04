#include "precomp.h"

#include "Core/Game/Entity/Definition/EntityDef.h"

#include "Core/Globals/S_Globals.h"

#include "Core/Game/Entity/EntityHandler.h"

#include "Core/Game/Combat/CombatHandler.h"

#include "Core/Network/NetworkHandler.h"

#include "Core/Network/Connection/ConnectionHandler.h"

#include "Core/Network/Client/ClientInfo.h"

#include "Core/Events/Query/EventQuery.h"

#include "Core/Config/Config.h"

#include "Shared/Network/Packets/PacketHandler.hpp"

#include <random>

void Entity::hide_entity(const bool _bShouldHide) const 
{
	m_bHideEntity = _bShouldHide;

	Packets::s_HideEntity packet;
	packet.interpreter = e_PacketInterpreter::PACKET_ENTITY_HIDE;
	packet.entityId    = uuid;
	packet.bShouldHide = _bShouldHide;

	PacketHandler::send_packet_multicast<Packets::s_HideEntity>
	(
		&packet,
		g_globals.networkHandler->get_server_host(),
		0,
		ENET_PACKET_FLAG_RELIABLE
	);
}

void Entity::teleport_to(Utilities::ivec2 _destination) 
{
	position = _destination;

	Packets::s_TeleportEntity packet;
	packet.interpreter = e_PacketInterpreter::PACKET_ENTITY_TELEPORT;
	packet.entityId = uuid;
	packet.x = _destination.x;
	packet.y = _destination.y;

	PacketHandler::send_packet_multicast<Packets::s_TeleportEntity>
	(
		&packet,
		g_globals.networkHandler->get_server_host(),
		0,
		ENET_PACKET_FLAG_RELIABLE
	);
}

void Entity::die()
{
	if (!m_bIsDead)
	{
		disengage();

		m_bIsDead = true;

		Packets::s_ActionPacket packet;
		packet.interpreter = e_PacketInterpreter::PACKET_ENTITY_DEATH;
		packet.entityId = uuid;
		PacketHandler::send_packet_multicast<Packets::s_ActionPacket>
		(
			&packet, g_globals.networkHandler->get_server_host(),
			0,
			0
		);
	}
}

void Entity::update()
{
	if(tickCounter > 0) 
	{
		tickCounter--;
	}

	if(m_bIsDead) 
	{
		m_respawnTimerElapsed++;

		//*------------------------------------------------------
		// Hide the NPC when it's past the death transition time.
		//*

		if (!m_bHideEntity)
		{
			if (m_respawnTimerElapsed > m_deathTransitionTime)
			{
				hide_entity(true);
			}
		}

		if(m_respawnTimerElapsed >= get_respawn_timer()) 
		{
			respawn();
		}

		return;
	}

	if(skills[DM::SKILLS::e_skills::HITPOINTS].levelboosted <= 0) 
	{
		die();
		return;
	}

	tick();
}

void Entity::tick()
{
	/// Override in child classes.
	///
	/// 
}

void Entity::disengage()
{

}

void Entity::try_set_target(std::shared_ptr<Entity> _entity, bool _bWasInstigated)
{

}

int Entity::get_attack_range()
{
	return 0;
}

int Entity::get_attack_speed()
{
	return 3;
}

int32_t Entity::get_respawn_timer() 
{
	return 6;
}

void Entity::respawn()
{
	//*
	// Reset death related params.
	//*
	{
		m_bIsDead = false;
		m_respawnTimerElapsed = 0.0f;
	}

	//*
	// Resets all stats and deathstate back to normal.
	//*
	for (uint8_t skill = 0; skill < DM::SKILLS::SKILL_COUNT; skill++)
	{
		DM::SKILLS::e_skills type = static_cast<DM::SKILLS::e_skills>(skill);
		skills[type].levelboosted = skills[type].level;
	}

	//*
	// Bring the entity back to its starter position.
	//*
	{
		teleport_to(respawnLocation);
	}

	//*
	// For now only broadcast hitpoints skill since we don't use any other skill visually.
	//*
	{
		broadcast_skill(DM::SKILLS::e_skills::HITPOINTS);
	}

	//*
	// Signal respawn to the client.
	//*
	{
		Packets::s_ActionPacket packet;
		packet.interpreter = e_PacketInterpreter::PACKET_ENTITY_RESPAWN;
		packet.entityId = uuid;
		PacketHandler::send_packet_multicast<Packets::s_ActionPacket>
		(
			&packet,
			g_globals.networkHandler->get_server_host(),
			0,
			ENET_PACKET_FLAG_RELIABLE
		);
	}

	//*-------------------------
	// Make entity visible again
	//*
	hide_entity(false);
}

void Entity::broadcast_hit(std::shared_ptr<Entity> _by, int32_t _hitAmount) const
{
	Packets::s_EntityHit packet;
	packet.interpreter  = e_PacketInterpreter::PACKET_ENTITY_HIT;
	packet.action       = e_Action::SOFT_ACTION;
	packet.fromEntityId = _by->uuid;
	packet.toEntityId   = uuid;
	packet.hitAmount    = _hitAmount;

	PacketHandler::send_packet_multicast<Packets::s_EntityHit>
	(
		&packet,
		g_globals.networkHandler->get_server_host(),
		0,
		ENET_PACKET_FLAG_RELIABLE
	);
}

void Entity::broadcast_skill(DM::SKILLS::e_skills _skillType) 
{
	const DM::SKILLS::Skill& skill = skills[_skillType];

	Packets::s_UpdateSkill packet;
	packet.interpreter  = e_PacketInterpreter::PACKET_ENTITY_SKILL_UPDATE;
	packet.action       = e_Action::SOFT_ACTION;
	packet.entityId     = uuid;
	packet.skillType    = static_cast<uint8_t>(_skillType);
	packet.level        = skill.level;
	packet.levelBoosted = skill.levelboosted;

	PacketHandler::send_packet_multicast<Packets::s_UpdateSkill>
	(
		&packet,
		g_globals.networkHandler->get_server_host(),
		0,
		ENET_PACKET_FLAG_RELIABLE
	);
}

const bool Entity::is_dead() const
{
	return m_bIsDead;
}

/// <summary>
/// TODO: Replace these with statemachines maybe?
/// </summary>
void NPC::tick()
{
	//*----------------------------------------------------------
	// Try engaging with target if available.
	// If engaging and outside of max wandering range, disengage.
	//*
	if (auto target = m_target.lock(); !m_target.expired())
	{
		int32_t distance = Utilities::ivec2::get_distance(respawnLocation, position);

		if (distance > static_cast<int32_t>(maxWanderingDistance))
		{
			m_target.reset();
		}
		else
		{
			auto optNPC = g_globals.entityHandler->get_entity(uuid);

			CombatHandler::engage
			(
				std::dynamic_pointer_cast<NPC>(optNPC.value()),
				target
			);
			return;
		}
	}

	//*----------------------------------------------------------------------------
	// Movement behaviour of NPC's, they roll a dice 1-9 every game tick.
	// If the roll was succesfull, the npc will choose a random location within its
	// specified wandering range.
	//*
	{
		if (!m_bIsMoving)
		{
			const int chanceToMove = 9;
			const int dice = (rand() % chanceToMove) + 1;

			if (dice == chanceToMove)
			{
				Utilities::ivec2 offset = Utilities::ivec2
				(
					(rand() % wanderingDistance) - (wanderingDistance / 2),
					(rand() % wanderingDistance) - (wanderingDistance / 2)
				);

				m_bIsMoving = true;

				m_targetPos = respawnLocation + offset;
			}
		}

		if (m_bIsMoving)
		{
			g_globals.entityHandler->move_entity_to(uuid, m_targetPos);

			if (m_targetPos == position)
			{
				m_bIsMoving = false;
			}
		}
	}
}

int32_t NPC::get_respawn_timer()
{
	return m_respawnTimer;
}

void NPC::try_set_target(std::shared_ptr<Entity> _entity, bool _bWasInstigated)
{
	if(m_target.expired()) 
	{
		//*
		// If it was instigated by another entity, give this NPC a full attacking delay.
		//*
		if (_bWasInstigated)
		{
			tickCounter = get_attack_speed();
		}

		m_target = _entity;
	}
}

int NPC::get_attack_range() 
{
	return attackRange;
}

void NPC::disengage() 
{
	m_target.reset();
}

void Player::die()
{
	//*------------------------
	//Call base death behaviour
	//*
	Entity::die();

	std::optional<uint64_t> optClientHandle = g_globals.entityHandler->transpose_player_to_client_handle(uuid);

	if (uint64_t clientHandle = optClientHandle.value(); optClientHandle.has_value())
	{
		enet_uint32 clientHandle32 = static_cast<enet_uint32>(clientHandle);

		RefClientInfo clientInfo = g_globals.connectionHandler->get_client_info(clientHandle32);

		//*------------------------------------------------------------
		// Queue packet that prevents the player from making any input.
		// It starts off as a soft action as to allow the other remaining packets to get executed.
		// The packet will get repeated as a hard action.
		//*
		Packets::s_PacketHeader packet;
		packet.action = e_Action::HARD_ACTION;
		packet.interpreter = e_PacketInterpreter::PACKET_ENTITY_DEATH;

		clientInfo->packetquery->queue_packet
		(
			std::move(std::make_unique<Packets::s_PacketHeader>(packet))
		);
	}
}

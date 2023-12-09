#include "SmartAI.h"
#include "ScriptedCreature.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Define.h"
#include "SharedDefines.h"
#include "Unit.h"
#include "ScriptedGossip.h"
#include "TaskScheduler.h"
#include "PacmanMap.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "ObjectMgr.h"
#include "Position.h"
#include "PacmanMap.h"
#include "GridNotifiers.h"
#include "EventMap.h"

class npc_pacman_handler : public CreatureScript
{
public:
	npc_pacman_handler() : CreatureScript("npc_pacman_handler") { }

    struct npc_pacman_handlerAI : public ScriptedAI {

        npc_pacman_handlerAI(Creature* creature) : ScriptedAI(creature) , _eventControler()
        {
        }

        void InitializeAI() override
        {
            AreaTrigger const* at = sObjectMgr->GetAreaTriggerEx(PacMan::AT_PLAYER_START);
            if (at != nullptr) {
                me->GetMotionMaster()->MovePoint(PacMan::AT_PLAYER_START, { at->target_X, at->target_Y, at->target_Z }, true, at->target_Orientation);
            }
            else {
                me->GetMotionMaster()->MovePoint(PacMan::AT_PLAYER_START, me->GetHomePosition());
            }
            _default_position = me->GetPosition();
            if (_playerGuid != ObjectGuid::Empty) {
                Player* player = ObjectAccessor::FindPlayer(_playerGuid);
                if (player) {
                    /* just in case, i cannot modify DBC so i use infinity visual auras on player.
                       This may require GM intervention in case of disconnection, for example but will handle the teleport/summon case */
                    player->RemoveAurasDueToSpell(PacMan::SPELL_STUN_VISUAL);
                    player->RemoveAurasDueToSpell(PacMan::SPELL_EVENT_AURA_DAMAGE);
                    player->RemoveAurasDueToSpell(PacMan::SPELL_EVENT_AURA);
                    player->RemoveAurasDueToSpell(PacMan::SPELL_SPHERE_VISUAL);
                }
            }
            _playerSuperpower = 0;
            _playerKillSpree = 0;
            _playerScore = 0;
            _playerGuid = ObjectGuid::Empty;
            _events.Reset();
            _eventControler.reset();
        }

        void Reset() override
        {
            InitializeAI();
        }

        void OnDespawn() override
        {
            Reset();
        }

        void ReceiveEmote(Player* player, uint32 emote) override
        {
        }

        bool OnGossipHello(Player* player) override
        {
            if (_eventControler.isRuning()) {
                CloseGossipMenuFor(player);
                if (player->GetGUID() == _playerGuid) {
                    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Stop event", 0, PacMan::PC_GOSSIP_STOP_EVENT);
                }
                else {
                    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Set 4 minutes (by default) deadline for current player", 0, PacMan::PC_GOSSIP_SET_DEADLINE);
                }
                SendGossipMenuFor(player, player->GetGossipTextId(me) + 1, me); // creature busy message not the best solution to increment the current id
            }
            else {
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Let's start!", 0, 0);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Event demonstration. Ghosts will stay on them positions", 0, PacMan::PC_GOSSIP_DEBUG_IDLE_GHOSTS);

                auto const& score = _leaderBoardCache[player->GetGUID()];
                if (score.first != 0) {
                    AddGossipItemFor(player, GOSSIP_ICON_BATTLE, PacMan::sprintf("Your previous try scores: %u.", score.first), 0, PacMan::PC_GOSSIP_DUMMY);
                }
                if (score.second != 0) {
                    AddGossipItemFor(player, GOSSIP_ICON_BATTLE, PacMan::sprintf("Your previous record: %u.", score.second), 0, PacMan::PC_GOSSIP_DUMMY);
                }
                SendGossipMenuFor(player, player->GetGossipTextId(me), me);
            }
            return true;
        }

        bool OnGossipSelect(Player* player, uint32 sender, uint32 gossipListId) override
        {
            if (!player) {
                return false;
            }

            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);

            if (action == 0 || action == PacMan::PC_GOSSIP_DEBUG_IDLE_GHOSTS) {
                ClearGossipMenuFor(player);
                me->Say("Let's start!", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                _playerGuid = player->GetGUID();

                try {
                    /*There is space here for handling errors in calculations, which is omitted because it is beyond the scope of the task.*/
                    _eventControler.prepareEvent(*me);
/*                    _eventControler.calculateArea(PacMan::POS_MAP);*/
                    _eventControler.summonGameObjects();
                    _eventControler.summonCreatures();

                    _eventControler.run(player);
                    if (action == PacMan::PC_GOSSIP_DEBUG_IDLE_GHOSTS) {
                        _eventControler.pingGhosts(PacMan::STOP_GHOSTS);
                    }
                    _events.ScheduleEvent(PacMan::CHECK_PLAYER_POSITION, 500ms);
                }
                catch (std::exception& /*e*/) {
                     me->Say("Not today, Honey", LANG_UNIVERSAL);
                     Reset();
                     return true;
                }

                AreaTrigger const* at = sObjectMgr->GetAreaTriggerEx(PacMan::AT_NPC_VANTAGE);
                if (at) {
                    me->NearTeleportTo({ at->target_X, at->target_Y, at->target_Z, at->target_Orientation }, true);
                    me->CastSpell(nullptr, PacMan::SPELL_NPC_TELEPORT_VISUAL);
                    _events.ScheduleEvent(PacMan::SHOW_TELEPORT_VISUAL, 1ms);
                    _events.ScheduleEvent(PacMan::TELEPORT_PLAYER_AT_START, 20ms);
                }
                return true;
            }
            else if (action == PacMan::PC_GOSSIP_STOP_EVENT) {
                ClearGossipMenuFor(player);
                me->Say("See you later!", LANG_UNIVERSAL);
                CloseGossipMenuFor(player);
                Reset();
                return true;
            }
            else if (action == PacMan::PC_GOSSIP_SET_DEADLINE) {
                ClearGossipMenuFor(player);
                if (_eventControler.isRuning()) {
                    me->Yell(PacMan::sprintf("hurry up %s, you only have 4 minutes", player->GetName().c_str()), LANG_UNIVERSAL);
                    _events.ScheduleEvent(PacMan::STOP_FAIL, PacMan::EVENT_DEADLINE);
                }
                return true;
            }
            return false;
        }
        void UpdateAI(uint32 diff) override
        {
//             _checkPlayers += diff;
//             if (_checkPlayers > 10000 && _eventControler.isRuning()) {
//                 std::list<Player*> players;
//                 Trinity::AnyPlayerInObjectRangeCheck check(me, 120, false);  // check alive and dead players
//                 Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, check);
//                 Cell::VisitWorldObjects(me, searcher, 120);
//                 if (players.empty()) {
//                     Reset();
//                 }
//                 _checkPlayers = 0;                
//             }
            _events.Update(diff);

            switch (_events.ExecuteEvent())
            {
                case PacMan::CHECK_PLAYER_POSITION: {
                    _eventControler.updatePlayerStatus(_events, _playerGuid);
                    _events.RescheduleEvent(PacMan::CHECK_PLAYER_POSITION, 200ms);
                    break;
                }
                case PacMan::REMOVE_PLAYER_STUN: {
                   Player* player = me->GetMap()->GetPlayer(_playerGuid);
                   if (!player) {
                       return;
                   }

                   player->RemoveAurasDueToSpell(PacMan::SPELL_STUN_VISUAL);
                   _events.ScheduleEvent(PacMan::PLAYER_STARTED, 1ms);
                   break;
                }
                case PacMan::SHOW_TELEPORT_VISUAL: {
                    Player* player = me->GetMap()->GetPlayer(_playerGuid);
                    if (!player) {
                        return;
                    }

                    player->CastSpell(nullptr, PacMan::SPELL_PLAYER_TELEPORT_VISUAL);
                    break;
                }
                case PacMan::TELEPORT_PLAYER_AT_START: {
                    Player* player = me->GetMap()->GetPlayer(_playerGuid);
                    if (!player) {
                        return;
                    }

                    _eventControler.movePlayerToStartPosition(player, _events);
                    break;
                }
                case PacMan::RENEW_EVENT_AURA_DAMAGE: {
                    Player* player = me->GetMap()->GetPlayer(_playerGuid);
                    if (!player) {
                        return;
                    }

                    player->CastSpell(player, PacMan::SPELL_EVENT_AURA_DAMAGE);
                    _events.RescheduleEvent(PacMan::RENEW_EVENT_AURA_DAMAGE, PacMan::EVENT_AURA_STACK_SPEED);
                    break;
                }
                case PacMan::RENEW_EVENT_AURA: {
                    Player* player = me->GetMap()->GetPlayer(_playerGuid);
                    if (!player) {
                        return;
                    }

                    player->RemoveAurasDueToSpell(PacMan::SPELL_EVENT_AURA);
                    player->CastSpell(player, PacMan::SPELL_EVENT_AURA);
                    _events.RescheduleEvent(PacMan::RENEW_EVENT_AURA, 8s);
                    break;
                }
                case PacMan::PLAYER_STARTED: {
                    Player* player = me->GetMap()->GetPlayer(_playerGuid);
                    if (!player) {
                        return;
                    }

                    player->CastSpell(player, PacMan::SPELL_EVENT_AURA);
                    /* and first stack */
                    player->CastSpell(player, PacMan::SPELL_EVENT_AURA_DAMAGE);
                    _events.ScheduleEvent(PacMan::RENEW_EVENT_AURA_DAMAGE, PacMan::EVENT_AURA_STACK_SPEED);
                    _events.ScheduleEvent(PacMan::RENEW_EVENT_AURA, 8s);
                    break;
                }
                case PacMan::REMOVE_SPHERE_VISUAL: {
                    Player* player = me->GetMap()->GetPlayer(_playerGuid);
                    if (!player) {
                        return;
                    }

                    _playerSuperpower = 0;
                    _playerKillSpree = 0;
                    player->RemoveAurasDueToSpell(PacMan::SPELL_SPHERE_VISUAL);
                    break;
                }
                case PacMan::STOP_FAIL: {
                    Player* player = me->GetMap()->GetPlayer(_playerGuid);
                    if (!player) {
                        Reset();
                        return;
                    }

                    std::pair<uint32, uint32>& scores = _leaderBoardCache[_playerGuid];
                    scores.first = _playerScore;
                    /* i think we should not save scores in fail event case */
                    /*scores.second = std::max(scores.second, _playerScore);*/
                    me->Yell(PacMan::sprintf("You only scored %d points, %s. Not impressive.", _playerScore, player->GetName().c_str()), LANG_UNIVERSAL);
                    Reset();
                    break;
                }
                case PacMan::STOP_SUCCESS: {
                    Player* player = me->GetMap()->GetPlayer(_playerGuid);
                    if (!player) {
                        Reset();
                        return;
                    }

                    std::pair<uint32, uint32>& scores = _leaderBoardCache[_playerGuid];
                    scores.first = _playerScore;
                    scores.second = std::max(scores.second, _playerScore);
                    me->Yell(PacMan::sprintf("Congratulations on winning! Your score of %d points, %s, is absolutely impressive!", _playerScore, player->GetName().c_str()), LANG_UNIVERSAL);
                    Reset();
                    break;
                }
                default:
                    break;
            }
        }


        void DoAction(int32 action) override
        {
            switch (action)
            {
                case PacMan::REGULAR_POINT_DESPAWN: {
                    if (_eventControler.isRuning()) {
                        Player* player = me->GetMap()->GetPlayer(_playerGuid);
                        if (!player) {
                            return;
                        }

                        player->RemoveAuraFromStack(PacMan::SPELL_EVENT_AURA_DAMAGE);
                        player->RegenerateHealth();
                        player->CastSpell(player, PacMan::SPELL_EVENT_AURA_DECREASE_SPEED);

                        if (_eventControler.isEnd()) {
                            /* reuse action for stopping the ghosts */
                            _eventControler.pingGhosts(PacMan::BONUS_POINT_DESPAWN_HA);
                            _events.Reset();
                            _events.ScheduleEvent(PacMan::STOP_SUCCESS, 1s);
                        }
                    }
                    break;
                }
                case PacMan::BLINKY_WIN:
                case PacMan::INKY_WIN:
                case PacMan::CLYDE_WIN: {
                    _eventControler.pingGhosts(static_cast<PacMan::EPacmManHandlerAction>(action));

                    Player* player = me->GetMap()->GetPlayer(_playerGuid);
                    if (!player) {
                        return;
                    }

                    int32_t lifes = _eventControler.lifeCheck(player, _events);
                    if (lifes == 0) {
                        _events.ScheduleEvent(PacMan::STOP_FAIL, 1ms);
                    }
                    break;
                }
                case PacMan::BLINKY_LOSE: 
                case PacMan::INKY_LOSE: 
                case PacMan::CLYDE_LOSE: {
                    _eventControler.pingGhosts(static_cast<PacMan::EPacmManHandlerAction>(action));
                    Player* player = me->GetMap()->GetPlayer(_playerGuid);
                    if (!player) {
                        return;
                    }

                    uint64 now = static_cast<uint64>(time(nullptr));
                    uint32 score = 0;
                    // just ignore C4018
                    if (now - _playerSuperpower <= std::chrono::duration_cast<std::chrono::seconds>(PacMan::GHOST_AI_FRIGHTENED_TIME).count()) {
                        ++_playerKillSpree;
                        score = PacMan::SCORE_GHOST_EATEN * _playerKillSpree;
                        _playerScore += score;
                    }
                    else {
                        /* possible delay */
                        score = PacMan::SCORE_GHOST_EATEN * ((_playerKillSpree == 0) ? 1 : 0);
                        _playerScore += score;
                    }
                    me->Yell(PacMan::sprintf("Nice one! %s got %u point!", player->GetName(), score), LANG_UNIVERSAL);
                    break;
                }
                case PacMan::BONUS_POINT_DESPAWN: {
                    Player* player = me->GetMap()->GetPlayer(_playerGuid);
                    if (!player) {
                        return;
                    }

                    player->CastSpell(nullptr, PacMan::SPELL_SPHERE_VISUAL);
                    _playerSuperpower = static_cast<uint64>(time(nullptr));
                    _events.ScheduleEvent(PacMan::REMOVE_SPHERE_VISUAL, PacMan::GHOST_AI_FRIGHTENED_TIME);
                    _eventControler.pingGhosts(static_cast<PacMan::EPacmManHandlerAction>(action));
                    if (_eventControler.isEnd()) {
                        _events.Reset();
                        _events.ScheduleEvent(PacMan::STOP_SUCCESS, 1s);
                    }
                }
                default:
                    break;
            }
        }

        ObjectGuid GetGUID(int32 guidIndex) const override
        {
            if (guidIndex == PacMan::GET_CURRENT_PLAYER) {
                return _playerGuid;
            }
            else if (guidIndex == PacMan::GET_BLINKY_POSITION) {
                /* little trick */
                ObjectGuid result(_eventControler.getGhostPosition(PacMan::BLINKY));
                return result;
            }
            return ObjectGuid::Empty;
        }


        void SetGUID(ObjectGuid const& guid, int32 action) override
        {
            if (action == PacMan::REGULAR_POINT_DESPAWN) {
                _eventControler.removeCreature(guid);
                if (_eventControler.spawnFruitIfNeeded()) {
                    me->Yell("Yammy!", LANG_UNIVERSAL);
                }
                _playerScore += PacMan::SCORE_REGULAR_POINT;
                DoAction(action);
            }
            else if (action == PacMan::BONUS_POINT_DESPAWN) {
                _eventControler.removeCreature(guid);
                if (_eventControler.spawnFruitIfNeeded()) {
                    me->Yell("Yammy!", LANG_UNIVERSAL);
                }
                _playerScore += PacMan::SCORE_POWER_POINT;
                DoAction(action);
            }
            else if (action == PacMan::MOVE_FRUIT) {
                _eventControler.handleFruit(guid);
            }
//             switch (action)
//             {
//                 case PacMan::REGULAR_POINT_DESPAWN: {
//                     DoAction(action);
//                     _eventControler.removeCreature(guid);
//                     break;
//                 }
//             default:
//                 break;
//             }
        }

    private:
        Position _default_position;
/*        uint32 _checkPlayers = 0;*/
        EventMap _events;
        ObjectGuid _playerGuid;

        /* registr killspree */
        uint64 _playerSuperpower = 0;
        uint32 _playerKillSpree = 0;

        uint32 _playerScore = 0;

        std::unordered_map<ObjectGuid, std::pair<uint32, uint32>> _leaderBoardCache;
        PacMan::EventController _eventControler;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_pacman_handlerAI(creature);
    }

};

void AddSC_npc_pacman_handler()
{
    new npc_pacman_handler();
}

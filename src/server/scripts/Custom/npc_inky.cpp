#include "SmartAI.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "PacmanMap.h"
#include "SpellMgr.h"
#include "GhostAI.h"
class npc_inky : public CreatureScript
{
public:
    npc_inky() : CreatureScript("npc_inky") { }

    struct npc_inkyAI : public PacMan::GhostAI {

        npc_inkyAI(Creature* creature) : PacMan::GhostAI(creature) {
        }

        void InitializeAI() override {
            PacMan::GhostAI::InitializeAI();

            /* Define scatter positions */
            _scatterPositions.push(PacMan::INKY_SCATTER_POS1);
            _scatterPositions.push(PacMan::INKY_SCATTER_POS2);
            _scatterPositions.push(PacMan::INKY_SCATTER_POS3);

            if (PacMan::GHOSTS_START_BY_TIMER) {
                _events.ScheduleEvent(PacMan::GHOST_DELAYED_START, PacMan::INKY_AI_DELAYED_START);
            }
            /* else trigger by eated points */
        }

        void DoAction(int32 action) override {
            if (action == PacMan::GHOST_FIND_TARGET_POINT) {
                switch (_state) {
                    case PacMan::EAIState::AI_STATE_CHASE: {
                        Player* player = ObjectAccessor::GetPlayer(me->GetMap(), _playerGuid);
                        if (!player) {
                            return;
                        }

                        Position playerPosition = player->GetPosition();
                        Position myPosition = me->GetPosition();

                        std::pair<int32_t, int32_t> myPseudoPosition = _mapManager->toPseudoCellTransform(myPosition);
                        std::pair<int32_t, int32_t> playerPseudoPosition = _mapManager->toPseudoCellTransform(playerPosition);

                        if (myPseudoPosition == playerPseudoPosition) {
                            return PacMan::GhostAI::PlayerCatched();
                        }

                        if (_forceChangePath || _pathFinder->isAdjacent(myPseudoPosition, _currentTarget) || !me->isMoving()) {
                            _currentTarget =  _mapManager->getNearestAvailablePoint(CalculateTarget(playerPseudoPosition));
                            std::vector<Position> path = _mapManager->toWaypointPathTransform(_pathFinder->findPath(myPseudoPosition, _currentTarget, 100));

                            me->GetMotionMaster()->MoveSmoothPath(0, path.data(), path.size(), false);

                            /* The path calculation function may require significant resources but AI must be effective */
                            _events.ScheduleEvent(PacMan::GHOST_CHASE_FORCE_RECALCULATE_PATH, 1500ms);
                        }
                        break;
                    }
                    default:
                        return PacMan::GhostAI::DoAction(action);
                }

                _forceChangePath = false;
                _events.RescheduleEvent(PacMan::GHOST_FIND_TARGET_POINT, 100ms);
            }
            else if (action == PacMan::GHOST_WIN) {
                Unit* owner = me->GetOwner();
                if (owner) {
                    owner->GetAI()->DoAction(PacMan::INKY_WIN);
                }
            }
            else if (action == PacMan::GHOST_LOSE) {
                Unit* owner = me->GetOwner();
                if (owner) {
                    owner->GetAI()->DoAction(PacMan::INKY_LOSE);
                }
            }
            else {
                return PacMan::GhostAI::DoAction(action);
            }
        }

//         void UpdateAI(uint32 diff) override
//         {
//             _events.Update(diff);
// 
//             switch (_events.ExecuteEvent()) {
//                 case PacMan::GHOST_DELAYED_START: {
//                     Unit* owner = me->GetOwner();
//                     if (owner) {
//                         _playerGuid = owner->GetAI()->GetGUID(PacMan::GET_CURRENT_PLAYER);
//                     }
//                     else {
//                         _playerGuid = ObjectGuid::Empty;
//                     }
//                     me->RemoveAurasDueToSpell(PacMan::SPELL_STUN_VISUAL);
//                     _state = PacMan::EAIState::AI_STATE_SCATTER;
//                     Position myPosition = me->GetPosition();
//                     _currentTarget = _mapManager->toPseudoCellTransform(myPosition);
//                     _events.ScheduleEvent(PacMan::GHOST_FIND_TARGET_POINT, 1ms);
//                     _events.ScheduleEvent(PacMan::GHOST_CHANGE_STATE, PacMan::GHOST_AI_SCATTER_TIME);
//                     break;
//                 }
//                 case PacMan::GHOST_CHANGE_STATE: {
//                     switch (_state) {
//                         case PacMan::EAIState::AI_STATE_CHASE: {
//                             /*DEBUG*/
//                             _state = PacMan::EAIState::AI_STATE_FRIGHTEND;
//                             me->CastSpell(nullptr, PacMan::SPELL_FEAR_VISUAL);
//                             me->CastSpell(nullptr, PacMan::SPELL_NPC_CHANGE_STATE_VISUAL);
//                             _events.ScheduleEvent(PacMan::GHOST_CHANGE_STATE, PacMan::GHOST_AI_FRIGHTENED_TIME);
// 
//                             Player* player = ObjectAccessor::GetPlayer(me->GetMap(), _playerGuid);
//                             if (player) {
//                                 srand(static_cast<unsigned int>(time(nullptr) / 2 + me->GetGUID().GetRawValue()));
//                                 int32_t random = rand() % 10;
//                                 me->Yell(PacMan::sprintf(PacMan::GHOST_SAY_WHEN_WAS_FRIGHTENED[random], player->GetName().c_str()), LANG_UNIVERSAL);
//                             }
//                             /*DEBUG*/
//                             break;
//                         }
//                         case PacMan::EAIState::AI_STATE_SCATTER: {
//                             _state = PacMan::EAIState::AI_STATE_CHASE;
//                             me->CastSpell(nullptr, PacMan::SPELL_NPC_CHANGE_STATE_VISUAL);
//                             _events.ScheduleEvent(PacMan::GHOST_CHANGE_STATE, PacMan::GHOST_AI_CHASE_TIME);
//                             break;
//                         }
//                         case PacMan::EAIState::AI_STATE_FRIGHTEND: {
//                             _state = PacMan::EAIState::AI_STATE_SCATTER;
//                             me->RemoveAurasDueToSpell(PacMan::SPELL_FEAR_VISUAL);
//                             me->CastSpell(nullptr, PacMan::SPELL_NPC_CHANGE_STATE_VISUAL);
//                             _events.ScheduleEvent(PacMan::GHOST_CHANGE_STATE, PacMan::GHOST_AI_SCATTER_TIME);
//                             break;
//                         }
//                         default:
//                             break;
//                     }
//                     _forceChangePath = true;
//                     break;
//                 }
//                 case PacMan::GHOST_FIND_TARGET_POINT: {
//                     DoAction(PacMan::GHOST_FIND_TARGET_POINT);
//                     break;
//                 }
//                 case PacMan::GHOST_CHASE_FORCE_RECALCULATE_PATH: {
//                     _forceChangePath = true;
//                     break;
//                 }
//                 case PacMan::GHOST_BACK_TO_START: {
//                     me->NearTeleportTo(_startPosition);
//                 }
//                 case PacMan::GHOST_RESPAWN: {
//                     me->Respawn(true);
//                     _state = PacMan::EAIState::AI_STATE_SCATTER;
//                     Position _startPosition = me->GetPosition();
//                     _currentTarget = _mapManager->toPseudoCellTransform(_startPosition);
//                     _events.ScheduleEvent(PacMan::GHOST_FIND_TARGET_POINT, 1ms);
//                     _events.ScheduleEvent(PacMan::GHOST_CHANGE_STATE, PacMan::GHOST_AI_SCATTER_TIME);
//                 }
//                 default:
//                     break;
//             }
//         }
    private:

        std::pair<int32_t, int32_t> CalculateTarget(const std::pair<int32_t, int32_t>& playerPosition) {
            Unit* owner = me->GetOwner();
            if (!owner) {
                return playerPosition;
            }

            ObjectGuid blinkyPosition = owner->GetAI()->GetGUID(PacMan::GET_BLINKY_POSITION);
            uint64 coordinates = blinkyPosition.GetRawValue();

            /* unpack */
            uint16_t num = 0x01;
            uint8_t* ptr = reinterpret_cast<uint8_t*>(&num);
            bool isBigEndian = ptr[0] == 0x00;

            std::pair<int32_t, int32_t> blinkyPseudoPosition;
            if (isBigEndian) {
                coordinates = (coordinates >> 32) | (coordinates << 32);
            }
            blinkyPseudoPosition.first = static_cast<uint32_t>(coordinates >> 32);
            blinkyPseudoPosition.second = static_cast<uint32_t>(coordinates);
            /* end unpack */

            /* vector from blinky to player */
            int32_t vectorJ = playerPosition.second - blinkyPseudoPosition.second;
            int32_t vectorI = playerPosition.first - blinkyPseudoPosition.first;

            int32_t targetI = playerPosition.first + vectorI;
            int32_t targetJ = playerPosition.second + vectorJ;

            if (targetI < 0) targetI = 0;
            if (targetI >= _mapManager->getMapInfo().height) targetI = _mapManager->getMapInfo().height - 1;
            if (targetJ < 0) targetJ = 0;
            if (targetJ >= _mapManager->getMapInfo().width) targetJ = _mapManager->getMapInfo().width - 1;

            return { targetI, targetJ };
        }

//     private:
//         std::unique_ptr<PacMan::MapManager> _mapManager;
//         std::unique_ptr <PacMan::PathFinder> _pathFinder;
// 
//         PacMan::EAIState _state = PacMan::EAIState::AI_STATE_SCATTER;
//         Position _startPosition;
//         std::pair<int32_t, int32_t> _currentTarget;
//         std::queue<std::pair<int32_t, int32_t>> _scatterPositions;
// 
//         bool _forceChangePath = false;
// 
//         ObjectGuid _playerGuid;
// 
//         EventMap _events;
    };
    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_inkyAI(creature);
    }
};

void AddSC_npc_inky()
{
    new npc_inky();
}

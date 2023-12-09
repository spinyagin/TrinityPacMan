#include "SmartAI.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "PacmanMap.h"
#include "SpellMgr.h"
#include "GhostAI.h"
class npc_blinky : public CreatureScript
{
public:
    npc_blinky() : CreatureScript("npc_blinky") { }

    struct npc_blinkyAI : public PacMan::GhostAI {

        npc_blinkyAI(Creature* creature) : PacMan::GhostAI(creature) {
        }

        void InitializeAI() override {
            PacMan::GhostAI::InitializeAI();

            /* Define scatter positions */
            _scatterPositions.push(PacMan::BLINKY_SCATTER_POS1);
            _scatterPositions.push(PacMan::BLINKY_SCATTER_POS2);
            _scatterPositions.push(PacMan::BLINKY_SCATTER_POS3);

            _events.ScheduleEvent(PacMan::GHOST_DELAYED_START, PacMan::BLINKY_AI_DELAYED_START);
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
                            _currentTarget = playerPseudoPosition;
                            std::vector<Position> path = _mapManager->toWaypointPathTransform(_pathFinder->findPath(myPseudoPosition, _currentTarget, 100));

                            me->GetMotionMaster()->MoveSmoothPath(0, path.data(), path.size(), false);

                            /* The path calculation function may require significant resources but AI must be effective */
                            _events.ScheduleEvent(PacMan::GHOST_CHASE_FORCE_RECALCULATE_PATH, 1500ms);
                        }
                        break;
                    }
//                     case PacMan::EAIState::AI_STATE_SCATTER: {
//                         Position position = me->GetPosition();
//                         std::pair<int32_t, int32_t> pseudoPosition = _mapManager->toPseudoCellTransform(position);
// 
//                         if (myPseudoPosition == playerPseudoPosition) {
//                             Unit* owner = me->GetOwner();
//                             if (owner) {
//                                 owner->GetAI()->DoAction(PacMan::BLINKY_WIN);
//                             }
// 
//                             /* TODO: add some visual effects */
// 
// //                             _events.Reset();
// //                             return;
//                         }
// 
//                         if (_pathFinder->isAdjacent(pseudoPosition, _currentTarget) || !me->isMoving() || _forceChangePath) {
//                             std::pair<int32_t, int32_t> _currentTarget = _scatterPositions.front();
//                             _scatterPositions.pop();
//                             _scatterPositions.push(_currentTarget);
// 
//                             std::vector<Position> path = _mapManager->toWaypointPathTransform(_pathFinder->findPath(pseudoPosition, _currentTarget, 100));
//                             me->GetMotionMaster()->MoveSmoothPath(0, path.data(), path.size(), false);
//                         }
//                         break;
//                     }
//                     case PacMan::EAIState::AI_STATE_FRIGHTEND: {
//                         if (myPseudoPosition == playerPseudoPosition) {
//                             Unit* owner = me->GetOwner();
//                             if (owner) {
//                                 owner->GetAI()->DoAction(PacMan::BLINKY_LOSE);
//                             }
// 
//                             srand(static_cast<unsigned int>(time(nullptr) / 2 + me->GetGUID().GetRawValue()));
//                             int32_t random = rand() % 10;
//                             me->Say(PacMan::sprintf(PacMan::GHOST_SAYS_WHEN_WAS_EATEN[random], player->GetName().c_str()), LANG_UNIVERSAL);
// 
//                             me->KillSelf();
//                             _events.Reset();
//                             _events.ScheduleEvent(PacMan::GHOST_BACK_TO_START, PacMan::GHOST_CORPSE_TELEPORT_TIME);
//                             _events.ScheduleEvent(PacMan::GHOST_RESPAWN, PacMan::GHOST_RESPAWN_TIME);
// 
//                             /* TODO: add some visual effects */
// 
//                             return;
//                         }
// 
//                         if (_pathFinder->isAdjacent(myPseudoPosition, _currentTarget) || !me->isMoving() || _forceChangePath) {
//                             std::pair<int32_t, int32_t> _currentTarget = _mapManager->getRandomNearPointEx(myPseudoPosition.first, myPseudoPosition.second);
//                             std::vector<Position> path = _mapManager->toWaypointPathTransform(_pathFinder->findPath(myPseudoPosition, _currentTarget, 42)); // depth an magic number need to tune this method
// 
//                             me->GetMotionMaster()->MoveSmoothPath(0, path.data(), path.size(), false);  //MovePath(path, false);
//                         }
//                         break;
//                     }
                    default:
                        return PacMan::GhostAI::DoAction(action);
                }

                _forceChangePath = false;
                _events.RescheduleEvent(PacMan::GHOST_FIND_TARGET_POINT, 100ms);
            }
//             else if (action == PacMan::INKY_WIN) {
//                 srand(static_cast<unsigned int>(time(nullptr) / 2 + me->GetGUID().GetRawValue()));
//                 int32_t random = rand() % 10;
//                 me->Say(PacMan::sprintf(PacMan::GHOSTS_SAYS_WHEN_ANOTHER_ONE_CATCH_PLAYER[random], "Inky"), LANG_UNIVERSAL);
//             }
//             else if (action == PacMan::CLYDE_WIN) {
//                 srand(static_cast<unsigned int>(time(nullptr) / 2 + me->GetGUID().GetRawValue()));
//                 int32_t random = rand() % 10;
//                 me->Say(PacMan::sprintf(PacMan::GHOSTS_SAYS_WHEN_ANOTHER_ONE_CATCH_PLAYER[random], "Clyde"), LANG_UNIVERSAL);
//             }
//             else if (action == PacMan::INKY_LOSE) {
//                 srand(static_cast<unsigned int>(time(nullptr) / 2 + me->GetGUID().GetRawValue()));
//                 int32_t random = rand() % 10;
//                 me->Say(PacMan::sprintf(PacMan::GHOSTS_SAYS_WHEN_ANOTHER_ONE_IS_EATEN[random], "Inky"), LANG_UNIVERSAL);
//             }
//             else if (action == PacMan::CLYDE_LOSE) {
//                 srand(static_cast<unsigned int>(time(nullptr) / 2 + me->GetGUID().GetRawValue()));
//                 int32_t random = rand() % 10;
//                 me->Say(PacMan::sprintf(PacMan::GHOSTS_SAYS_WHEN_ANOTHER_ONE_IS_EATEN[random], "Clyde"), LANG_UNIVERSAL);
//             }
            else if (action == PacMan::GHOST_WIN) {
                Unit* owner = me->GetOwner();
                if (owner) {
                    owner->GetAI()->DoAction(PacMan::BLINKY_WIN);
                }
            }
            else if (action == PacMan::GHOST_LOSE) {
                Unit* owner = me->GetOwner();
                if (owner) {
                    owner->GetAI()->DoAction(PacMan::BLINKY_LOSE);
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
//                     Position _startPosition = me->GetPosition();
//                     _currentTarget = _mapManager->toPseudoCellTransform(_startPosition);
//                     _events.ScheduleEvent(PacMan::GHOST_FIND_TARGET_POINT, 1ms);
//                     _events.ScheduleEvent(PacMan::GHOST_CHANGE_STATE, PacMan::GHOST_AI_SCATTER_TIME);
//                     break;
//                 }
//                 case PacMan::GHOST_CHANGE_STATE: {
//                     switch (_state)
//                     {
//                         case PacMan::EAIState::AI_STATE_CHASE: {
//                             /*DEBUG*/
//                             _state = PacMan::EAIState::AI_STATE_FRIGHTEND;
//                             me->CastSpell(nullptr, PacMan::SPELL_FEAR_VISUAL);
//                             me->CastSpell(nullptr, PacMan::SPELL_NPC_CHANGE_STATE_VISUAL);
//                             _events.ScheduleEvent(PacMan::GHOST_CHANGE_STATE, PacMan::GHOST_AI_FRIGHTENED_TIME);
// 
//                             Player* player = ObjectAccessor::GetPlayer(me->GetMap(), _playerGuid);
//                             if (player) {
//                                 srand(static_cast<unsigned int>(time(nullptr)));
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
// 
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
        return new npc_blinkyAI(creature);
    }

};

void AddSC_npc_blinky()
{
    new npc_blinky();
}

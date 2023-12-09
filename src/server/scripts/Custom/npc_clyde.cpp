#include "SmartAI.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "PacmanMap.h"
#include "SpellMgr.h"
#include "GhostAI.h"

class npc_clyde : public CreatureScript
{
public:
    npc_clyde() : CreatureScript("npc_clyde") { }

    struct npc_clydeAI : public PacMan::GhostAI {

        npc_clydeAI(Creature* creature) : PacMan::GhostAI(creature) {
        }

        void InitializeAI() override {
            PacMan::GhostAI::InitializeAI();
            /* Define scatter positions */
            _scatterPositions.push(PacMan::CLYDE_SCATTER_POS1);
            _scatterPositions.push(PacMan::CLYDE_SCATTER_POS2);
            _scatterPositions.push(PacMan::CLYDE_SCATTER_POS3);

            if (PacMan::GHOSTS_START_BY_TIMER) {
                _events.ScheduleEvent(PacMan::GHOST_DELAYED_START, PacMan::CLYDE_AI_DELAYED_START);
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

                            int32_t distance = std::abs(playerPseudoPosition.first - myPseudoPosition.first) +
                                               std::abs(playerPseudoPosition.second - myPseudoPosition.second);

                            if (distance < PacMan::CLYDE_AI_CHASE_DISTANCE) {
                                _currentTarget = _scatterPositions.front();
                                _scatterPositions.pop();
                                _scatterPositions.push(_currentTarget);
                            }
                            else {
                                _currentTarget = playerPseudoPosition;
                            }

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
                    owner->GetAI()->DoAction(PacMan::CLYDE_WIN);
                }
            }
            else if (action == PacMan::GHOST_LOSE) {
                Unit* owner = me->GetOwner();
                if (owner) {
                    owner->GetAI()->DoAction(PacMan::CLYDE_LOSE);
                }
            }
            else {
                return PacMan::GhostAI::DoAction(action);
            }
        }
    };
    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_clydeAI(creature);
    }
};

void AddSC_npc_clyde()
{
    new npc_clyde();
}

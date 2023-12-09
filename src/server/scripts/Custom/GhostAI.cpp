#include "GhostAI.h"
#include "ObjectAccessor.h"
namespace PacMan {
    GhostAI::GhostAI(Creature* creature) : ScriptedAI(creature)
    {

    }

    void GhostAI::InitializeAI()
    {
        /* you can load map from another source  */
        try {
            _mapManager = std::make_unique<MapManager>(TXT_MAP);
            _mapManager->parse();
            _mapManager->calculateArea(POS_MAP);
            _pathFinder = std::make_unique<PathFinder>(_mapManager->getGrid());
        }
        catch (std::exception& /*e*/) {
            me->KillSelf();
            return;
        }

        _defautlSpeed = me->GetSpeed(MOVE_RUN);
        me->CastSpell(nullptr, SPELL_STUN_VISUAL);
        _state = EAIState::AI_STATE_SCATTER;
        /* else trigger by eated points */
    }

    void GhostAI::Reset()
    {
        _events.Reset();
    }

    void GhostAI::OnDespawn()
    {
        _events.Reset();
    }

    void GhostAI::DoAction(int32 action)
    {
        if (action == GHOST_FIND_TARGET_POINT) {
            Player* player = ObjectAccessor::GetPlayer(me->GetMap(), _playerGuid);
            if (!player) {
                return;
            }

            Position playerPosition = player->GetPosition();
            Position myPosition = me->GetPosition();

            std::pair<int32_t, int32_t> myPseudoPosition = _mapManager->toPseudoCellTransform(myPosition);
            std::pair<int32_t, int32_t> playerPseudoPosition = _mapManager->toPseudoCellTransform(playerPosition);

            switch (_state) {
                case EAIState::AI_STATE_SCATTER: {
                    Position position = me->GetPosition();
                    std::pair<int32_t, int32_t> pseudoPosition = _mapManager->toPseudoCellTransform(position);

                    if (myPseudoPosition == playerPseudoPosition) {
                        return PlayerCatched();
                    }

                    if (_pathFinder->isAdjacent(pseudoPosition, _currentTarget) || !me->isMoving() || _forceChangePath) {
                        std::pair<int32_t, int32_t> _currentTarget = _scatterPositions.front();
                        _scatterPositions.pop();
                        _scatterPositions.push(_currentTarget);

                        std::vector<Position> path = _mapManager->toWaypointPathTransform(_pathFinder->findPath(pseudoPosition, _currentTarget, 100));
                        me->GetMotionMaster()->MoveSmoothPath(0, path.data(), path.size(), false);
                    }
                    break;
                }
                case EAIState::AI_STATE_FRIGHTEND: {
                    if (myPseudoPosition == playerPseudoPosition) {
                        me->GetAI()->DoAction(GHOST_LOSE);

                        srand(static_cast<unsigned int>(time(nullptr) / 2 + me->GetGUID().GetRawValue()));
                        int32_t random = rand() % 10;
                        me->Say(sprintf(GHOST_SAYS_WHEN_WAS_EATEN[random], player->GetName().c_str()), LANG_UNIVERSAL);

                        me->RemoveAurasDueToSpell(SPELL_FEAR_VISUAL);
                        _isFeightDeath = true;
                        me->CastSpell(me, SPELL_FEIGN_DEATH);
                        _events.Reset();
                        _events.ScheduleEvent(GHOST_BACK_TO_START, GHOST_RESPAWN_TIME);

                        /* TODO: add some visual effects */

                        return;
                    }

                    if (_pathFinder->isAdjacent(myPseudoPosition, _currentTarget) || !me->isMoving() || _forceChangePath) {
                        std::pair<int32_t, int32_t> _currentTarget = _mapManager->getRandomNearPointEx(myPseudoPosition.first, myPseudoPosition.second);
                        std::vector<Position> path = _mapManager->toWaypointPathTransform(_pathFinder->findPath(myPseudoPosition, _currentTarget, 42)); // depth an magic number need to tune this method

                        me->GetMotionMaster()->MoveSmoothPath(0, path.data(), path.size(), false);  //MovePath(path, false);
                    }
                    break;
                }
                default:
                    break;
            }

            _forceChangePath = false;
            _events.RescheduleEvent(GHOST_FIND_TARGET_POINT, 100ms);
        }
        else if (action == BONUS_POINT_DESPAWN) {
            _state = EAIState::AI_STATE_FRIGHTEND;
            me->CastSpell(nullptr, SPELL_FEAR_VISUAL);
            me->CastSpell(me, SPELL_NPC_CHANGE_STATE_VISUAL);
            _events.RescheduleEvent(GHOST_CHANGE_STATE, GHOST_AI_FRIGHTENED_TIME);

            Player* player = ObjectAccessor::GetPlayer(me->GetMap(), _playerGuid);
            if (player) {
                srand(static_cast<unsigned int>(time(nullptr) / 2 + me->GetGUID().GetRawValue()));
                int32_t random = rand() % 10;
                me->Yell(sprintf(GHOST_SAY_WHEN_WAS_FRIGHTENED[random], player->GetName().c_str()), LANG_UNIVERSAL);
            }

            me->SetSpeed(MOVE_RUN, _defautlSpeed * 0.6f); // - 40% speed

            _forceChangePath = true;
        }
        else if (_isFeightDeath) {
            /* prevent replica when _isFeightDeath */
            return;
        }
        else if (action == INKY_WIN) {
            srand(static_cast<unsigned int>(time(nullptr) / 2 + me->GetGUID().GetRawValue()));
            int32_t random = rand() % 10;
            me->Yell(sprintf(GHOSTS_SAYS_WHEN_ANOTHER_ONE_CATCH_PLAYER[random], "Inky"), LANG_UNIVERSAL);
            PlayerCatched(false);
        }
        else if (action == CLYDE_WIN) {
            srand(static_cast<unsigned int>(time(nullptr) / 2 + me->GetGUID().GetRawValue()));
            int32_t random = rand() % 10;
            me->Yell(sprintf(GHOSTS_SAYS_WHEN_ANOTHER_ONE_CATCH_PLAYER[random], "Clyde"), LANG_UNIVERSAL);
            PlayerCatched(false);
        }
        else if (action == INKY_LOSE) {
            srand(static_cast<unsigned int>(time(nullptr) / 2 + me->GetGUID().GetRawValue()));
            int32_t random = rand() % 10;
            me->Yell(sprintf(GHOSTS_SAYS_WHEN_ANOTHER_ONE_IS_EATEN[random], "Inky"), LANG_UNIVERSAL);
        }
        else if (action == CLYDE_LOSE) {
            srand(static_cast<unsigned int>(time(nullptr) / 2 + me->GetGUID().GetRawValue()));
            int32_t random = rand() % 10;
            me->Yell(sprintf(GHOSTS_SAYS_WHEN_ANOTHER_ONE_IS_EATEN[random], "Clyde"), LANG_UNIVERSAL);
        }
        else if (action == BLINKY_LOSE) {
            srand(static_cast<unsigned int>(time(nullptr) / 2 + me->GetGUID().GetRawValue()));
            int32_t random = rand() % 10;
            me->Yell(sprintf(GHOSTS_SAYS_WHEN_ANOTHER_ONE_IS_EATEN[random], "Blinky"), LANG_UNIVERSAL);
        }
        else if (action == BLINKY_WIN) {
            srand(static_cast<unsigned int>(time(nullptr) / 2 + me->GetGUID().GetRawValue()));
            int32_t random = rand() % 10;
            me->Yell(sprintf(GHOSTS_SAYS_WHEN_ANOTHER_ONE_CATCH_PLAYER[random], "Blinky"), LANG_UNIVERSAL);
            PlayerCatched(false);
        }
        else if (action == STOP_GHOSTS) {
            _events.Reset();
        }
    }

    void GhostAI::UpdateAI(uint32 diff)
    {
        _events.Update(diff);

        switch (_events.ExecuteEvent()) {
            case GHOST_DELAYED_START: {
                Unit* owner = me->GetOwner();
                if (owner) {
                    _playerGuid = owner->GetAI()->GetGUID(GET_CURRENT_PLAYER);
                }
                else {
                    _playerGuid = ObjectGuid::Empty;
                }
                me->RemoveAurasDueToSpell(SPELL_STUN_VISUAL);
                _startPosition = me->GetPosition();
                _currentTarget = _mapManager->toPseudoCellTransform(_startPosition);
                if (_state != EAIState::AI_STATE_FRIGHTEND) {
                    _state = EAIState::AI_STATE_SCATTER;
                    _events.ScheduleEvent(GHOST_CHANGE_STATE, GHOST_AI_SCATTER_TIME);
                }
                _events.ScheduleEvent(GHOST_FIND_TARGET_POINT, 1ms);
                break;
            }
            case GHOST_CHANGE_STATE: {
                switch (_state) {
                case EAIState::AI_STATE_CHASE: {
                    _state = EAIState::AI_STATE_SCATTER;
                    me->CastSpell(me, SPELL_NPC_CHANGE_STATE_VISUAL);
                    _events.ScheduleEvent(GHOST_CHANGE_STATE, GHOST_AI_SCATTER_TIME);
                    break;
                }
                case EAIState::AI_STATE_SCATTER: {
                    _state = EAIState::AI_STATE_CHASE;
                    me->CastSpell(me, SPELL_NPC_CHANGE_STATE_VISUAL);
                    _events.ScheduleEvent(GHOST_CHANGE_STATE, GHOST_AI_CHASE_TIME);
                    break;
                }
                case EAIState::AI_STATE_FRIGHTEND: {
                    _state = EAIState::AI_STATE_SCATTER;
                    me->SetSpeed(MOVE_RUN, _defautlSpeed);
                    me->RemoveAurasDueToSpell(SPELL_FEAR_VISUAL);
                    me->CastSpell(me, SPELL_NPC_CHANGE_STATE_VISUAL);
                    _events.ScheduleEvent(GHOST_CHANGE_STATE, GHOST_AI_SCATTER_TIME);
                    break;
                }
                default:
                    break;
                }
                _forceChangePath = true;
                break;
            }
            case GHOST_FIND_TARGET_POINT: {
                DoAction(GHOST_FIND_TARGET_POINT);
                break;
            }
            case GHOST_CHASE_FORCE_RECALCULATE_PATH: {
                _forceChangePath = true;
                break;
            }
            case GHOST_BACK_TO_START: {
                me->NearTeleportTo(_startPosition);
                me->RemoveAurasDueToSpell(SPELL_FEIGN_DEATH);
                me->RemoveAurasDueToSpell(SPELL_FEAR_VISUAL);
                me->CastSpell(nullptr, SPELL_STUN_VISUAL);
                _isFeightDeath = false;
                me->SetSpeed(MOVE_RUN, _defautlSpeed);
                _state = EAIState::AI_STATE_SCATTER;
                _events.ScheduleEvent(GHOST_DELAYED_START, GHOST_RESPAWN_TIME);
            }
            default:
                break;
        }
    }

    void GhostAI::PlayerCatched(bool byMe /* = true */)
    {
        if (byMe) {
            me->GetAI()->DoAction(GHOST_WIN);
        }
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveIdle();
        me->CastSpell(nullptr, SPELL_SELF_STUN);
        _events.RescheduleEvent(GHOST_FIND_TARGET_POINT, PLAYER_STUN_DURATION);
        /* TODO: add some visual effects */

//                         _events.Reset();
//                         return;
    }

}// namespace PacMan

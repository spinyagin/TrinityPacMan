#pragma once
#include "SmartAI.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "PacmanMap.h"
#include "SpellMgr.h"
#include "MotionMaster.h"

namespace PacMan {
    struct GhostAI : public ScriptedAI {

        GhostAI(Creature* creature);

        void InitializeAI() override;

        void Reset() override;

        void OnDespawn() override;

        void DoAction(int32 action) override;

        void UpdateAI(uint32 diff) override;

        void PlayerCatched(bool byMe = true);

    protected:
        std::unique_ptr<MapManager> _mapManager;
        std::unique_ptr <PathFinder> _pathFinder;

        PacMan::EAIState _state = EAIState::AI_STATE_SCATTER;
        Position _startPosition;
        std::pair<int32_t, int32_t> _currentTarget;
        std::queue<std::pair<int32_t, int32_t>> _scatterPositions;

        bool _forceChangePath = false;
        bool _isFeightDeath = false;
        ObjectGuid _playerGuid;
        float _defautlSpeed = 0.0f;

        EventMap _events;
    };
}

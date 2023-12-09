#include "SmartAI.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "PacmanMap.h"

class npc_regular_point : public CreatureScript
{
public:
    npc_regular_point() : CreatureScript("npc_regular_point") { }

    struct npc_regular_pointAI : public ScriptedAI {

        npc_regular_pointAI(Creature* creature) : ScriptedAI(creature) {}



        void OnDespawn() override
        {
        }

        void DoAction(int32 action) override {
            if (action == PacMan::CREATURE_TOUCHED_BY_PLAYER) {
                me->KillSelf(); // death animation
                me->DespawnOrUnsummon(200ms); // delayed corpse despawn
                Unit* owner = me->GetOwner();
                if (owner)
                    owner->GetAI()->SetGUID(me->GetGUID(), PacMan::REGULAR_POINT_DESPAWN);
            }
        }


        /* This solution works very slowly, I'll leave it for history */
        void UpdateAI(uint32 diff) override
        {
           /* events.Update(diff);*/

/*            if (events.ExecuteEvent() == PacMan::CHECK_TOUCHED_BY_PLAYER)
            {
                Unit* owner = me->GetOwner();
                if (owner) {
                    _currentPlayer = owner->GetAI()->GetGUID(PacMan::GET_CURRENT_PLAYER);
                    std::list<Player*> players;
                    me->GetPlayerListInGrid(players, 0.1f); // probably there are minimum distance normalized > 0.1f 
                    for (auto&player : players) {
                        if (player->GetGUID() == _currentPlayer) {
                            if (me->GetExactDist(player->GetPosition()) < 1.0f) {
                                me->KillSelf(); // death animation
                                me->DespawnOrUnsummon(100ms);
                            }
                            return;
                        }
                    }
                }
            }
            events.RescheduleEvent(PacMan::CHECK_TOUCHED_BY_PLAYER, 50ms);
            */
        }

    private:
//         ObjectGuid _currentPlayer;
//         EventMap events;
    };
    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_regular_pointAI(creature);
    }
};

void AddSC_npc_regular_point()
{
    new npc_regular_point();
}

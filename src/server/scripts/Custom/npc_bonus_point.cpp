#include "SmartAI.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "PacmanMap.h"

class npc_bonus_point : public CreatureScript
{
public:
    npc_bonus_point() : CreatureScript("npc_bonus_point") { }

    struct npc_bonus_pointAI : public ScriptedAI {

        npc_bonus_pointAI(Creature* creature) : ScriptedAI(creature) {}

        void DoAction(int32 action) override {
            if (action == PacMan::CREATURE_TOUCHED_BY_PLAYER) {
                me->KillSelf(); // death animation
                me->DespawnOrUnsummon(200ms); // delayed corpse despawn
                Unit* owner = me->GetOwner();
                if (owner)
                    owner->GetAI()->SetGUID(me->GetGUID(), PacMan::BONUS_POINT_DESPAWN);
            }
        }

    };
    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_bonus_pointAI(creature);
    }
};

void AddSC_npc_bonus_point()
{
    new npc_bonus_point();
}
